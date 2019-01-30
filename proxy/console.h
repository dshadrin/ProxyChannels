#pragma once
#include "actor.h"
#include "manager.h"
#include <boost/asio.hpp>

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
#else
#warning CConsoleActor is not worked
#endif

    DECLARE_MODULE_TAG;
};
