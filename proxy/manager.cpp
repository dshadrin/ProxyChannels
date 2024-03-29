/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "stdinc.h"
#include "manager.h"
#include "configurator.h"
#include "actor.h"
#include "channel.h"

//////////////////////////////////////////////////////////////////////////
std::mutex CManager::sManagerMtx;
std::unique_ptr<CManager> CManager::s_Manager;
IMPLEMENT_MODULE_TAG(CManager, "MGR");

//////////////////////////////////////////////////////////////////////////
CManager::CManager() :
    m_sigHandler({SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM}),
    m_tp(),
    m_ioFlag(false),
    m_stopFlag(false)
{
}

//////////////////////////////////////////////////////////////////////////
CManager::~CManager()
{
}

//////////////////////////////////////////////////////////////////////////
CManager * CManager::instance()
{
    return !s_Manager ? nullptr : s_Manager.get();
}

//////////////////////////////////////////////////////////////////////////
void CManager::init()
{
    if (!s_Manager)
    {
        {
            std::unique_lock<std::mutex> lock(sManagerMtx);
            if (!s_Manager)
            {
                CConfigurator::init();
                s_Manager.reset(new CManager());
            }
        }

        CLogger::Get()->Start();

        LOG_INFO << "Manager started";
        s_Manager->m_tp.SetWorkUnit(std::bind(&CManager::AsioServiceWork, s_Manager.get()), true);
        s_Manager->m_tp.SetWorkUnit(std::bind(&CManager::AsioServiceWork, s_Manager.get()), true);

        s_Manager->RunActors();
        s_Manager->SetChannels();

        // blocks while stop
        std::unique_lock<std::mutex> stopLock(s_Manager->m_stopMtx);
        s_Manager->m_stopCond.wait(stopLock, []() -> bool
        {
            return s_Manager->m_stopFlag;
        });
    }
}

//////////////////////////////////////////////////////////////////////////
void CManager::reset()
{
    if (s_Manager)
    {
        // stop io_service
        if (!s_Manager->m_ioService.stopped())
        {
            s_Manager->m_ioService.stop();
            // wait finish io_service
            std::unique_lock<std::mutex> lock(s_Manager->m_ioMtx);
            while (s_Manager->m_ioFlag)
            {
                s_Manager->m_ioCond.wait(lock);
            }
        }

        // Erase actors
        {
            std::unique_lock<std::mutex> lock(s_Manager->m_actorsMtx);
            for (auto& actor : s_Manager->m_actors)
            {
                actor.second->Stop();
            }
            s_Manager->m_actors.clear();
        }

        LOG_INFO << "Manager finished";
        CLogger::Get()->Stop();
        s_Manager.reset();
    }
}


void CManager::Stop()
{
    std::unique_lock<std::mutex> lock(m_stopMtx);
    m_stopFlag = true;
    m_stopCond.notify_all();
}

std::weak_ptr<CActor> CManager::FindActor( std::string id)
{
    auto it = m_actors.find(id);
    return (it != m_actors.end()) ? it->second : nullptr;
}

//////////////////////////////////////////////////////////////////////////
void CManager::AsioServiceWork()
{
    try
    {
        LOG_INFO << "Asio: io_service started";
        m_ioFlag = true;
        boost::asio::io_service::work work(m_ioService);
        m_ioService.run();
    }
    catch (const std::exception& e)
    {
        LOG_ERR << "Asio: io_service crashed (" << e.what() << ")\n\n" << boost::stacktrace::stacktrace();
    }

    // set flag io_service finished
    std::unique_lock<std::mutex> lock(m_ioMtx);
    m_ioFlag = false;
    m_ioCond.notify_one();
    LOG_INFO << "Asio: io_service finished";
}

void CManager::RunActors()
{
    CConfigurator* cnf = CConfigurator::instance();
    assert(cnf);

    // run actors
    bpt::ptree& tActors = cnf->getSubTree("proxy.actors");
    for (auto& actorCnf : tActors)
    {
        std::string actorName;
        try
        {
            if (actorCnf.first == "actor")
            {
                std::unique_lock<std::mutex> lock(m_actorsMtx);
                actorName = actorCnf.second.get<std::string>("name");
                std::shared_ptr<CActor> actor(CActor::MakeActor(actorCnf.second));
                if (actor)
                {
                    if (m_actors.find(actor->GetId()) == m_actors.end())
                    {
                        std::string id = actor->GetId();
                        auto it = m_actors.insert(std::make_pair(id, actor));
                        if (it.second)
                        {
                            it.first->second->Start();
                        }
                        else
                        {
                            LOG_ERR << "Actor with id = " << id << " was not inserted to map - DELETED";
                        }
                    }
                    else
                    {
                        LOG_WARN << "Actor with id = " << actor->GetId() << " was created before - SKIPPED";
                        actor.reset();
                    }
                }
                else
                {
                    LOG_WARN << "Actor with name " << actorName << " cannot be created";
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_WARN << "Error create actor with name " << actorName << ": " << e.what();
        }
    }
}

void CManager::SetChannels()
{
    CConfigurator* cnf = CConfigurator::instance();
    assert(cnf);

    // run actors
    bpt::ptree& tChannels = cnf->getSubTree("proxy.channels");
    for (auto& chCnf : tChannels)
    {
        std::string channelId;
        try
        {
            if (chCnf.first == "channel")
            {
                std::unique_lock<std::mutex> lock(m_actorsMtx);
                channelId = chCnf.second.get<std::string>("id");
                std::shared_ptr<CChannel> channel = std::make_shared<CChannel>(chCnf.second);
                if (channel)
                {
                    if (m_channels.find(channel->GetId()) == m_channels.end())
                    {
                        auto it = m_channels.insert(std::make_pair(channelId, channel));
                        if (it.second)
                        {
                            it.first->second->Connect();
                        }
                        else
                        {
                            LOG_ERR << "Channel with id = " << channelId << " was not inserted to map - DELETED";
                        }
                    }
                    else
                    {
                        LOG_WARN << "Channel with id = " << channelId << " was created before - SKIPPED";
                        channel.reset();
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_WARN << "Error create channel with id = " << channelId << ": " << e.what();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
