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
    std::string m_tag;
    std::shared_ptr<SLogPackage> m_package;

    ENetProtocol m_protocol;
    ESeverity m_severity;
    int8_t m_logChannel;

private:
    struct SWork
    {
        std::string buffer;
        timespec timestamp;
    } m_work;
};