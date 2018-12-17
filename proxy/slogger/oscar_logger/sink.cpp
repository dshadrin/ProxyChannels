/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#include "stdinc.h"
#include "sink.h"
#include "manager.h"
#include "thread_pool.h"
#include <iostream>

CSink* CSink::MakeSink(const std::string& name)
{
    std::string sinkName = boost::algorithm::to_upper_copy(name);
    boost::algorithm::trim(sinkName);

    if (sinkName == CONSOLE_SINK)
        return new CConsoleSink();

    else if (sinkName == FILE_SINK)
        return new CFileSink();

    return nullptr;
}

void CSink::SetProperty(const std::string& name, const std::string& value)
{
    if (name == "severity")
        m_severity = StringToSeverity(value);
}

void CConsoleSink::Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData)
{
    for (auto& it : *logData)
    {
        if (it->command == ELogCommand::eMessage)
        {
            std::cout << "[" << TS::GetTimestampStr(it->timestamp) << "][" << it->tag << "][" << SeverityToString(it->severity) << "] - " << it->message << std::endl;
        }
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
        CreateFileName();
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
    if (!m_fileName.empty())
    {
        OpenStream();
        for (auto& it : *logData)
        {
            switch (it->command)
            {
                case ELogCommand::eMessage:
                    ofs << "[" << TS::GetTimestampStr(it->timestamp) << "][" << it->tag << "][" << SeverityToString(it->severity) << "] - " << it->message << std::endl;
                    break;
                case ELogCommand::eStop:
                    CloseStream();
                    m_fileName.clear();
                    return;
                case ELogCommand::eChangeFile:
                    m_prefix = it->message;
                    m_suffix = it->tag;
                    CloseStream();
                    m_fileName.clear();
                    CreateFileName();
                    OpenStream();
                    break;
                default:
                    break;
            }
        }
        CloseStream();
    }
}

void CFileSink::OpenStream()
{
    if (!ofs.is_open())
    {
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
