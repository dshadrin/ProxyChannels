#pragma once
#include "log_adaptor.h"

//////////////////////////////////////////////////////////////////////////
class CEtfLoggerAdaptor : public CLogAdaptor
{
public:
    CEtfLoggerAdaptor(boost::property_tree::ptree& pt);
    virtual ~CEtfLoggerAdaptor() = default;

    void Start() override;
    void Stop() override;
    std::string GetName() const override;

    void DoLog(PMessage msg);

private:
    ENetProtocol m_protocol;
};