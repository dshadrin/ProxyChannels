// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*
 * Copyright (C) 2014-2017 Rhonda Software.
 * All rights reserved.
 */

#include "stdinc.h"
#include "channel.h"
#include "manager.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CChannel, "CHNL");

CChannel::CChannel(boost::property_tree::ptree& pt) :
    m_id(pt.get<size_t>("id")) // can be throw if config incorrect
{
    CManager* manager = CManager::instance();
    size_t sourceId = pt.get<size_t>("source");     // can be throw if config incorrect
    size_t destId = pt.get<size_t>("destination");   // can be throw if config incorrect
    m_source = manager->FindActor(sourceId);
    if (m_source.expired())
    {
        APP_EXCEPTION_ERROR(GMSG << "Actor with id = " << sourceId << " was not registered.");
    }
    m_destination = manager->FindActor(destId);
    if (m_destination.expired())
    {
        APP_EXCEPTION_ERROR(GMSG << "Actor with id = " << destId << " was not registered.");
    }
}

//////////////////////////////////////////////////////////////////////////
CChannel::~CChannel()
{
    Disconnect();
}

void CChannel::Connect()
{
    m_connectionSource = m_source.lock()->m_sigInputMessage.connect(std::bind(&CChannel::InputMessages, this, std::placeholders::_1));
    LOG_INFO << "Channel from " << m_source.lock()->GetName() << " to " << m_destination.lock()->GetName() << " (id = " << m_destination.lock()->GetId() << ")";
}

void CChannel::Disconnect()
{
    m_connectionSource.disconnect();
}

void CChannel::InputMessages(PMessage msg)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if (!m_destination.expired())
    {
        auto dst = m_destination.lock();
        if (dst)
        {
            dst->m_sigOutputMessage(msg);
        }
    }
}
