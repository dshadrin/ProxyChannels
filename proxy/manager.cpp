// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "manager.h"
#include "configurator.h"
#include "actor.h"
#include "channel.h"

//////////////////////////////////////////////////////////////////////////
boost::mutex CManager::sManagerMtx;
std::unique_ptr<CManager> CManager::s_Manager;
IMPLEMENT_MODULE_TAG(CManager, "MGR");

//////////////////////////////////////////////////////////////////////////
CManager::CManager() :
    m_sigHandler({SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM}),
    m_tp(),
    m_ioFlag(false)
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
        boost::lock_guard<boost::mutex> lock(sManagerMtx);
        if (!s_Manager)
        {
            CConfigurator::init();
            s_Manager.reset(new CManager());
        }

        CLogger::Get()->Start();

        LOG_INFO << "Manager started";
        s_Manager->m_tp.SetWorkUnit(std::bind(&CManager::AsioServiceWork, s_Manager.get()));
        s_Manager->m_tp.SetWorkUnit(std::bind(&CManager::AsioServiceWork, s_Manager.get()));

        s_Manager->RunActors();
        s_Manager->SetChannels();
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
            boost::unique_lock<boost::mutex> lock(s_Manager->m_ioMtx);
            while (s_Manager->m_ioFlag)
            {
                s_Manager->m_ioCond.wait(lock);
            }
        }

        // Erase actors
        {
            boost::mutex::scoped_lock lock(s_Manager->m_actorsMtx);
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

std::weak_ptr<CActor> CManager::FindActor(size_t id)
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
    boost::lock_guard<boost::mutex> lock(m_ioMtx);
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
                boost::mutex::scoped_lock lock(m_actorsMtx);
                actorName = actorCnf.second.get<std::string>("name");
                std::shared_ptr<CActor> actor(CActor::MakeActor(actorCnf.second));
                if (actor)
                {
                    if (m_actors.find(actor->GetId()) == m_actors.end())
                    {
                        size_t id = actor->GetId();
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
        size_t channelId = 0;
        try
        {
            if (chCnf.first == "channel")
            {
                boost::mutex::scoped_lock lock(m_actorsMtx);
                channelId = chCnf.second.get<size_t>("id");
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
