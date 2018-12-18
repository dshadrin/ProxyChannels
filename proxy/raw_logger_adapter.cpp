#include "stdinc.h"
#include "raw_logger_adapter.h"
#include "oscar/flap_parser.h"
#include <iostream>

extern void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage);

CRawLoggerAdaptor::CRawLoggerAdaptor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", "RAW"))),
    m_severity(StringToSeverity(pt.get<std::string>("severity", "DEBUG"))),
    m_logChannel(pt.get<int8_t>("channel", LOG_UNKNOWN_CHANNEL)),
    m_tag(pt.get<std::string>("tag", "")),
    m_package(new SLogPackage)
{
    m_package->tag = m_tag;
    m_package->lchannel = m_logChannel;
    m_package->command = ELogCommand::eMessage;
    m_package->severity = (uint8_t)m_severity;
    m_sigOutputMessage.connect(std::bind(&CRawLoggerAdaptor::DoLog, this, std::placeholders::_1));
}

void CRawLoggerAdaptor::Start()
{

}

void CRawLoggerAdaptor::Stop()
{
    m_sigOutputMessage.disconnect_all_slots();
}

std::string CRawLoggerAdaptor::GetName() const
{
    return LOGGER_ADAPTOR;
}

void CRawLoggerAdaptor::DoLog(PMessage msg)
{
    if (m_protocol == ENetProtocol::eRaw)
    {
        timespec timestamp = TS::GetTimestamp();
        std::string::size_type pos = 0;

        auto logString = [this, &pos]() -> void
        {
            m_package->message = m_work.buffer.substr(0, pos + 1);
            m_work.buffer.erase(0, pos + 1);
            while (m_package->message.length() > 0 && (*m_package->message.rbegin() == '\r' || *m_package->message.rbegin() == '\n'))
            {
                m_package->message.pop_back();
            }
            if (!m_package->message.empty())
            {
                m_package->timestamp = m_work.timestamp;
                DirectSendToLogger(m_package);
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

