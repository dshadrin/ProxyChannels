#pragma once
#include "actor.h"

//////////////////////////////////////////////////////////////////////////
class CRawLoggerAdaptor : public CActor
{
public:
    CRawLoggerAdaptor(boost::property_tree::ptree& pt);
    virtual ~CRawLoggerAdaptor() = default;

    void Start() override;
    void Stop() override;
    std::string GetName() const override;

    void DoLog(PMessage msg);

private:
    ENetProtocol m_protocol;
    bool m_filterESC;
    SLogPackage m_package;

private:
    struct SWork
    {
        std::string buffer;
        timespec timestamp;
    } m_work;
};