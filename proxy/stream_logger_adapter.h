#pragma once
#include "log_adaptor.h"

//////////////////////////////////////////////////////////////////////////
class CStreamLoggerAdaptor : public CLogAdaptor
{
public:
    CStreamLoggerAdaptor(boost::property_tree::ptree& pt);
    virtual ~CStreamLoggerAdaptor() = default;

    void Start() override;
    void Stop() override;
    std::string GetName() const override;

    void DoLog(PMessage msg);

private:
    ENetProtocol m_protocol;
    SLogPackage m_package;

private:
    struct SWork
    {
        std::string buffer;
        timespec timestamp;
    } m_work;
};