#include "stdinc.h"
#include "actor.h"
#include "tcpserver.h"
#include "terminal.h"
#include "oscar_logger_adapter.h"
#include "raw_logger_adapter.h"

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
    std::string actorProto;
    int32_t actorId = -1;

    try
    {
        actorName = pt.get<std::string>("name", "UNKNOWN");
        actorProto = pt.get<std::string>("protocol", NO_PROTO);
        actorId = pt.get<int32_t>("id", -1);
        LOG_DEBUG << "Begin creating actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")";

        if (actorName == TCP_SERVER)
            actor = new CTcpServerActor(pt);

        else if (actorName == UART_CLIENT)
        {
            LOG_DEBUG << "Try to open serial port: " << CTerminal::MakePortName(pt);
            actor = new CTerminal(pt);
        }

        else if (actorName == LOGGER_ADAPTOR)
        {
            if (actorProto == PROTO_OSCAR)
                actor = new COscarLoggerAdaptor(pt);

            else if (actorProto == PROTO_RAW)
                actor = new CRawLoggerAdaptor(pt);

            else
            {
                APP_EXCEPTION_ERROR("Unknown protocol.");
            }
        }
        else
        {
            APP_EXCEPTION_ERROR("Unknown actor name.");
        }

        LOG_DEBUG << "Created actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")";
    }
    catch (const std::exception& e)
    {
        LOG_ERR << "Cannot create actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")" << ": " << e.what();
    }

    return actor;
}
