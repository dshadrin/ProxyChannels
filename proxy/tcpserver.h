#pragma once
#include "actor.h"
#include "manager.h"
#include "tcpserver_child.h"
#include <boost/asio.hpp>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template<class _Client>
class CTcpServerActor : public CActor
{
public:
    CTcpServerActor(boost::property_tree::ptree& pt);
    virtual ~CTcpServerActor();

    void Start() override;
    void Stop() override;
    std::string GetName() const override { return GetTag() + ":" + std::to_string(m_port); }

private:
    void AcceptHandler(const boost::system::error_code &ec);
    void EraseClient(size_t id);
    static size_t GenerateId();

private:
    uint16_t m_port;
    ENetProtocol m_protocol;
    boost::asio::io_service& m_ioService;
    boost::asio::ip::tcp::endpoint m_tcpEndpoint;
    boost::asio::ip::tcp::acceptor m_tcpAcceptor;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    std::unique_ptr<boost::thread> m_svrThread;
    size_t m_inClientBuffSize;

    boost::mutex m_mtxTcpServer;
    std::list<_Client> m_clients;

    DECLARE_MODULE_TAG;
};

// TODO: move to some cpp file
template<class _Client>
IMPLEMENT_MODULE_TAG(CTcpServerActor<_Client>, "TCPS");

template<class _Client>
CTcpServerActor<_Client>::CTcpServerActor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_port(pt.get<uint16_t>("port", 23)),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", PROTO_TELNET))),
    m_ioService(CManager::instance()->IoService()),
    m_tcpEndpoint{ boost::asio::ip::tcp::v4(), m_port },
    m_tcpAcceptor{ m_ioService, m_tcpEndpoint },
    m_socket(new boost::asio::ip::tcp::socket(m_ioService)),
    m_inClientBuffSize(pt.get<uint16_t>("input_buffer_size", 1024))
{
    assert(pt.get<std::string>("name") == GetTag());
}

template<class _Client>
CTcpServerActor<_Client>::~CTcpServerActor()
{
    LOG_INFO << "Destroyed TCP client (on port = " << m_port << ")";
}

template<class _Client>
void CTcpServerActor<_Client>::Stop()
{
    m_tcpAcceptor.close();
    m_socket->close();

    boost::mutex::scoped_lock lock(m_mtxTcpServer);
    for (typename std::list<_Client>::iterator itClient = m_clients.begin(); itClient != m_clients.end(); )
    {
        LOG_INFO << "Erase client with id = " << itClient->Id();

        itClient->Stop();
        m_clients.erase(itClient++);
    }
    m_sigInputMessage.disconnect_all_slots();
    m_sigOutputMessage.disconnect_all_slots();
    LOG_INFO << "TCP server on port " << m_port << " stopped.";
}

template<class _Client>
void CTcpServerActor<_Client>::EraseClient(size_t id)
{
    boost::mutex::scoped_lock lock(m_mtxTcpServer);
    const auto& it = std::find_if(m_clients.cbegin(), m_clients.cend(), [id](const _Client& client) -> bool
    {
        return client.Id() == id;
    });

    if (it != m_clients.end())
    {
        LOG_INFO << "Erase client with id = " << id;
        m_clients.erase(it);
    }
}

template<class _Client>
void CTcpServerActor<_Client>::AcceptHandler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        boost::system::error_code err;
        LOG_INFO << "TCP client connected: " << m_socket->remote_endpoint(err).address().to_string() << ":" << m_socket->remote_endpoint(err).port() << " -> "
            << m_socket->local_endpoint(err).address().to_string() << ":" << m_socket->local_endpoint(err).port();

        const size_t id = GenerateId();
        boost::mutex::scoped_lock lock(m_mtxTcpServer);
        m_clients.emplace_back(m_ioService, m_socket.release(), m_inClientBuffSize, id, m_sigInputMessage);

        _Client& client = m_clients.back();
        client.m_sigEraseMe.connect(std::bind(&CTcpServerActor::EraseClient, this, std::placeholders::_1));
        client.m_outMsgConn = m_sigOutputMessage.connect(std::bind(&_Client::SendMessage, &client, std::placeholders::_1));

        LOG_INFO << "Created client with id = " << id;
    }
    else
    {
        LOG_WARN << "Error accept connection (CTcpServerActor): " << ec.message();
    }

    m_socket.reset(new boost::asio::ip::tcp::socket(m_ioService));
    m_tcpAcceptor.async_accept(*m_socket, std::bind(&CTcpServerActor::AcceptHandler, this, std::placeholders::_1));
}

template<class _Client>
void CTcpServerActor<_Client>::Start()
{
    m_tcpAcceptor.listen();
    m_tcpAcceptor.async_accept(*m_socket, std::bind(&CTcpServerActor::AcceptHandler, this, std::placeholders::_1));
    LOG_INFO << "TCP server on port " << m_port << " started (protocol = " << ConvertId2ProtocolName(m_protocol) << ").";
}

template<class _Client>
size_t CTcpServerActor<_Client>::GenerateId()
{
    static boost::mutex mtx;
    static size_t counter = 0;
    boost::mutex::scoped_lock lock(mtx);
    return counter++;
}
