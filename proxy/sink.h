/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#pragma once
#include "thread_pool.h"
#include "utils/logbuilder.h"
#include <set>
#include <fstream>
#include <boost/describe.hpp>
#include <atomic>

//////////////////////////////////////////////////////////////////////////
BOOST_DEFINE_ENUM_CLASS( ESinkType, UNKNOWN_SINK, CONSOLE_SINK, FILE_SINK );

//////////////////////////////////////////////////////////////////////////
class CSink
{
public:
    CSink() = default;
    virtual ~CSink() = default;

    static CSink* MakeSink(const std::string& type, const boost::property_tree::ptree& pt);

    virtual void SetProperty(const std::string& name, const std::string& value);
    virtual void Write(std::shared_ptr<std::vector<PLog>> logData) = 0;
    int8_t Channel() const { return m_channel; }

    static bool WaitJobFinishAllSinks(uint32_t ms);
    static void IncrementJobCounter();
    static void DecrementJobCounter();

protected:
    ESeverity m_severity;
    int8_t m_channel;
    DECLARE_MODULE_TAG;
    static std::condition_variable m_jobCond;
    static std::mutex m_jobMutex;
    static std::atomic_uint_fast64_t m_jobCounter;
};

//////////////////////////////////////////////////////////////////////////
class CConsoleSink : public CSink
{
public:
    CConsoleSink(const boost::property_tree::ptree& pt);
    virtual ~CConsoleSink() = default;

    void Write(std::shared_ptr<std::vector<PLog>> logData) override;
};

//////////////////////////////////////////////////////////////////////////
class CFileSink : public CSink
{
public:
    CFileSink(const boost::property_tree::ptree& pt);
    virtual ~CFileSink() = default;

    void SetProperty(const std::string& name, const std::string& value) override;
    void CreateFileName();

    void Write(std::shared_ptr<std::vector<PLog>> logData) override;

private:
    void OpenStream();
    void CloseStream();

private:
    std::ofstream m_ofs;
    std::string m_fileNameTemplate;
    std::string m_prefix;
    std::string m_suffix;
    std::string m_fileName;
    bool m_isOpenByDemand;
};
