// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*
 * Copyright (C) 2014-2017 Rhonda Software.
 * All rights reserved.
 */

#include "stdinc.h"
#include "manager_adaptor.h"
#include "manager.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CManagerAdaptor, "MCMD");

//////////////////////////////////////////////////////////////////////////
CManagerAdaptor::CManagerAdaptor(boost::property_tree::ptree& pt) :
    CActor(pt.get<std::string>("name"), pt.get<size_t>("id"))
{
    try
    {
        m_sigOutputMessage.connect(std::bind(&CManagerAdaptor::DoCommand, this, std::placeholders::_1));
    }
    catch ( std::exception &e )
    {
        APP_EXCEPTION_ERROR(GMSG << "Manager adapter creating error: " << e.what());
    }
}

//////////////////////////////////////////////////////////////////////////
CManagerAdaptor::~CManagerAdaptor()
{
    LOG_INFO << "Destroyed manager adapter";
}

void CManagerAdaptor::Start()
{

}

void CManagerAdaptor::Stop()
{
    m_sigOutputMessage.disconnect_all_slots();
    m_sigInputMessage.disconnect_all_slots();
}

void CManagerAdaptor::DoCommand(PMessage msg)
{
    for (char ch : *msg)
    {
        m_inBuffer.push_back(ch);
    }

    size_t pos = m_inBuffer.find('\n');
    if (pos != std::string::npos)
    {
        std::string sCmd = m_inBuffer.substr(0, pos + 1);
        m_inBuffer.erase(0, pos + 1);
        boost::trim(sCmd);
        boost::to_upper(sCmd);

        if (sCmd == "QUIT" || sCmd == "EXIT")
        {
            LOG_WARN << "Get command " << sCmd;
            CManager::instance()->Stop();
        }
    }
}
