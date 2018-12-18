/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#include "stdinc.h"
#include "sink.h"
#include "manager.h"
#include "thread_pool.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////////
// Factory method
//////////////////////////////////////////////////////////////////////////
CSink* CSink::MakeSink(const std::string& name, const boost::property_tree::ptree& pt)
{
    std::string sinkName = boost::algorithm::to_upper_copy(name);
    boost::algorithm::trim(sinkName);

    if (sinkName == CONSOLE_SINK)
        return new CConsoleSink(pt);

    else if (sinkName == FILE_SINK)
        return new CFileSink(pt);

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

void CConsoleSink::Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData)
{
    for (auto& it : *logData)
    {
        if (it->command == ELogCommand::eMessage && it->lchannel == LOG_INTERNAL_CHANNEL)
        {
            std::cout << "[" << TS::GetTimestampStr(it->timestamp) << "][" << it->tag << "][" << SeverityToString(it->severity) << "] - " << it->message << std::endl;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// File sink
//////////////////////////////////////////////////////////////////////////
CFileSink::CFileSink(const boost::property_tree::ptree& pt)
{
    for (auto& prop : pt)
    {
        SetProperty(prop.first, prop.second.get_value<std::string>());
    }
}

void CFileSink::SetProperty(const std::string& name, const std::string& value)
{
    if (name == "prefix")
        m_prefix = value;

    else if (name == "suffix")
        m_suffix = value;

    else if (name == "template" && !value.empty())
    {
        m_fileNameTemplate = value;
    }

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
    boost::filesystem::path path(buffer);
    std::string fName = path.stem().string();
    if (!m_prefix.empty())
    {
        fName.insert(0, 1, '_');
        fName.insert(0, m_prefix);
    }
    if (!m_suffix.empty())
    {
        fName.append(1, '_');
        fName.append(m_suffix);
    }
    fName.append(path.extension().string());
    m_fileName = fName;
}

void CFileSink::Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData)
{
    OpenStream();
    for (auto& it : *logData)
    {
        if (it->lchannel == m_channel)
        {
            switch (it->command)
            {
                case ELogCommand::eMessage:
                    ofs << "[" << TS::GetTimestampStr(it->timestamp) << "][" << it->tag << "][" << SeverityToString(it->severity) << "] - " << it->message << std::endl;
                    break;
                case ELogCommand::eStop:
                    if (m_channel != 0)
                    {
                        CloseStream();
                        m_fileName.clear();
                    }
                    else
                    {
                        ofs << "[" << TS::GetTimestampStr(TS::GetTimestamp()) << "][LOG ][WARN] - Attempt to close default log channel." << std::endl;
                    }
                    return;
                case ELogCommand::eChangeFile:
                    if (m_channel != 0)
                    {
                        m_prefix = it->message;
                        m_suffix = it->tag;
                        CloseStream();
                        m_fileName.clear();
                        OpenStream();
                    }
                    else
                    {
                        ofs << "[" << TS::GetTimestampStr(TS::GetTimestamp()) << "][LOG ][WARN] - Attempt to change file name for default log channel." << std::endl;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    CloseStream();
}

void CFileSink::OpenStream()
{
    if (!ofs.is_open())
    {
        if (m_fileName.empty())
        {
            CreateFileName();
        }

        ofs.open(m_fileName, std::ios_base::out | std::ios_base::app);
    }
}

void CFileSink::CloseStream()
{
    if (ofs.is_open())
    {
        ofs.flush();
        ofs.close();
    }
}
