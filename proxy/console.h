#pragma once

#include <boost/asio.hpp>
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR) || defined(WIN32)

#include "actor.h"
#include "manager.h"

#ifdef WIN32
#include <boost/asio/steady_timer.hpp>
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CConsoleActor : public CActor
{
public:
    CConsoleActor(boost::property_tree::ptree& pt);
    virtual ~CConsoleActor() = default;

    void Start() override;
    void Stop() override;
    std::string GetName() const override { return CONSOLE; }

private:
    void ReadHandler(const boost::system::error_code &ec, std::size_t bytesTransferred);

private:
    enum { MAX_BODY_LENGTH = 512 };
    boost::asio::io_service& m_ioService;

#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
    boost::asio::posix::stream_descriptor m_in;
    boost::asio::streambuf m_inBuffer;
#elif defined WIN32
    size_t m_delayNs;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> m_steadyTimer;
#endif

    DECLARE_MODULE_TAG;
};
#else
#include "tcpserver.h"
typedef CTcpServerActor<CTcpServerChildTelnet> CConsoleActor;
#endif
