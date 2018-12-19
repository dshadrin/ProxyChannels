/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#pragma once

#include "sink.h"
#include "thread_pool.h"
#include <set>
#include <boost/noncopyable.hpp>

//////////////////////////////////////////////////////////////////////////
class CLogger : boost::noncopyable
{
public:
    CLogger(const boost::property_tree::ptree& pt);
    ~CLogger();
    void Start();
    void Stop();

    static CLogger* Get();
    void Push(PLog logPackage);

private:
    void WaitEmptyQueue();
    bool TimerClockHandler();
    void LogMultiplexer();
    void CreateSinks(const boost::property_tree::ptree& pt);
    static boost::property_tree::ptree ReadConfiguration();

private:
    thread_pool* m_tp;
    boost::mutex m_queueMutex;
    std::multiset<PLog, CCompareLogPackages> m_logRecords;
    std::vector<std::shared_ptr<CSink>> m_sinks;

    timer_job m_tmJob;
    bool m_continueTimer;
    DECLARE_MODULE_TAG;

    static std::unique_ptr<CLogger> s_Logger;
};
