// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "etf_logger_adapter.h"
#include "utils/timestamp.h"

CEtfLoggerAdaptor::CEtfLoggerAdaptor(boost::property_tree::ptree& pt) :
    CLogAdaptor(pt),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", PROTO_ETFLOG)))
{
    m_sigOutputMessage.connect(std::bind(&CEtfLoggerAdaptor::DoLog, this, std::placeholders::_1));
}

void CEtfLoggerAdaptor::Start()
{

}

void CEtfLoggerAdaptor::Stop()
{
    m_sigOutputMessage.disconnect_all_slots();
}

std::string CEtfLoggerAdaptor::GetName() const
{
    return LOGGER_ADAPTOR;
}

void CEtfLoggerAdaptor::DoLog(PMessage msg)
{
    if (m_protocol == ENetProtocol::eEtfLog)
    {
        SLogPackage logPackage;
        SEtfLogHeader* logHeader = reinterpret_cast<SEtfLogHeader*>(&(*msg.get())[0]);

        if (memcmp(&logHeader[0], "LOG", 3) == 0)
        {
            logPackage.tag = std::string(&logHeader->module[0], &logHeader->module[3]);
            logPackage.tag.append(1, ' ');

            logPackage.severity = logHeader->severity + 1;
            logPackage.message = std::string(&(*msg.get())[0] + ETF_LOG_HEADER_SIZE, &(*msg.get())[0] + msg->size());
            logPackage.lchannel = LOG_CLIENT_CHANNEL;
            logPackage.timestamp = TS::GetTimestamp();
            logPackage.command = ELogCommand::eMessage;

            m_toLog(logPackage);
        }
    }
}

