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

    void Start() override {}
    void Stop() override {}
    std::string GetName() const override { return "STDIN"; }

private:
    boost::asio::io_service& m_ioService;

#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
    boost::asio::posix::stream_descriptor m_in;
#endif

    DECLARE_MODULE_TAG;
};
