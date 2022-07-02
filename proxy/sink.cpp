/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "stdinc.h"
#include "sink.h"
#include "manager.h"
#include "actor.h"
#include "thread_pool.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////////
// Factory method
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CSink, "SINK");
std::condition_variable CSink::m_jobCond;
std::mutex CSink::m_jobMutex;
std::atomic_uint_fast64_t CSink::m_jobCounter(0);

CSink* CSink::MakeSink(const std::string& type, const boost::property_tree::ptree& pt)
{
    try
    {
        ESinkType sType = ConvertString2Id<ESinkType>( type );

        switch (sType)
        {
        case ESinkType::FILE_SINK:
            return new CFileSink( pt );
        case ESinkType::CONSOLE_SINK:
            return new CConsoleSink( pt );
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Error create sink: " << e.what();
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Base class
//////////////////////////////////////////////////////////////////////////
void CSink::SetProperty(const std::string& name, const std::string& value)
{
    if (name == "severity")
        m_severity = StringToSeverity(value);

    else if (name == "channel")
        m_channel = std::stoi(value);
}

bool CSink::WaitJobFinishAllSinks(uint32_t ms)
{
    std::chrono::high_resolution_clock::time_point tmEnd = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(ms);
    std::unique_lock<std::mutex> jobLock(m_jobMutex);

    m_jobCond.wait_until(jobLock, tmEnd, []() { return m_jobCounter.load() == 0; });

    return m_jobCounter.load() == 0;
}

void CSink::IncrementJobCounter()
{
    m_jobCounter++;
}

void CSink::DecrementJobCounter()
{
    m_jobCounter--;
    m_jobCond.notify_all();
}

//////////////////////////////////////////////////////////////////////////
// Console sink
//////////////////////////////////////////////////////////////////////////
CConsoleSink::CConsoleSink(const boost::property_tree::ptree& pt)
{
    for (auto& prop : pt)
    {
        SetProperty(prop.first, prop.second.get_value<std::string>());
    }
}

void CConsoleSink::Write(std::shared_ptr<std::vector<PLog>> logData)
{
    for (auto& it : *logData)
    {
        if (it->command == ELogCommand::eMessage && it->lchannel == LOG_INTERNAL_CHANNEL)
        {
            std::cout << "[" << TS::GetTimestampStr(it->timestamp) << "][" << it->tag << "][" << SeverityToString(it->severity) << "] - " << it->message << std::endl;
        }
    }
    DecrementJobCounter();
}

//////////////////////////////////////////////////////////////////////////
// File sink
//////////////////////////////////////////////////////////////////////////
CFileSink::CFileSink(const boost::property_tree::ptree& pt) :
    m_isOpenByDemand(false)
{
    for (auto& prop : pt)
    {
        SetProperty(prop.first, prop.second.get_value<std::string>());
    }

    if (m_channel == LOG_INTERNAL_CHANNEL || !m_isOpenByDemand)
    {
        CreateFileName();
        std::cout << "Created filename for channel " << (int)m_channel << " : " << m_fileName << std::endl;
    }
}

void CFileSink::SetProperty(const std::string& name, const std::string& value)
{
    if (name == "prefix")
        m_prefix = value;

    else if (name == "suffix")
        m_suffix = value;

    else if (name == "template" && !value.empty())
        m_fileNameTemplate = value;

    else if (name == "open_by_demand" && boost::algorithm::to_lower_copy(value) == "true")
        m_isOpenByDemand = true;

    else
        CSink::SetProperty(name, value);
}

void CFileSink::CreateFileName()
{
    tm tmStruct;
    long us;
    TS::ConvertTimestamp(TS::GetTimestamp(), &tmStruct, &us);
    std::string buffer;
    buffer.resize(100);
    size_t sz = strftime(&buffer[0], 100, m_fileNameTemplate.c_str(), &tmStruct);
    buffer.resize(sz);
    fs::path path(buffer);
    std::string fName = path.stem().string();
    if (!m_prefix.empty())
    {
        fName.insert(0, 1, '_');
        fName.insert(0, m_prefix);
    }
    fName.append(1, '_');
    fName.append(std::to_string((int)m_channel));
    if (!m_suffix.empty())
    {
        fName.append(1, '_');
        fName.append(m_suffix);
    }
    fName.append(path.extension().string());
    m_fileName = std::move(fName);
}

void CFileSink::Write(std::shared_ptr<std::vector<PLog>> logData)
{
    auto writeMessage = [this](const PLog& data) -> void
    {
        if (m_ofs.is_open())
        {
            m_ofs << "[" << TS::GetTimestampStr(data->timestamp) << "][" << data->tag << "][" << SeverityToString(data->severity) << "] - " << data->message << std::endl;
        }
    };

    bool isBreak = false;
    OpenStream();
    for (auto& it : *logData)
    {
        if (it->lchannel == m_channel)
        {
            switch (it->command)
            {
                case ELogCommand::eMessage:
                    writeMessage(it);
                    break;
                case ELogCommand::eStop:
                    if (m_channel > LOG_INTERNAL_CHANNEL)
                    {
                        m_fileName.clear();
                        isBreak = true;
                    }
                    else
                    {
                        LOG_WARN << "Attempt to close non applicable log channel " << (int)m_channel;
                    }
                    break;
                case ELogCommand::eChangeFile:
                    if (m_channel > LOG_INTERNAL_CHANNEL)
                    {
                        m_prefix = it->message;
                        m_suffix = it->tag;
                        CloseStream();
                        CreateFileName();
                        LOG_DEBUG << "Created filename for channel " << (int)m_channel << " : " << m_fileName;
                        OpenStream();
                    }
                    else
                    {
                        LOG_WARN << "Attempt to change file name for non applicable log channel " << (int)m_channel;
                    }
                    break;
                default:
                    break;
            }
            if (isBreak)
                break;
        }
    }
    CloseStream();
    DecrementJobCounter();
}

void CFileSink::OpenStream()
{
    if (!m_ofs.is_open() && !m_fileName.empty())
    {
        m_ofs.open(m_fileName, std::ios_base::out | std::ios_base::app);
    }
}

void CFileSink::CloseStream()
{
    if (m_ofs.is_open())
    {
        m_ofs.flush();
        m_ofs.close();
    }
}
