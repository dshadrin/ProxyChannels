#pragma once
#include "actor.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CTcpServerChild
{
public:
    explicit CTcpServerChild(boost::asio::io_service& ioservice,
                              boost::asio::ip::tcp::socket* socket,
                              size_t inBuffSize, size_t id,
                              signal_msg_t& sigInputMessage,
                              ENetProtocol protocol);
    ~CTcpServerChild();

    void OscarHeaderReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);
    void OscarBodyReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);
    void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);
    void FirstReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);
    void WriteHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);

    boost::asio::ip::tcp::socket* GetSocket();
    std::vector<char>& GetBuffer();

    void Stop();
    void ReceiveMessage(PMessage msg);
    void SendMessage(PMessage msg);
    size_t Id() const { return m_id; }

    signal_erase_t m_sigEraseMe;
    boost::signals2::connection m_outMsgConn;

private:
    void DestroyMe(const boost::system::error_code &ec);

    signal_msg_t& m_sigInputMessage;

private:
    size_t m_id;
    ENetProtocol m_protocol;
    boost::mutex m_mtxTcpClient;
    std::queue<PMessage> m_msgQueue;
    boost::asio::io_service& m_ioService;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    std::vector<char>::size_type m_inBuffSize;
    std::unique_ptr<std::vector<char>> m_inBuffer;
    std::vector<char> m_outBuffer;
    size_t m_bodyReadedBytes;
    size_t m_bodySize;
    bool m_idleWrite;
    bool m_partialReaded;

    DECLARE_MODULE_TAG;
};
