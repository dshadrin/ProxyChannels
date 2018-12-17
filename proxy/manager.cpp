#include "stdinc.h"
#include "manager.h"
#include "configurator.h"
#include "actor.h"
#include "channel.h"

//////////////////////////////////////////////////////////////////////////
#ifdef USE_SIMPLE_LOGGER
#include <iostream>
void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage)
{
    static boost::mutex mtx;
    boost::mutex::scoped_lock lock(mtx);
    std::cout << "[" << CLogMessageBuilder::GetTimestampStr(logPackage->timestamp) << "][" << logPackage->tag << "][" << SeverityToString(logPackage->severity) << "] - " << logPackage->message << std::endl;
}
size_t G_TagSize = 4;
size_t G_MaxMessageSize = 512;
#endif

//////////////////////////////////////////////////////////////////////////
boost::mutex CManager::sManagerMtx;
std::unique_ptr<CManager> CManager::sManager;
IMPLEMENT_MODULE_TAG(CManager, "MGR");

//////////////////////////////////////////////////////////////////////////
CManager::CManager() :
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
    return !sManager ? nullptr : sManager.get();
}

//////////////////////////////////////////////////////////////////////////
void CManager::init()
{
    if (!sManager)
    {
        boost::lock_guard<boost::mutex> lock(sManagerMtx);
        if (!sManager)
        {
            CConfigurator::init();
            sManager.reset(new CManager());
        }

#ifndef USE_SIMPLE_LOGGER
        CLogger::Get()->Start();
#endif

        LOG_INFO << "Manager started";
        sManager->m_tp.SetWorkUnit(std::bind(&CManager::AsioServiceWork, sManager.get()));

        sManager->RunActors();
        sManager->SetChannels();
    }
}

//////////////////////////////////////////////////////////////////////////
void CManager::reset()
{
    if (sManager)
    {
        // stop io_service
        if (!sManager->m_ioService.stopped())
        {
            sManager->m_ioService.stop();
            // wait finish io_service
            boost::unique_lock<boost::mutex> lock(sManager->m_ioMtx);
            while (sManager->m_ioFlag)
            {
                sManager->m_ioCond.wait(lock);
            }
        }

        // Erase actors
        {
            boost::mutex::scoped_lock lock(sManager->m_actorsMtx);
            for (auto& actor : sManager->m_actors)
            {
                actor.second->Stop();
            }
            sManager->m_actors.clear();
        }

        LOG_INFO << "Manager finished";
#ifndef USE_SIMPLE_LOGGER
        CLogger::Get()->Stop();
#endif
        sManager.reset();
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
                std::shared_ptr<CChannel> channel(new CChannel(chCnf.second));
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
