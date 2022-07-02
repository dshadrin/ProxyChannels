/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "stdinc.h"
#include "configurator.h"
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>

std::mutex CConfigurator::sConfMtx;
std::unique_ptr<CConfigurator> CConfigurator::s_Configurator;

IMPLEMENT_MODULE_TAG(CConfigurator, "CONF");

CConfigurator::CConfigurator(const std::string& xmlPath)
    : m_pt(new bpt::ptree())
{
    fs::path cfgname(xmlPath.empty() ? fs::current_path() : fs::path(xmlPath));
    cfgname /= "proxy.xml";
    if (fs::exists(cfgname))
    {
        bpt::xml_parser::read_xml(cfgname.string(), *m_pt);
        // logger is not inited so output to console
        std::cout << "Configuration is loaded (" << cfgname << ")" << std::endl;
    }
    else
    {
        // logger is not inited so output to console
        std::cout << "Cannot load config file (" << cfgname << ")" << std::endl;
    }
}

CConfigurator * CConfigurator::instance()
{
    return !s_Configurator ? nullptr : s_Configurator.get();
}

void CConfigurator::init(const std::string& xmlPath)
{
    if (!s_Configurator)
    {
        std::unique_lock<std::mutex> lock(sConfMtx);
        if (!s_Configurator)
        {
            s_Configurator.reset(new CConfigurator(xmlPath));
        }
    }
}

bpt::ptree& CConfigurator::getSubTree(const std::string& path)
{
    return m_pt->get_child(path);
}

void CConfigurator::set(const std::string& path, const std::string& paramValue)
{
    try
    {
        // check if param path is correct
        // if path not exist the exception will be thrown
        std::string value = m_pt->get<std::string>(path);

        // trim value from spaces and quotas
        value = boost::algorithm::trim_copy_if(paramValue, boost::algorithm::is_any_of(" \"'"));

        // set new value
        m_pt->put<std::string>(path, value);
    }
    catch (const std::exception& e)
    {
        APP_EXCEPTION_ERROR(GMSG << "Configurator error: " << e.what());
    }
}
