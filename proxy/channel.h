/*
 * Copyright (C) 2018 Rhonda Software.
 * All rights reserved.
 */

//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////
#include "actor.h"
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
class CChannel
{
public:
    explicit CChannel(boost::property_tree::ptree& pt);
    virtual ~CChannel();

    const std::string& GetId() const { return m_id; }
    void Connect();
    void Disconnect();

    void InputMessages(PMessage msg);

private:
    std::mutex m_mtx;
    std::string m_id;
    std::weak_ptr<CActor> m_source;
    std::weak_ptr<CActor> m_destination;
    boost::signals2::connection m_connectionSource;
    DECLARE_MODULE_TAG;
};

//////////////////////////////////////////////////////////////////////////
