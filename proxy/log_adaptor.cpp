// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "log_adaptor.h"

//////////////////////////////////////////////////////////////////////////
CLogAdaptor::CLogAdaptor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<std::string>("id"))
{
    boost::property_tree::ptree ptSinks = pt.get_child("sinks");
    for (auto& ptSink : ptSinks)
    {
        if (ptSink.first == "sink")
        {
            m_channels.emplace_back(ptSink.second);
            size_t idx = m_channels.size() - 1;
            m_toLog.connect([this, idx](const SLogPackage& pkg) -> void
            {
                m_channels[idx].TransferLogMessage(pkg);
            });
        }
    }
}

CLogAdaptor::~CLogAdaptor()
{
    m_toLog.disconnect_all_slots();
}
