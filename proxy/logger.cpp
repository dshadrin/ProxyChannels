/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#include "stdinc.h"
#include "logger.h"
#include "manager.h"
#include <iostream>
#include <iterator>
#include <boost/property_tree/xml_parser.hpp>

////////////////////////////////////////////////////////////////////////// 
void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage)
{
    CLogger::Get()->Push(logPackage);
}

//////////////////////////////////////////////////////////////////////////
#define LOG_OUTPUT_DELAY_MS 2500
#define LOG_OUTPUT_TIMER_PERIOD 1500
IMPLEMENT_MODULE_TAG(CLogger, "LOG");
//////////////////////////////////////////////////////////////////////////
std::unique_ptr<CLogger> CLogger::s_Logger;
//////////////////////////////////////////////////////////////////////////
size_t G_TagSize = 4;
size_t G_MaxMessageSize = 4096;

//////////////////////////////////////////////////////////////////////////
CLogger::CLogger(const boost::property_tree::ptree& pt) :
    m_tp(nullptr),
    m_continueTimer(false)
{
    CreateSinks(pt);
}


CLogger::~CLogger()
{

}

boost::property_tree::ptree CLogger::ReadConfiguration()
{
    boost::property_tree::ptree ptLogger;

    try
    {
        boost::filesystem::path cfgname(boost::filesystem::current_path());
        cfgname /= "proxy.xml";

        if (boost::filesystem::exists(cfgname))
        {
            boost::property_tree::ptree pt;
            boost::property_tree::xml_parser::read_xml(cfgname.string(), pt);
            ptLogger = pt.get_child("proxy.logger");
        }
        else
        {
            std::cout << "Cannot load config file (" << cfgname << ")" << std::endl;
        }
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("Wrong configuration file.");
    }

    return std::move(ptLogger);
}

void CLogger::Start()
{
    assert(CManager::instance() != nullptr);
    m_tp = &CManager::instance()->ThreadPool();
    assert(m_tp != nullptr);
    m_continueTimer = true;
    m_tmJob.tmPoint = boost::chrono::high_resolution_clock::now() + boost::chrono::milliseconds(1500);
    m_tmJob.tmPeriod = boost::chrono::milliseconds(LOG_OUTPUT_TIMER_PERIOD);
    m_tmJob.callback = std::bind(&CLogger::TimerClockHandler, this);
    m_tp->SetTimer(m_tmJob);
    LOG_WARN << "Logger was started";
}

void CLogger::Stop()
{
    LOG_WARN << "Logger was finished";
    WaitEmptyQueue();
    m_continueTimer = false;
}

void CLogger::WaitEmptyQueue()
{
    size_t count = 1;
    while (count)
    {
        {
            boost::mutex::scoped_lock lock(m_queueMutex);
            count = m_logRecords.size();
        }
        if (count)
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        }
    }
}

CLogger* CLogger::Get()
{
    static boost::mutex mtx;

    if (!s_Logger)
    {
        boost::mutex::scoped_lock lock(mtx);
        if (!s_Logger)
        {
            boost::property_tree::ptree pt = CLogger::ReadConfiguration();
            s_Logger.reset(new CLogger(pt));
        }
    }
    return s_Logger.get();
}

void CLogger::Push(std::shared_ptr<SLogPackage> logPackage)
{
    if (logPackage->lchannel != LOG_UNKNOWN_CHANNEL)
    {
        boost::mutex::scoped_lock lock(m_queueMutex);
        m_logRecords.insert(logPackage);
    }
    else
    {
        LOG_WARN << "Skip log message with unknown channel";
    }
}

bool CLogger::TimerClockHandler()
{
    m_tp->SetWorkUnit(std::bind(&CLogger::LogMultiplexer, this));
    return m_continueTimer;
}

void CLogger::LogMultiplexer()
{
    if (CSink::WaitJobFlag(1000) && m_logRecords.size() > 0)
    {
        std::shared_ptr<SLogPackage> pack(new SLogPackage());
        pack->timestamp = TS::GetTimestamp();
        TS::TimestampAdjust(pack->timestamp, -LOG_OUTPUT_DELAY_MS);

        boost::mutex::scoped_lock lock(m_queueMutex);
        auto it = m_logRecords.upper_bound(pack);

        std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> outRecords(new std::vector<std::shared_ptr<SLogPackage>>());
        std::copy(std::begin(m_logRecords), it, std::back_inserter(*outRecords));
        m_logRecords.erase(std::begin(m_logRecords), it);

        if (outRecords->size() > 0)
        {
            for (auto& it : m_sinks)
            {
                if (std::find_if(std::begin(*outRecords), std::end(*outRecords), [&it](const std::shared_ptr<SLogPackage>& item) -> bool
                {
                    return item->lchannel == it->Channel();
                }) != std::end(*outRecords))
                {
                    it->SetFlag();
                    m_tp->SetWorkUnit([it, outRecords]() -> void
                    {
                        it->Write(outRecords);
                    });
                }
            }
        }
    }
}

void CLogger::CreateSinks(const boost::property_tree::ptree& pt)
{
    const boost::property_tree::ptree& sinks = pt.get_child("sinks");
    for (auto& sink : sinks)
    {
        if (sink.first == "sink")
        {
            std::string sinkName = sink.second.get<std::string>("name", "");
            if (!sinkName.empty())
            {
                CSink* ptr = CSink::MakeSink(sinkName, sink.second);
                if (ptr)
                {
                    m_sinks.emplace_back(ptr);
                    std::cout << "Created " << sinkName << " sink." << std::endl;
                }
            }
        }
    }
}
