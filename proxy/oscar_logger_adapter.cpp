#include "stdinc.h"
#include "oscar_logger_adapter.h"
#include "oscar/flap_parser.h"
#include <iostream>

extern void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage);

COscarLoggerAdaptor::COscarLoggerAdaptor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", "RAW")))
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
        std::shared_ptr<SLogPackage> logPackage(new SLogPackage());
        oscar::ostd::flap_parser fp(*msg.get());
        switch (fp.get_channel())
        {
            case FlapChannel::Message:
            {
                auto it = fp.begin();
                logPackage->tag = oscar::tlv::get_value<std::string>(it.get()); ++it;
                logPackage->severity = oscar::tlv::get_value<uint8_t>(it.get()); ++it;
                logPackage->message = oscar::tlv::get_value<std::string>(it.get()); ++it;
                logPackage->lchannel = oscar::tlv::get_value<int8_t>(it.get()); ++it;
                logPackage->timestamp.tv_sec = oscar::tlv::get_value<time_t>(it.get()); ++it;
                logPackage->timestamp.tv_nsec = (long)oscar::tlv::get_value<int64_t>(it.get());
                logPackage->command = ELogCommand::eMessage;

                DirectSendToLogger(logPackage);
#ifdef USE_OSCAR_LOGGER
            }
                break;
            case FlapChannel::Control:
            {
                auto it = fp.begin();
                switch (fp.get_snac_service())
                {
                    case SnacService::Start:
                        logPackage->message = oscar::tlv::get_value<std::string>(it.get()); ++it;
                        logPackage->tag = oscar::tlv::get_value<std::string>(it.get()); ++it;
                        logPackage->lchannel = oscar::tlv::get_value<int8_t>(it.get()); ++it;
                        logPackage->timestamp.tv_sec = oscar::tlv::get_value<time_t>(it.get()); ++it;
                        logPackage->timestamp.tv_nsec = (long)oscar::tlv::get_value<int64_t>(it.get());
                        logPackage->command = ELogCommand::eChangeFile;
                        DirectSendToLogger(logPackage);
                        break;
                    case SnacService::Stop:
                        logPackage->lchannel = oscar::tlv::get_value<int8_t>(it.get()); ++it;
                        logPackage->timestamp.tv_sec = oscar::tlv::get_value<time_t>(it.get()); ++it;
                        logPackage->timestamp.tv_nsec = (long)oscar::tlv::get_value<int64_t>(it.get());
                        logPackage->command = ELogCommand::eStop;
                        DirectSendToLogger(logPackage);
                        break;
                    default:;
                }
#endif
            }
                break;
            default:;
        }
    }
}

