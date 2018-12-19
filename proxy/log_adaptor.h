#pragma once

#include "actor.h"
#include "log_channel.h"

//////////////////////////////////////////////////////////////////////////
class CLogAdaptor : public CActor
{
public:
    CLogAdaptor(boost::property_tree::ptree& pt);
    virtual ~CLogAdaptor();

protected:
    signal_log_t m_toLog;

private:
    std::vector<CLogChannel> m_channels;
};
