/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#pragma once
#include "thread_pool.h"
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

    static CSink* MakeSink(const std::string& name);

    virtual void SetProperty(const std::string& name, const std::string& value);
    virtual void Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData) = 0;

protected:
    ESeverity m_severity;
};

//////////////////////////////////////////////////////////////////////////
class CConsoleSink : public CSink
{
public:
    CConsoleSink() = default;
    virtual ~CConsoleSink() = default;

    void Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData) override;
};

//////////////////////////////////////////////////////////////////////////
class CFileSink : public CSink
{
public:
    CFileSink() : m_prefix("Log") {}
    virtual ~CFileSink() = default;

    void SetProperty(const std::string& name, const std::string& value) override;
    void CreateFileName();

    void Write(std::shared_ptr<std::vector<std::shared_ptr<SLogPackage>>> logData) override;

private:
    void OpenStream();
    void CloseStream();

private:
    std::ofstream ofs;
    std::string m_fileNameTemplate;
    std::string m_prefix;
    std::string m_suffix;
    std::string m_fileName;
};
