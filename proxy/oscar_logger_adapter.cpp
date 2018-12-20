#include "stdinc.h"
#include "oscar_logger_adapter.h"
#include "oscar/flap_parser.h"
#include <iostream>

COscarLoggerAdaptor::COscarLoggerAdaptor(boost::property_tree::ptree& pt) :
    CLogAdaptor(pt),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", PROTO_OSCAR)))
{
    m_sigOutputMessage.connect(std::bind(&COscarLoggerAdaptor::DoLog, this, std::placeholders::_1));
}

void COscarLoggerAdaptor::Start()
{

}

void COscarLoggerAdaptor::Stop()
{
    m_sigOutputMessage.disconnect_all_slots();
}

std::string COscarLoggerAdaptor::GetName() const
{
    return LOGGER_ADAPTOR;
}

void COscarLoggerAdaptor::DoLog(PMessage msg)
{
    if (m_protocol == ENetProtocol::eOscar)
    {
        SLogPackage logPackage;
        oscar::ostd::flap_parser fp(*msg.get());
        switch (fp.get_channel())
        {
            case FlapChannel::Message:
            {
                auto it = fp.begin();
                logPackage.tag = oscar::tlv::get_value<std::string>(it.get()); ++it;
                logPackage.severity = oscar::tlv::get_value<uint8_t>(it.get()); ++it;
                logPackage.message = oscar::tlv::get_value<std::string>(it.get()); ++it;
                logPackage.lchannel = oscar::tlv::get_value<int8_t>(it.get()); ++it;
                logPackage.timestamp.tv_sec = oscar::tlv::get_value<time_t>(it.get()); ++it;
                logPackage.timestamp.tv_nsec = (long)oscar::tlv::get_value<int64_t>(it.get());
                logPackage.command = ELogCommand::eMessage;

                m_toLog(logPackage);
            }
                break;
            case FlapChannel::Control:
            {
                auto it = fp.begin();
                switch (fp.get_snac_service())
                {
                    case SnacService::Start:
                        logPackage.message = oscar::tlv::get_value<std::string>(it.get()); ++it;
                        logPackage.tag = oscar::tlv::get_value<std::string>(it.get()); ++it;
                        logPackage.lchannel = oscar::tlv::get_value<int8_t>(it.get()); ++it;
                        logPackage.timestamp.tv_sec = oscar::tlv::get_value<time_t>(it.get()); ++it;
                        logPackage.timestamp.tv_nsec = (long)oscar::tlv::get_value<int64_t>(it.get());
                        logPackage.command = ELogCommand::eChangeFile;
                        m_toLog(logPackage);
                        break;
                    case SnacService::Stop:
                        logPackage.lchannel = oscar::tlv::get_value<int8_t>(it.get()); ++it;
                        logPackage.timestamp.tv_sec = oscar::tlv::get_value<time_t>(it.get()); ++it;
                        logPackage.timestamp.tv_nsec = (long)oscar::tlv::get_value<int64_t>(it.get());
                        logPackage.command = ELogCommand::eStop;
                        m_toLog(logPackage);
                        break;
                    default:;
                }
            }
                break;
            default:;
        }
    }
}

