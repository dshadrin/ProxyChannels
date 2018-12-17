#pragma once
#include "actor.h"

//////////////////////////////////////////////////////////////////////////
class CNetLoggerAdaptor : public CActor
{
public:
    CNetLoggerAdaptor(boost::property_tree::ptree& pt);
    virtual ~CNetLoggerAdaptor() = default;

    void Start() override;
    void Stop() override;
    std::string GetName() const override;

    void DoLog(PMessage msg);

private:
    ENetProtocol m_protocol;
};