#include "stdinc.h"
#include "tcpserver.h"
#include "manager.h"
#include <boost/asio.hpp>

/*
class ScopedLock
{
public:
    ScopedLock(boost::mutex& m, int id) : m_(m), id_(id)
    {
        LOG_INFO << "lock id: " << id_;
        m_.lock();
    }

    ~ScopedLock()
    {
        m_.unlock();
        LOG_INFO << "ulock id: " << id_;
    }

private:
    boost::mutex& m_;
    int id_;
    DECLARE_MODULE_TAG;
};
IMPLEMENT_MODULE_TAG(ScopedLock, "MTX");
*/

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CTcpServerActor, "TCPS");

CTcpServerActor::CTcpServerActor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_port(pt.get<uint16_t>("port", 2002)),
    m_protocol(ConvertProtocolName2Id(pt.get<std::string>("protocol", "RAW"))),
    m_ioService(CManager::instance()->IoService()),
    m_tcpEndpoint{ boost::asio::ip::tcp::v4(), m_port },
    m_tcpAcceptor{ m_ioService, m_tcpEndpoint },
    m_socket( new boost::asio::ip::tcp::socket(m_ioService) ),
    m_inClientBuffSize(pt.get<uint16_t>("input_buffer_size", 1024))
{
    assert(pt.get<std::string>("name") == GetTag());
}

CTcpServerActor::~CTcpServerActor()
{
    LOG_INFO << "Destroyed TCP client (on port = " << m_port << ")";
}

void CTcpServerActor::Start()
{
    m_tcpAcceptor.listen();
    m_tcpAcceptor.async_accept(*m_socket, std::bind(&CTcpServerActor::AcceptHandler, this, std::placeholders::_1));
    LOG_INFO << "TCP server on port " << m_port << " started (protocol = " << ConvertId2ProtocolName(m_protocol) << ").";
}

void CTcpServerActor::Stop()
{
    m_tcpAcceptor.close();
    m_socket->close();

    boost::mutex::scoped_lock lock(m_mtxTcpServer);
    for (std::list<CTcpServerChild>::iterator itClient = m_clients.begin(); itClient != m_clients.end(); )
    {
        LOG_INFO << "Erase client with id = " << itClient->Id();

        itClient->Stop();
        m_clients.erase(itClient++);
    }
    m_sigInputMessage.disconnect_all_slots();
    m_sigOutputMessage.disconnect_all_slots();
    LOG_INFO << "TCP server on port " << m_port << " stopped.";
}

void CTcpServerActor::EraseClient(size_t id)
{
    boost::mutex::scoped_lock lock(m_mtxTcpServer);
    const auto& it = std::find_if(m_clients.cbegin(), m_clients.cend(), [id](const CTcpServerChild& client) -> bool
    {
        return client.Id() == id;
    });

    if (it != m_clients.end())
    {
        LOG_INFO << "Erase client with id = " << id;
        m_clients.erase(it);
    }
}

void CTcpServerActor::AcceptHandler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        boost::system::error_code err;
        LOG_INFO << "TCP client connected: " << m_socket->remote_endpoint(err).address().to_string() << ":" << m_socket->remote_endpoint(err).port() << " -> "
                                             << m_socket->local_endpoint(err).address().to_string() << ":" << m_socket->local_endpoint(err).port();

        const size_t id = GenerateId();
        boost::mutex::scoped_lock lock(m_mtxTcpServer);
        m_clients.emplace_back(m_ioService, m_socket.release(), m_inClientBuffSize, id, m_sigInputMessage, m_protocol);

        CTcpServerChild& client = m_clients.back();
        client.m_sigEraseMe.connect(std::bind(&CTcpServerActor::EraseClient, this, std::placeholders::_1));
        client.m_outMsgConn = m_sigOutputMessage.connect(std::bind(&CTcpServerChild::SendMessage, &client, std::placeholders::_1));

        LOG_INFO << "Created client with id = " << id;
    }
    else
    {
        LOG_WARN << "Error accept connection (CTcpStringServerActor): " << ec.message();
    }

    m_socket.reset(new boost::asio::ip::tcp::socket(m_ioService));
    m_tcpAcceptor.async_accept(*m_socket, std::bind(&CTcpServerActor::AcceptHandler, this, std::placeholders::_1));
}

size_t CTcpServerActor::GenerateId()
{
    static boost::mutex mtx;
    static size_t counter = 0;
    boost::mutex::scoped_lock lock(mtx);
    return counter++;
}
