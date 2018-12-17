#include "stdinc.h"
#include "actor.h"
#include "tcpserver.h"
#include "terminal.h"
#include "netloggeradapter.h"

//////////////////////////////////////////////////////////////////////////
ENetProtocol ConvertProtocolName2Id(const std::string pName)
{
    std::string name = boost::algorithm::to_upper_copy(pName);

    if (name == "TELNET")
        return ENetProtocol::eTelnet;

    else if (name == "OSCAR")
        return ENetProtocol::eOscar;

    return ENetProtocol::eRaw;
}

std::string ConvertId2ProtocolName(ENetProtocol pId)
{
    switch (pId)
    {
    case ENetProtocol::eTelnet:
        return "TELNET";
    case ENetProtocol::eOscar:
        return "OSCAR";
    case ENetProtocol::eRaw:
    default:
        return "RAW";
    }
}

//////////////////////////////////////////////////////////////////////////
// factory method
CActor* CActor::MakeActor(boost::property_tree::ptree& pt)
{
    DEFINE_MODULE_TAG("ACTR");
    CActor* actor = nullptr;
    std::string actorName;
    int32_t actorId = -1;

    try
    {
        actorName = pt.get<std::string>("name", "UNKNOWN");
        actorId = pt.get<int32_t>("id", -1);
        LOG_DEBUG << "Begin creating actor " << actorName << "(id = " << actorId << ")";

        if (actorName == TCP_SERVER)
            actor = new CTcpServerActor(pt);

        else if (actorName == UART_CLIENT)
            actor = new CTerminal(pt);

        else if (actorName == LOGGER_ADAPTOR)
            actor = new CNetLoggerAdaptor(pt);

        LOG_DEBUG << "Created actor " << actorName << "(id = " << actorId << ")";
    }
    catch (const std::exception& e)
    {
        LOG_ERR << "Cannot create actor " << actorName << "(id = " << actorId << ")" << ": " << e.what();
    }

    return actor;
}
