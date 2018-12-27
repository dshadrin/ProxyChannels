#pragma once

#include "thread_pool.h"
#include "logger.h"
#include "signals.h"
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

    boost::asio::io_service& IoService() { return m_ioService; }
    thread_pool& ThreadPool() { return m_tp; }
    std::weak_ptr<CActor> FindActor(size_t id);

private:
    CSignalHandler m_sigHandler;
    thread_pool m_tp;
    boost::asio::io_service m_ioService;
    boost::mutex m_ioMtx;
    boost::condition_variable m_ioCond;
    bool m_ioFlag;

    boost::mutex m_actorsMtx;
    std::map<size_t, std::shared_ptr<CActor>> m_actors;
    std::map<size_t, std::shared_ptr<CChannel>> m_channels;

private:
    void AsioServiceWork();
    void RunActors();
    void SetChannels();

private:
    static boost::mutex sManagerMtx;
    static std::unique_ptr<CManager> s_Manager;
    DECLARE_MODULE_TAG;
};
