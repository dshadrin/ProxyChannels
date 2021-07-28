// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "tcpserver.h"
#include "manager.h"
#include "oscar/flap_parser.h"
#include <boost/asio.hpp>

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CTcpServerChild, "TCPC");

//////////////////////////////////////////////////////////////////////////
CTcpServerChild::CTcpServerChild(boost::asio::io_service& ioservice,
                                 boost::asio::ip::tcp::socket* socket,
                                 size_t inBuffSize,
                                 size_t id,
                                 signal_msg_t& sigInputMessage) :
    m_sigInputMessage(sigInputMessage),
    m_id(id),
    m_ioService(ioservice),
    m_socket(socket),
    m_inBuffSize(inBuffSize),
    m_inBuffer(new std::vector<char>(m_inBuffSize)),
    m_idleWrite(true)
{

}

//////////////////////////////////////////////////////////////////////////
CTcpServerChild::~CTcpServerChild()
{
    LOG_INFO << "Destroyed TCP client (id = " << m_id << ")";
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::SendMessage(PMessage msg)
{
    std::unique_lock<std::mutex> lock(m_mtxTcpClient);
    if (!m_idleWrite)
    {
        m_msgQueue.push(msg);
    }
    else
    {
        m_outBuffer = *msg.get();
        m_socket->async_write_some(boost::asio::buffer(m_outBuffer), std::bind(&CTcpServerChild::WriteHandler, this, std::placeholders::_1, std::placeholders::_2));
        m_idleWrite = false;
    }
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::DestroyMe(const boost::system::error_code &ec)
{
    LOG_WARN << "Error async operation: " << ec.message();
    LOG_INFO << "TCP client (id = " << m_id << ") disconnected.";
    Stop();
    m_sigEraseMe(m_id);
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::WriteHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        std::unique_lock<std::mutex> lock(m_mtxTcpClient);
        if (bytesTransferred < m_outBuffer.size())
        {
            m_outBuffer.erase(m_outBuffer.begin(), m_outBuffer.begin() + bytesTransferred);
            m_socket->async_write_some(boost::asio::buffer(m_outBuffer), std::bind(&CTcpServerChild::WriteHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else if (!m_msgQueue.empty())
        {
            PMessage msg = m_msgQueue.back();
            m_msgQueue.pop();
            m_outBuffer = *msg.get();
            m_socket->async_write_some(boost::asio::buffer(m_outBuffer), std::bind(&CTcpServerChild::WriteHandler, this, std::placeholders::_1, std::placeholders::_2));
            m_idleWrite = false;
        }
        else
        {
            m_idleWrite = true;
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
boost::asio::ip::tcp::socket* CTcpServerChild::GetSocket()
{
    return m_socket.get();
}

//////////////////////////////////////////////////////////////////////////
std::vector<char>& CTcpServerChild::GetBuffer()
{
    return *m_inBuffer.get();
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::Stop()
{
    m_outMsgConn.disconnect();
    m_socket->shutdown(boost::asio::socket_base::shutdown_both);
    m_socket->close();
}

//////////////////////////////////////////////////////////////////////////
CTcpServerChildStream::CTcpServerChildStream(boost::asio::io_service& ioservice,
                                             boost::asio::ip::tcp::socket* socket,
                                             size_t inBuffSize,
                                             size_t id,
                                             signal_msg_t& sigInputMessage) :
    CTcpServerChild(ioservice, socket, inBuffSize, id, sigInputMessage)
{
    m_socket->async_read_some(boost::asio::buffer(GetBuffer()), std::bind(&CTcpServerChildStream::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildStream::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    ReadDataHandler(ec, bytesTransferred);
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildStream::ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_inBuffer->resize(bytesTransferred);
        PMessage msg(m_inBuffer.release());
        m_inBuffer.reset(new std::vector<char>(m_inBuffSize));
        m_socket->async_read_some(boost::asio::buffer(*m_inBuffer.get()), std::bind(&CTcpServerChildStream::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
        m_sigInputMessage(msg);
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
CTcpServerChildTelnet::CTcpServerChildTelnet(boost::asio::io_service& ioservice,
                                             boost::asio::ip::tcp::socket* socket,
                                             size_t inBuffSize,
                                             size_t id,
                                             signal_msg_t& sigInputMessage) :
    CTcpServerChild(ioservice, socket, inBuffSize, id, sigInputMessage)
{
    m_socket->async_read_some(boost::asio::buffer(GetBuffer()), std::bind(&CTcpServerChildTelnet::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildTelnet::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        if (*m_inBuffer->begin() == '\xff')
        {
            // TODO: handle control sequence
            m_socket->async_read_some(boost::asio::buffer(*m_inBuffer.get()), std::bind(&CTcpServerChildTelnet::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            ReadDataHandler(ec, bytesTransferred);
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildTelnet::ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_inBuffer->resize(bytesTransferred);
        PMessage msg(m_inBuffer.release());
        m_inBuffer.reset(new std::vector<char>(m_inBuffSize));
        m_socket->async_read_some(boost::asio::buffer(*m_inBuffer.get()), std::bind(&CTcpServerChildTelnet::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
        m_sigInputMessage(msg);
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
CTcpServerChildOscar::CTcpServerChildOscar(boost::asio::io_service& ioservice,
                                           boost::asio::ip::tcp::socket* socket,
                                           size_t inBuffSize,
                                           size_t id,
                                           signal_msg_t& sigInputMessage) :
    CTcpServerChild(ioservice, socket, inBuffSize, id, sigInputMessage),
    m_bodyReadBytes(0),
    m_headerReadBytes(0),
    m_bodySize(0)
{
    GetBuffer().resize(oscar::FLAP_HEADER_SIZE);
    m_socket->async_read_some(boost::asio::buffer(GetBuffer()), std::bind(&CTcpServerChildOscar::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildOscar::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        if (GetBuffer()[0] != oscar::FLAP_MARK)
        {
            LOG_ERR << "Input packet is not oscar protocol.";
            DestroyMe(ec);
        }
        else
        {
            m_headerReadBytes += bytesTransferred;
            if (m_headerReadBytes == oscar::FLAP_HEADER_SIZE)
            {
                m_bodyReadBytes = 0;
                m_bodySize = oscar::tlv::get_value_item<uint16_t>(&GetBuffer()[oscar::FLAP_DATA_SIZE_OFFSET]);
                m_inBuffer->resize(m_bodySize + oscar::FLAP_HEADER_SIZE);
                boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[oscar::FLAP_HEADER_SIZE], m_bodySize), std::bind(&CTcpServerChildOscar::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
            }
            else // m_headerReadBytes < oscar::FLAP_HEADER_SIZE
            {
                boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[m_headerReadBytes], oscar::FLAP_HEADER_SIZE - m_headerReadBytes), std::bind(&CTcpServerChildOscar::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
            }
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildOscar::ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_bodyReadBytes += bytesTransferred;
        if (m_bodyReadBytes < m_bodySize)
        {
            boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[m_bodyReadBytes], m_bodySize - m_bodyReadBytes), std::bind(&CTcpServerChildOscar::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            PMessage msg(m_inBuffer.release());
            m_headerReadBytes = 0;
            m_inBuffer.reset(new std::vector<char>(oscar::FLAP_HEADER_SIZE));
            boost::asio::async_read(*m_socket, boost::asio::buffer(GetBuffer(), oscar::FLAP_HEADER_SIZE), std::bind(&CTcpServerChildOscar::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
            m_sigInputMessage(msg);
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
CTcpServerChildEtfLog::CTcpServerChildEtfLog(boost::asio::io_service& ioservice,
                                             boost::asio::ip::tcp::socket* socket,
                                             size_t inBuffSize,
                                             size_t id,
                                             signal_msg_t& sigInputMessage) :
    CTcpServerChild(ioservice, socket, inBuffSize, id, sigInputMessage),
    m_bodyReadBytes(0),
    m_headerReadBytes(0),
    m_bodySize(0)
{
    PLog msg(new SLogPackage);
    msg->message = "Log";
    msg->tag = "rpc";
    msg->lchannel = LOG_CLIENT_CHANNEL;
    msg->timestamp = TS::GetTimestamp();
    msg->command = ELogCommand::eChangeFile;
    DirectSendToLogger(msg);

    m_socket->async_read_some(boost::asio::buffer(GetBuffer()), std::bind(&CTcpServerChildEtfLog::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
}


CTcpServerChildEtfLog::~CTcpServerChildEtfLog()
{
    PLog msg(new SLogPackage);
    msg->lchannel = LOG_CLIENT_CHANNEL;
    msg->timestamp = TS::GetTimestamp();
    msg->command = ELogCommand::eStop;
    DirectSendToLogger(msg);
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChildEtfLog::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        if (GetBuffer()[0] != 'L')
        {
            LOG_ERR << "Input packet is not ETF log protocol.";
            DestroyMe(ec);
        }
        else
        {
            m_headerReadBytes += bytesTransferred;
            if (m_headerReadBytes == ETF_LOG_HEADER_SIZE)
            {
                m_bodyReadBytes = 0;
                m_bodySize = reinterpret_cast<SEtfLogHeader*>(&GetBuffer()[0])->msgSize;
                if (m_bodySize > 0)
                {
                    m_inBuffer->resize(m_bodySize + ETF_LOG_HEADER_SIZE);
                    boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[ETF_LOG_HEADER_SIZE], m_bodySize), std::bind(&CTcpServerChildEtfLog::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
                }
                else
                {
                    ReadDataHandler(ec, m_bodySize);
                }
            }
            else // ETF_LOG_HEADER_SIZE
            {
                boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[m_headerReadBytes], ETF_LOG_HEADER_SIZE - m_headerReadBytes), std::bind(&CTcpServerChildEtfLog::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
            }
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

void CTcpServerChildEtfLog::ReadDataHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_bodyReadBytes += bytesTransferred;
        if (m_bodyReadBytes < m_bodySize)
        {
            boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[m_bodyReadBytes], m_bodySize - m_bodyReadBytes), std::bind(&CTcpServerChildEtfLog::ReadDataHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            PMessage msg(m_inBuffer.release());
            m_headerReadBytes = 0;
            m_inBuffer.reset(new std::vector<char>(ETF_LOG_HEADER_SIZE));
            boost::asio::async_read(*m_socket, boost::asio::buffer(GetBuffer(), oscar::FLAP_HEADER_SIZE), std::bind(&CTcpServerChildEtfLog::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
            m_sigInputMessage(msg);
        }
    }
    else
    {
        DestroyMe(ec);
    }
}
