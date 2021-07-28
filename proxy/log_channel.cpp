/*
 * Copyright (C) 2014-2017 Rhonda Software.
 * All rights reserved.
 */

#include "stdinc.h"
#include "log_channel.h"
#include "logger.h"
#include <boost/algorithm/string_regex.hpp>

//////////////////////////////////////////////////////////////////////////
DEFINE_MODULE_TAG("LCNL");

//////////////////////////////////////////////////////////////////////////
CLogChannel::CLogChannel(boost::property_tree::ptree& pt) :
    m_channel(pt.get<int8_t>("id")) // can be throw if config incorrect
{
    boost::optional<boost::property_tree::ptree&> pt_filters = pt.get_child_optional("filters");
    if (pt_filters)
    {
        try
        {
            for (auto& pt_filter : *pt_filters)
            {
                if (pt_filter.first == "filter")
                {
                    std::string name = pt_filter.second.get<std::string>("name", "");
                    if (!name.empty())
                    {
                        std::shared_ptr<CFilter> filter(CFilter::MakeFilter(name, pt_filter.second));
                        if (filter)
                        {
                            m_filters.push_back(filter);
                            LOG_INFO << "Created filter: " << name;
                        }
                        else
                        {
                            LOG_WARN << "Error creating filter: " << name;
                        }
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_WARN << "Creating filter error: " << e.what();
        }
    }
}

void CLogChannel::TransferLogMessage(const SLogPackage& msg)
{
    PLog pMsg(new SLogPackage(msg));
    if (!m_filters.empty())
    {
        for (auto& filter : m_filters)
        {
            filter->Execute(pMsg->message);
        }
    }
    if (pMsg->lchannel == LOG_UNKNOWN_CHANNEL)
    {
        pMsg->lchannel = Channel();
    }
    DirectSendToLogger(pMsg);
}

//////////////////////////////////////////////////////////////////////////
CFilter* CFilter::MakeFilter(const std::string& nameFilter, boost::property_tree::ptree& pt)
{
    if (nameFilter == RAGEX_ERASE)
        return new CRegexEraseFilter(pt);

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
CRegexEraseFilter::CRegexEraseFilter(boost::property_tree::ptree& pt) :
    m_regex(pt.get<std::string>("expression")) // can raise exception
{

}

void CRegexEraseFilter::Execute(std::string& str)
{
    try
    {
        boost::algorithm::erase_all_regex(str, m_regex);
    }
    catch (const std::exception&)
    {
        // do nothing
    }
}
