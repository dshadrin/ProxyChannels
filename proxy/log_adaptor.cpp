#include "stdinc.h"
#include "log_adaptor.h"

//////////////////////////////////////////////////////////////////////////
CLogAdaptor::CLogAdaptor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id"))
{
    boost::property_tree::ptree ptChannels = pt.get_child("channels");
    for (auto& ptChannel : ptChannels)
    {
        if (ptChannel.first == "channel")
        {
            m_channels.emplace_back(ptChannel.second);
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
