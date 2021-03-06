// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "actor.h"
#include "tcpserver.h"
#include "terminal.h"
#include "oscar_logger_adapter.h"
#include "stream_logger_adapter.h"
#include "etf_logger_adapter.h"
#include "console.h"
#include "manager_adaptor.h"

//////////////////////////////////////////////////////////////////////////
ENetProtocol ConvertProtocolName2Id(const std::string& pName)
{
    std::string name = boost::algorithm::to_upper_copy(pName);

    if (name == PROTO_STREAM)
        return ENetProtocol::eStream;

    else if (name == PROTO_OSCAR)
        return ENetProtocol::eOscar;

    else if (name == PROTO_TELNET)
        return ENetProtocol::eTelnet;

    else if (name == PROTO_ETFLOG)
        return ENetProtocol::eEtfLog;

    APP_EXCEPTION_ERROR(GMSG << "Unknown protocol name: " << pName);
}

std::string ConvertId2ProtocolName(ENetProtocol pId)
{
    switch (pId)
    {
    case ENetProtocol::eOscar:
        return PROTO_OSCAR;
    case ENetProtocol::eStream:
        return PROTO_STREAM;
    case ENetProtocol::eTelnet:
        return PROTO_TELNET;
    case ENetProtocol::eEtfLog:
        return PROTO_ETFLOG;
    default:
        APP_EXCEPTION_ERROR(GMSG << "Unknown protocol id: " << (int)pId);
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
        {
            switch (ConvertProtocolName2Id(actorProto))
            {
                case ENetProtocol::eOscar:
                    actor = new CTcpServerActor<CTcpServerChildOscar>(pt);
                    break;
                case ENetProtocol::eStream:
                    actor = new CTcpServerActor<CTcpServerChildStream>(pt);
                    break;
                case ENetProtocol::eTelnet:
                    actor = new CTcpServerActor<CTcpServerChildTelnet>(pt);
                    break;
                case ENetProtocol::eEtfLog:
                    actor = new CTcpServerActor<CTcpServerChildEtfLog>(pt);
                    break;
            };
        }
        else if (actorName == CONSOLE)
        {
            LOG_DEBUG << "Try to console input handler";
            actor = new CConsoleActor(pt);
        }
        else if (actorName == UART_CLIENT)
        {
            LOG_DEBUG << "Try to open serial port: " << CTerminal::MakePortName(pt);
            actor = new CTerminal(pt);
        }
        else if (actorName == MANAGER_ADAPTOR)
        {
            LOG_DEBUG << "Try to create manager adaptor";
            actor = new CManagerAdaptor(pt);
        }
        else if (actorName == LOGGER_ADAPTOR)
        {
            switch (ConvertProtocolName2Id(actorProto))
            {
                case ENetProtocol::eOscar:
                    actor = new COscarLoggerAdaptor(pt);
                    break;
                case ENetProtocol::eStream:
                    actor = new CStreamLoggerAdaptor(pt);
                    break;
                case ENetProtocol::eEtfLog:
                    actor = new CEtfLoggerAdaptor(pt);
                    break;
                default:
                    APP_EXCEPTION_ERROR(GMSG << "Unsupported adaptor protocol: " << actorProto);
                    break;
            }
        }
        else
        {
            APP_EXCEPTION_ERROR(GMSG << "Unknown actor name: " << actorName);
        }

        LOG_DEBUG << "Created actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")";
    }
    catch (const boost::system::system_error & s)
    {
        LOG_ERR << "Cannot create actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")" << ": " << s.what();
    }
    catch (const std::exception& e)
    {
        LOG_ERR << "Cannot create actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")" << ": " << e.what();
    }

    return actor;
}
