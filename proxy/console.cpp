// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "console.h"
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR) || defined(WIN32)

#ifdef WIN32
#include <conio.h>
#endif

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CConsoleActor, "CON ");

//////////////////////////////////////////////////////////////////////////

CConsoleActor::CConsoleActor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id")),
    m_ioService(CManager::instance()->IoService()),
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
    m_in(m_ioService, ::dup(STDIN_FILENO)),
    m_inBuffer(MAX_BODY_LENGTH)
#elif defined WIN32
    m_delayNs( pt.get<uint32_t>( "timer-delay", 100 ) * 1000 * 1000 ),
    m_steadyTimer(m_ioService, std::chrono::nanoseconds( m_delayNs ))
#endif
{
    LOG_DEBUG << "ID is " << pt.get<size_t>( "id" );
    LOG_DEBUG << "Delay is " << pt.get<uint32_t>( "timer-delay", 100 ) << "ms";
}

void CConsoleActor::ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred)
{
    if (!ec || ec == boost::asio::error::not_found)
    {
        std::string inStr;
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
        inStr.resize(bytesTransferred);
        m_inBuffer.sgetn((char*)inStr.data(), bytesTransferred);
        Start();
#elif defined WIN32
        Start();
        while ( _kbhit() != 0 )
        {
            char ch = static_cast<char>( _getche() );
            if ( ch == 0 || ch == 0xE0 )
            {
                ch = _getche();
            }
            else
            {
                if ( ch == '\r' )
                {
                    inStr.push_back( ch );
                    ch = '\n';
                    _putch( ch );
                }
                inStr.push_back( ch );
            }
        }
#endif
        if ( !inStr.empty() )
        {
            PMessage msg = std::make_shared<std::vector<char>>( inStr.begin(), inStr.end() );
            m_sigInputMessage( msg );
        }
    }
    else
    {
        Stop();
    }
}

void CConsoleActor::Stop()
{
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
    m_in.close();
#elif defined WIN32
    m_steadyTimer.cancel();
#endif
}

void CConsoleActor::Start()
{
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
    boost::asio::async_read_until(m_in, m_inBuffer, '\n',
                                  boost::bind(&CConsoleActor::ReadHandler, this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
#elif defined WIN32
    m_steadyTimer.expires_from_now( std::chrono::nanoseconds( m_delayNs ) );
    m_steadyTimer.async_wait( boost::bind( &CConsoleActor::ReadHandler, this, boost::asio::placeholders::error, 0 ) );
#endif
}
#endif
