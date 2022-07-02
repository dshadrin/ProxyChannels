/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "stdinc.h"
#include "stream_logger_adapter.h"
#include "oscar/flap_parser.h"
#include <iostream>

extern void DirectSendToLogger(PLog logPackage);

CStreamLoggerAdaptor::CStreamLoggerAdaptor(boost::property_tree::ptree& pt) :
    CLogAdaptor(pt),
    m_protocol( ConvertProtocolName2Id( pt.get<std::string>( "protocol", "PROTO_STREAM" ) ) ),
    m_package{ "", 
               pt.get<std::string>("tag", ""),
               {0, 0}, ELogCommand::eMessage,
               (uint8_t)StringToSeverity(pt.get<std::string>("severity", "DEBUG")),
               LOG_UNKNOWN_CHANNEL }
{
    m_sigOutputMessage.connect(std::bind(&CStreamLoggerAdaptor::DoLog, this, std::placeholders::_1));
}

void CStreamLoggerAdaptor::Start()
{

}

void CStreamLoggerAdaptor::Stop()
{
    m_sigOutputMessage.disconnect_all_slots();
}

std::string CStreamLoggerAdaptor::GetName() const
{
    return ConvertId2String<EActorType>( EActorType::LOGGER_ADAPTOR );
}

void CStreamLoggerAdaptor::DoLog(PMessage msg)
{
    if (m_protocol == ENetProtocol::STREAM)
    {
        timespec timestamp = TS::GetTimestamp();
        std::string::size_type pos = 0;

        auto logString = [this, &pos]() -> void
        {
            m_package.message = m_work.buffer.substr(0, pos + 1);
            m_work.buffer.erase(0, pos + 1);
            boost::algorithm::trim_right(m_package.message);
            if (!m_package.message.empty())
            {
                m_package.timestamp = m_work.timestamp;
                m_toLog(m_package);
            }
        };

        if (m_work.buffer.empty())
        {
            m_work.buffer.assign(msg->begin(), msg->end());
            m_work.timestamp = timestamp;
        }
        else
        {
            m_work.buffer.append(msg->begin(), msg->end());

            pos = m_work.buffer.find('\n');
            if (pos != std::string::npos)
            {
                logString();
                m_work.timestamp = timestamp;
            }
        }

        pos = m_work.buffer.find('\n');
        while (pos != std::string::npos)
        {
            logString();
            pos = m_work.buffer.find('\n');
        }
    }
}

