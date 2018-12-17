#pragma once
#include "actor.h"
#include "tcpserver_child.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
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
    std::list<CTcpServerChild> m_clients;

    DECLARE_MODULE_TAG;
};
