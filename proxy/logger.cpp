/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "stdinc.h"
#include "logger.h"
#include "manager.h"
#include <iostream>
#include <iterator>
#include <chrono>
#include <boost/property_tree/xml_parser.hpp>

////////////////////////////////////////////////////////////////////////// 
void DirectSendToLogger(PLog logPackage)
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
        std::filesystem::path cfgname(std::filesystem::current_path());
        cfgname /= "proxy.xml";

        if (std::filesystem::exists(cfgname))
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
    m_tmJob.tmPoint = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1500);
    m_tmJob.tmPeriod = std::chrono::milliseconds(LOG_OUTPUT_TIMER_PERIOD);
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
            std::unique_lock<std::mutex> lock(m_queueMutex);
            count = m_logRecords.size();
        }
        if (count)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

CLogger* CLogger::Get()
{
    static std::mutex mtx;

    if (!s_Logger)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!s_Logger)
        {
            boost::property_tree::ptree pt = CLogger::ReadConfiguration();
            s_Logger.reset(new CLogger(pt));
        }
    }
    return s_Logger.get();
}

void CLogger::Push(PLog logPackage)
{
    if (logPackage->lchannel != LOG_UNKNOWN_CHANNEL)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_logRecords.insert(logPackage);
    }
    else
    {
        LOG_WARN << "Skip log message with unknown channel";
    }
}

bool CLogger::TimerClockHandler()
{
    m_tp->SetWorkUnit(std::bind(&CLogger::LogMultiplexer, this), false);
    return m_continueTimer;
}

void CLogger::LogMultiplexer()
{
    if (CSink::WaitJobFinishAllSinks(1000) && m_logRecords.size() > 0)
    {
        PLog pack(new SLogPackage());
        pack->timestamp = TS::GetTimestamp();
        TS::TimestampAdjust(pack->timestamp, -LOG_OUTPUT_DELAY_MS);

        std::unique_lock<std::mutex> lock(m_queueMutex);
        auto it = m_logRecords.upper_bound(pack);

        std::shared_ptr<std::vector<PLog>> outRecords(new std::vector<PLog>());
        std::copy(std::begin(m_logRecords), it, std::back_inserter(*outRecords));
        m_logRecords.erase(std::begin(m_logRecords), it);

        if (outRecords->size() > 0)
        {
            for (auto& it : m_sinks)
            {
                if (std::find_if(std::begin(*outRecords), std::end(*outRecords), [&it](const PLog& item) -> bool
                {
                    return item->lchannel == it->Channel();
                }) != std::end(*outRecords))
                {
                    CSink::IncrementJobCounter();
                    m_tp->SetWorkUnit([it, outRecords]() -> void
                    {
                        it->Write(outRecords);
                    }, false);
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
            std::string sinkType = sink.second.get<std::string>("type", "");
            if (!sinkType.empty())
            {
                CSink* ptr = CSink::MakeSink( sinkType, sink.second);
                if (ptr)
                {
                    m_sinks.emplace_back(ptr);
                    std::cout << "Created " << sinkType << std::endl;
                }
            }
        }
    }
}
