// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "console.h"
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CConsoleActor, "CON ");

//////////////////////////////////////////////////////////////////////////

CConsoleActor::CConsoleActor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_ioService(CManager::instance()->IoService()),
    m_in(m_ioService, ::dup(STDIN_FILENO)),
    m_inBuffer(MAX_BODY_LENGTH)
{

}

void CConsoleActor::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    std::string inStr;
    if (!ec || ec == boost::asio::error::not_found)
    {
        inStr.resize(bytesTransferred);
        m_inBuffer.sgetn((char*)inStr.data(), bytesTransferred);
        PMessage msg = std::make_shared<std::vector<char>>(std::vector<char>(inStr.begin(), inStr.end()));
        LOG_DEBUG << "Read from console: " << inStr;
        Start();
        m_sigInputMessage(msg);
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
