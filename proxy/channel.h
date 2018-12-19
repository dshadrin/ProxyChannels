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

    size_t GetId() const { return m_id; }
    void Connect();
    void Disconnect();

    void InputMessages(PMessage msg);

private:
    boost::mutex m_mtx;
    size_t m_id;
    std::weak_ptr<CActor> m_source;
    std::weak_ptr<CActor> m_destination;
    boost::signals2::connection m_connectionSource;
    DECLARE_MODULE_TAG;
};

//////////////////////////////////////////////////////////////////////////
