#include "stdinc.h"
#include "stream_logger_adapter.h"
#include "oscar/flap_parser.h"
#include <iostream>

extern void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage);

CStreamLoggerAdaptor::CStreamLoggerAdaptor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", "RAW"))),
    m_filterESC(pt.get<bool>("filter_esc", false)),
    m_package{ "", 
               pt.get<std::string>("tag", ""),
               {0, 0}, ELogCommand::eMessage,
               (uint8_t)StringToSeverity(pt.get<std::string>("severity", "DEBUG")),
               pt.get<int8_t>("channel", LOG_UNKNOWN_CHANNEL) }
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
    return LOGGER_ADAPTOR;
}

void CStreamLoggerAdaptor::DoLog(PMessage msg)
{
    if (m_protocol == ENetProtocol::eStream)
    {
        timespec timestamp = TS::GetTimestamp();
        std::string::size_type pos = 0;

        auto logString = [this, &pos]() -> void
        {
            std::shared_ptr<SLogPackage> logPackage(new SLogPackage(m_package));
            logPackage->message = m_work.buffer.substr(0, pos + 1);
            m_work.buffer.erase(0, pos + 1);
            boost::algorithm::trim_right(logPackage->message);
            if (!logPackage->message.empty())
            {
                if (m_filterESC)
                {
                    boost::algorithm::erase_all_regex(logPackage->message, boost::regex{ "(\x1B\\[([0-9]+;)*[0-9]*[m|h|l]{1})" });
                }
                if (!logPackage->message.empty())
                {
                    logPackage->timestamp = m_work.timestamp;
                    DirectSendToLogger(logPackage);
                }
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

