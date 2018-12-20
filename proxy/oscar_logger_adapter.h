#pragma once
#include "log_adaptor.h"

//////////////////////////////////////////////////////////////////////////
class COscarLoggerAdaptor : public CLogAdaptor
{
public:
    COscarLoggerAdaptor(boost::property_tree::ptree& pt);
    virtual ~COscarLoggerAdaptor() = default;

    void Start() override;
    void Stop() override;
    std::string GetName() const override;

    void DoLog(PMessage msg);

private:
    ENetProtocol m_protocol;
};