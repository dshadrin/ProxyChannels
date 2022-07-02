/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "logbuilder.h"
#include "timestamp.h"
#include <cstdint>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>

//////////////////////////////////////////////////////////////////////////
int8_t CLogMessageBuilder::s_logChannel = LOG_INTERNAL_CHANNEL;

//////////////////////////////////////////////////////////////////////////

void CLogMessageBuilder::Init(ESeverity sev, const std::string& tag)
{
    m_package->timestamp = TS::GetTimestamp();
    m_package->severity = static_cast<uint8_t>(sev);
    m_package->command = ELogCommand::eMessage;
    m_package->lchannel = s_logChannel;
    if (tag.length() > G_TagSize)
    {
        m_package->tag = tag.substr(0, G_TagSize);
    }
    else
    {
        m_package->tag = tag;
        if (tag.length() < G_TagSize)
        {
            m_package->tag.resize(G_TagSize, ' ');
        }
    }
}

CLogMessageBuilder::~CLogMessageBuilder()
{
    try
    {
        m_package->message = oss.str();
        if (m_package->message.length() > G_MaxMessageSize)
        {
            m_package->message.resize(G_MaxMessageSize);
        }
        m_Sender(m_package);
    }
    catch (const std::exception&)
    {
        // STD error!!! Skip log message
    }
    catch (...)
    {
        // Unknown error!!! Skip log message
    }
}

void CLogMessageBuilder::SetLoggingChannel(int8_t ch)
{
    s_logChannel = ch;
}

int8_t CLogMessageBuilder::GetLoggingChannel()
{
    return s_logChannel;
}

//////////////////////////////////////////////////////////////////////////
ESeverity StringToSeverity(const std::string& sSev)
{
    std::string strSev = sSev;
    boost::algorithm::to_upper(strSev);

    if (strSev == "TEST")
        return ESeverity::eTest;

    else if (strSev == "WARNING" || strSev == "WARN")
        return ESeverity::eWarn;

    else if (strSev == "DEBUG" || strSev == "DBG")
        return ESeverity::eDebug;

    else if (strSev == "ERROR" || strSev == "ERR")
        return ESeverity::eError;

    else if (strSev == "CRITICAL" || strSev == "CRIT")
        return ESeverity::eCrit;

    else if (strSev == "TRACE")
        return ESeverity::eTrace;

    return ESeverity::eInfo;
}

std::string SeverityToString(uint8_t sev)
{
    switch (sev)
    {
        case (uint8_t)ESeverity::eCrit:
            return "CRIT";
        case (uint8_t)ESeverity::eError:
            return "ERR ";
        case (uint8_t)ESeverity::eWarn:
            return "WARN";
        case (uint8_t)ESeverity::eTest:
            return "TEST";
        case (uint8_t)ESeverity::eInfo:
            return "INFO";
        case (uint8_t)ESeverity::eTrace:
            return "TRAC";
        case (uint8_t)ESeverity::eDebug:
        default:
            return "DBG ";
    }
}

//////////////////////////////////////////////////////////////////////////
