#include "stdinc.h"
#include "tcpserver.h"
#include "manager.h"
#include "oscar/flap_parser.h"
#include <boost/asio.hpp>

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CTcpServerChild, "TCPC");

//////////////////////////////////////////////////////////////////////////
CTcpServerChild::CTcpServerChild(boost::asio::io_service& ioservice,
                                 boost::asio::ip::tcp::socket* socket, size_t inBuffSize, size_t id,
                                 signal_msg_t& sigInputMessage, ENetProtocol protocol) :
    m_sigInputMessage(sigInputMessage),
    m_id(id),
    m_protocol(protocol),
    m_ioService(ioservice),
    m_socket(socket),
    m_inBuffSize(inBuffSize),
    m_inBuffer(new std::vector<char>(m_inBuffSize)),
    m_bodyReadedBytes(0),
    m_bodySize(0),
    m_idleWrite(true),
    m_partialReaded(false)
{
    switch (m_protocol)
    {
    case ENetProtocol::eTelnet:
        m_socket->async_read_some(boost::asio::buffer(GetBuffer()), std::bind(&CTcpServerChild::FirstReadHandler, this, std::placeholders::_1, std::placeholders::_2));
        break;
    case ENetProtocol::eOscar:
        GetBuffer().resize(oscar::FLAP_HEADER_SIZE);
        boost::asio::async_read(*m_socket, boost::asio::buffer(GetBuffer(), oscar::FLAP_HEADER_SIZE), std::bind(&CTcpServerChild::OscarHeaderReadHandler, this, std::placeholders::_1, std::placeholders::_2));
        break;
    case ENetProtocol::eRaw:
    default:
        m_socket->async_read_some(boost::asio::buffer(GetBuffer()), std::bind(&CTcpServerChild::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
    }
}

//////////////////////////////////////////////////////////////////////////
CTcpServerChild::~CTcpServerChild()
{
    LOG_INFO << "Destroyed TCP client (id = " << m_id << ")";
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::SendMessage(PMessage msg)
{
    boost::mutex::scoped_lock lock(m_mtxTcpClient);
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
void CTcpServerChild::OscarHeaderReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        if (!m_partialReaded)
        {
            if (GetBuffer()[0] != oscar::FLAP_MARK)
            {
                LOG_ERR << "Input packet is not oscar protocol.";
                DestroyMe(ec);
            }
            else
            {
                if ((!m_partialReaded && bytesTransferred == oscar::FLAP_HEADER_SIZE) || m_partialReaded)
                {
                    m_bodyReadedBytes = 0;
                    m_bodySize = oscar::tlv::get_value_item<uint16_t>(&GetBuffer()[oscar::FLAP_DATA_SIZE_OFFSET]);
                    m_inBuffer->resize(m_bodySize + oscar::FLAP_HEADER_SIZE);
                    boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[oscar::FLAP_HEADER_SIZE], m_bodySize), std::bind(&CTcpServerChild::OscarBodyReadHandler, this, std::placeholders::_1, std::placeholders::_2));
                }
                else if ((!m_partialReaded && bytesTransferred < oscar::FLAP_HEADER_SIZE))
                {
                    PMessage msg(m_inBuffer.release());
                    m_inBuffer.reset(new std::vector<char>(oscar::FLAP_HEADER_SIZE));
                    m_partialReaded = true;
                    boost::asio::async_read(*m_socket, boost::asio::buffer(GetBuffer(), oscar::FLAP_HEADER_SIZE), std::bind(&CTcpServerChild::OscarHeaderReadHandler, this, std::placeholders::_1, std::placeholders::_2));
                    m_sigInputMessage(msg);
                }
            }
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::OscarBodyReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_bodyReadedBytes += bytesTransferred;
        if (m_bodyReadedBytes < m_bodySize)
        {
            boost::asio::async_read(*m_socket, boost::asio::buffer(&GetBuffer()[m_bodyReadedBytes], m_bodySize - m_bodyReadedBytes), std::bind(&CTcpServerChild::OscarBodyReadHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            PMessage msg(m_inBuffer.release());
            m_inBuffer.reset(new std::vector<char>(m_inBuffSize));

            m_sigInputMessage(msg);

            m_partialReaded = false;
            GetBuffer().resize(oscar::FLAP_HEADER_SIZE);
            boost::asio::async_read(*m_socket, boost::asio::buffer(GetBuffer(), oscar::FLAP_HEADER_SIZE), std::bind(&CTcpServerChild::OscarHeaderReadHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        m_inBuffer->resize(bytesTransferred);
        PMessage msg(m_inBuffer.release());
        m_inBuffer.reset(new std::vector<char>(m_inBuffSize));

        m_sigInputMessage(msg);

        m_socket->async_read_some(boost::asio::buffer(*m_inBuffer.get()), std::bind(&CTcpServerChild::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        DestroyMe(ec);
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
void CTcpServerChild::FirstReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        if (*m_inBuffer->begin() == '\xff')
        {
            // TODO: handle control sequence
            m_socket->async_read_some(boost::asio::buffer(*m_inBuffer.get()), std::bind(&CTcpServerChild::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            ReadHandler(ec, bytesTransferred);
        }
    }
    else
    {
        DestroyMe(ec);
    }
}

//////////////////////////////////////////////////////////////////////////
void CTcpServerChild::WriteHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec)
    {
        boost::mutex::scoped_lock lock(m_mtxTcpClient);
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
