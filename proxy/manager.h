#pragma once

#include "logger.h"
#include "signals.h"
#include "thread_pool.h"
#include <boost/asio/io_service.hpp>

//////////////////////////////////////////////////////////////////////////
class CActor;
class CChannel;

//////////////////////////////////////////////////////////////////////////
class CManager
{
public:
    CManager();
    ~CManager();
    static CManager * instance();
    static void init();
    static void reset();
    void Stop();

    boost::asio::io_service& IoService() { return m_ioService; }
    thread_pool& ThreadPool() { return m_tp; }
    std::weak_ptr<CActor> FindActor( std::string id);
    bool IsStopped() const { return m_stopFlag; }

private:
    CSignalHandler m_sigHandler;
    thread_pool m_tp;
    boost::asio::io_service m_ioService;
    bool m_ioFlag;
    bool m_stopFlag;
    std::mutex m_ioMtx;
    std::condition_variable m_ioCond;
    std::mutex m_stopMtx;
    std::condition_variable m_stopCond;

    std::mutex m_actorsMtx;
    std::map<std::string, std::shared_ptr<CActor>> m_actors;
    std::map<std::string, std::shared_ptr<CChannel>> m_channels;

private:
    void AsioServiceWork();
    void RunActors();
    void SetChannels();

private:
    static std::mutex sManagerMtx;
    static std::unique_ptr<CManager> s_Manager;
    DECLARE_MODULE_TAG;
};
