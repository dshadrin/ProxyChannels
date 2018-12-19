/*
 * Copyright (C) 2018 Rhonda Software.
 * All rights reserved.
 */

//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////
#include "actor.h"
//////////////////////////////////////////////////////////////////////////
#define RAGEX_ERASE "REGEX-ERASE"
//////////////////////////////////////////////////////////////////////////
class CFilter
{
public:
    CFilter() = default;
    virtual ~CFilter() = default;
    virtual void Execute(std::string& str) = 0;
    static CFilter* MakeFilter(const std::string& nameFilter, boost::property_tree::ptree& pt);
};

//////////////////////////////////////////////////////////////////////////
class CRegexEraseFilter : public CFilter
{
public:
    CRegexEraseFilter(boost::property_tree::ptree& pt);
    virtual ~CRegexEraseFilter() = default;
    void Execute(std::string& str) override;

private:
    boost::regex m_regex;
};

//////////////////////////////////////////////////////////////////////////
class CLogChannel
{
public:
    explicit CLogChannel(boost::property_tree::ptree& pt);
    virtual ~CLogChannel() = default;

    int8_t Channel() const { return m_channel; }
    void TransferLogMessage(const SLogPackage& msg);

private:
    int8_t m_channel;
    std::vector<std::shared_ptr<CFilter>> m_filters;
};

//////////////////////////////////////////////////////////////////////////
