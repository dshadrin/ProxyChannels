// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
#include "stdinc.h"
#include "console.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CConsoleActor, "CON ");

//////////////////////////////////////////////////////////////////////////

CConsoleActor::CConsoleActor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_ioService(CManager::instance()->IoService())
    ,m_in(m_ioService, ::dup(STDIN_FILENO))
    ,m_inBuffer(MAX_BODY_LENGTH)
{

}

void CConsoleActor::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    std::string inStr;
    if (!ec)
    {
        // Write the message (minus the newline) to the server.
        inStr.resize(bytesTransferred - 1);
        m_inBuffer.sgetn(inStr.data(), bytesTransferred - 1);
        m_inBuffer.consume(1); // Remove newline from input.

        LOG_DEBUG << "Read from console(1): " << inStr;
        Start();
    }
    else if (ec == boost::asio::error::not_found)
    {
        // Didn't get a newline. Send whatever we have.
        inStr.resize(bytesTransferred);
        m_inBuffer.sgetn(inStr.data(), bytesTransferred);

        LOG_DEBUG << "Read from console(2): " << inStr;
        Start();
    }
    else
    {
        Stop();
    }
}

void CConsoleActor::Stop()
{
    m_in.close();
}

void CConsoleActor::Start()
{
    boost::asio::async_read_until(m_in, m_inBuffer, '\n',
                                  boost::bind(&CConsoleActor::ReadHandler, this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
}
#endif
