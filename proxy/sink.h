/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#pragma once
#include "thread_pool.h"
#include "utils/logbuilder.h"
#include <set>
#include <fstream>
#include <boost/noncopyable.hpp>

//////////////////////////////////////////////////////////////////////////
#define CONSOLE_SINK "CONSOLE"
#define FILE_SINK "FILE"

//////////////////////////////////////////////////////////////////////////
class CSink
{
public:
    CSink() = default;
    virtual ~CSink() = default;

    static CSink* MakeSink(const std::string& name, const boost::property_tree::ptree& pt);

    virtual void SetProperty(const std::string& name, const std::string& value);
    virtual void Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData) = 0;
    int8_t Channel() const { return m_channel; }

    static bool WaitJobFlag(uint32_t ms);
    uint32_t JobFlag() const { return 1 << m_channel; }
    void SetFlag();
    void ReleaseFlag();

protected:
    ESeverity m_severity;
    int8_t m_channel;
    DECLARE_MODULE_TAG;
    static boost::condition_variable m_jobCond;
    static boost::mutex m_jobMutex;
    static uint32_t m_jobFlag;
};

//////////////////////////////////////////////////////////////////////////
class CConsoleSink : public CSink
{
public:
    CConsoleSink(const boost::property_tree::ptree& pt);
    virtual ~CConsoleSink() = default;

    void Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData) override;
};

//////////////////////////////////////////////////////////////////////////
class CFileSink : public CSink
{
public:
    CFileSink(const boost::property_tree::ptree& pt);
    virtual ~CFileSink() = default;

    void SetProperty(const std::string& name, const std::string& value) override;
    void CreateFileName();

    void Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData) override;

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
