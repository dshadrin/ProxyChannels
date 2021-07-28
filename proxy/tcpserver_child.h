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
                             size_t inBuffSize,
                             size_t id,
                             signal_msg_t& sigInputMessage);

    virtual ~CTcpServerChild();

    virtual void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) = 0;
    virtual void ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) = 0;
    void WriteHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);

    boost::asio::ip::tcp::socket* GetSocket();
    std::vector<char>& GetBuffer();

    void Stop();
    void SendMessage(PMessage msg);
    size_t Id() const { return m_id; }

    signal_erase_t m_sigEraseMe;
    boost::signals2::connection m_outMsgConn;

protected:
    void DestroyMe(const boost::system::error_code &ec);

    signal_msg_t& m_sigInputMessage;

protected:
    size_t m_id;
    std::mutex m_mtxTcpClient;
    std::queue<PMessage> m_msgQueue;
    boost::asio::io_service& m_ioService;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    std::vector<char>::size_type m_inBuffSize;
    std::unique_ptr<std::vector<char>> m_inBuffer;
    std::vector<char> m_outBuffer;
    bool m_idleWrite;

    DECLARE_MODULE_TAG;
};

//////////////////////////////////////////////////////////////////////////
class CTcpServerChildStream : public CTcpServerChild
{
public:
    explicit CTcpServerChildStream(boost::asio::io_service& ioservice,
                                   boost::asio::ip::tcp::socket* socket,
                                   size_t inBuffSize,
                                   size_t id,
                                   signal_msg_t& sigInputMessage);

    virtual ~CTcpServerChildStream() = default;
    void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;
    void ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;
};

//////////////////////////////////////////////////////////////////////////
class CTcpServerChildTelnet : public CTcpServerChild
{
public:
    explicit CTcpServerChildTelnet(boost::asio::io_service& ioservice,
                                   boost::asio::ip::tcp::socket* socket,
                                   size_t inBuffSize,
                                   size_t id,
                                   signal_msg_t& sigInputMessage);

    virtual ~CTcpServerChildTelnet() = default;
    void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;
    void ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;
};

//////////////////////////////////////////////////////////////////////////
class CTcpServerChildOscar : public CTcpServerChild
{
public:
    explicit CTcpServerChildOscar(boost::asio::io_service& ioservice,
                                   boost::asio::ip::tcp::socket* socket,
                                   size_t inBuffSize,
                                   size_t id,
                                   signal_msg_t& sigInputMessage);

    virtual ~CTcpServerChildOscar() = default;
    void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;
    void ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;

private:
    size_t m_bodyReadBytes;
    size_t m_headerReadBytes;
    size_t m_bodySize;
};

//////////////////////////////////////////////////////////////////////////
class CTcpServerChildEtfLog : public CTcpServerChild
{
public:
    explicit CTcpServerChildEtfLog(boost::asio::io_service& ioservice,
                                  boost::asio::ip::tcp::socket* socket,
                                  size_t inBuffSize,
                                  size_t id,
                                  signal_msg_t& sigInputMessage);

    virtual ~CTcpServerChildEtfLog();
    void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;
    void ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred) override;

private:
    size_t m_bodyReadBytes;
    size_t m_headerReadBytes;
    size_t m_bodySize;
    signal_log_t m_toLog;
};
