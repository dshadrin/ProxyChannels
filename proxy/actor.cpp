/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

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
ENetProtocol ConvertProtocolName2Id( const std::string& pName )
{
    return ConvertString2Id<ENetProtocol>( pName );
}

std::string ConvertId2ProtocolName( ENetProtocol pId )
{
    return ConvertId2String<ENetProtocol>( pId );
}

//////////////////////////////////////////////////////////////////////////
// factory method
CActor* CActor::MakeActor(boost::property_tree::ptree& pt)
{
    DEFINE_MODULE_TAG("ACTR");
    CActor* actor = nullptr;
    std::string actorName;
    std::string actorProto;
    std::string actorId;

    try
    {
        actorName = pt.get<std::string>("name", "NO_NAME");
        EActorType aName = ConvertString2Id<EActorType>( actorName );

        actorProto = pt.get<std::string>("protocol", "NO_PROTO");
        ENetProtocol protocol = ConvertString2Id<ENetProtocol>( actorProto );

        actorId = pt.get<std::string>("id", "");
        LOG_DEBUG << "Begin creating actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")";

        switch (aName)
        {
        case EActorType::TCP_SERVER:
            switch (protocol)
            {
            case ENetProtocol::OSCAR:
                actor = new CTcpServerActor<CTcpServerChildOscar>( pt );
                break;
            case ENetProtocol::STREAM:
                actor = new CTcpServerActor<CTcpServerChildStream>( pt );
                break;
            case ENetProtocol::TELNET:
                actor = new CTcpServerActor<CTcpServerChildTelnet>( pt );
                break;
            case ENetProtocol::ETFLOG:
                actor = new CTcpServerActor<CTcpServerChildEtfLog>( pt );
                break;
            }
            break;
        case EActorType::CONSOLE_SERVER:
            LOG_DEBUG << "Try to console input handler";
            actor = new CConsoleActor( pt );
            break;
        case EActorType::UART_CLIENT:
            LOG_DEBUG << "Try to open serial port: " << CTerminal::MakePortName( pt );
            actor = new CTerminal( pt );
            break;
        case EActorType::MANAGER_ADAPTOR:
            LOG_DEBUG << "Try to create manager adaptor";
            actor = new CManagerAdaptor( pt );
            break;
        case EActorType::LOGGER_ADAPTOR:
            switch (protocol)
            {
            case ENetProtocol::OSCAR:
                actor = new COscarLoggerAdaptor( pt );
                break;
            case ENetProtocol::STREAM:
                actor = new CStreamLoggerAdaptor( pt );
                break;
            case ENetProtocol::ETFLOG:
                actor = new CEtfLoggerAdaptor( pt );
                break;
            default:
                APP_EXCEPTION_ERROR( GMSG << "Unsupported adaptor protocol: " << actorProto );
                break;
            }
            break;
        default:
            APP_EXCEPTION_ERROR( GMSG << "Unknown actor name: " << actorName );
        }

        LOG_DEBUG << "Created actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")";
    }
    catch (std::exception& e)
    {
        LOG_ERR << "Cannot create actor " << actorName << "(protocol = " << actorProto << ", id = " << actorId << ")" << ": " << e.what();
    }

    return actor;
}
