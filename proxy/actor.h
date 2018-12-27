#pragma once

//////////////////////////////////////////////////////////////////////////
#define NO_NAME "NO_NAME"
#define TCP_SERVER "TCP-SERVER"
#define UART_CLIENT "UART-CLIENT"
#define LOGGER_ADAPTOR "LOGGER-ADAPTOR"

#define NO_PROTO "NO_PROTO"
#define PROTO_OSCAR "OSCAR"
#define PROTO_STREAM "STREAM"
#define PROTO_TELNET "TELNET"

//////////////////////////////////////////////////////////////////////////
enum class ENetProtocol : uint8_t
{
    eOscar,
    // insert new protocols before this line
    eStream,
    eTelnet
};

ENetProtocol ConvertProtocolName2Id(const std::string& pName);
std::string ConvertId2ProtocolName(ENetProtocol pId);

//////////////////////////////////////////////////////////////////////////
class CActor
{
public:
    CActor(const std::string& tag, size_t id) : m_tag(tag), m_id(id) {}
    virtual ~CActor() = default;

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual std::string GetName() const = 0;
    const std::string& GetTag() const { return m_tag; }
    size_t GetId() const { return m_id; }

    signal_msg_t m_sigInputMessage;
    signal_msg_t m_sigOutputMessage;

    static CActor* MakeActor(boost::property_tree::ptree& pt);

protected:
    const std::string m_tag;
    const size_t m_id;
};
