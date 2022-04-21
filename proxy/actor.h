#pragma once
#include <boost/describe.hpp>

//////////////////////////////////////////////////////////////////////////
BOOST_DEFINE_ENUM_CLASS( EActorType, NO_NAME, TCP_SERVER, UART_CLIENT, LOGGER_ADAPTOR, CONSOLE_SERVER, MANAGER_ADAPTOR );
BOOST_DEFINE_ENUM_CLASS( ENetProtocol, NO_PROTO, OSCAR, STREAM, TELNET, ETFLOG );

//////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )
struct SEtfLogHeader
{
    uint8_t  tag[3];
    uint8_t  type;
    uint8_t  severity;
    uint8_t  module[3];
    uint32_t msgSize;
};
#pragma pack( pop )
const size_t ETF_LOG_HEADER_SIZE = sizeof(SEtfLogHeader);

template<class _Enum>
std::string ConvertId2String( _Enum id )
{
    char name[64]{};
    return boost::describe::enum_to_string<_Enum>( id, &name[0] );;
}

template<class _Enum>
_Enum ConvertString2Id( const std::string& str )
{
    _Enum id;
    std::string s = boost::algorithm::to_upper_copy( str );
    boost::algorithm::trim( s );
    boost::describe::enum_from_string<_Enum>( s.c_str(), id );
    return id;
}

ENetProtocol ConvertProtocolName2Id(const std::string& pName);
std::string ConvertId2ProtocolName(ENetProtocol pId);

//////////////////////////////////////////////////////////////////////////
class CActor
{
public:
    CActor(const std::string& tag, const std::string& id) : m_tag(tag), m_id(id) {}
    virtual ~CActor() = default;

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual std::string GetName() const = 0;
    const std::string& GetTag() const { return m_tag; }
    const std::string& GetId() const { return m_id; }

    signal_msg_t m_sigInputMessage;
    signal_msg_t m_sigOutputMessage;

    static CActor* MakeActor(boost::property_tree::ptree& pt);

protected:
    const std::string m_tag;
    const std::string m_id;
};
