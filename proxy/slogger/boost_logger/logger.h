/*
 * Copyright (C) 2018
 * All rights reserved.
 */

//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////
#include "utils/logbuilder.h"
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/noncopyable.hpp>

//////////////////////////////////////////////////////////////////////////
// check and watch that main thread is working
//  if thread was waked up by timeout that main thread was friezed before
//////////////////////////////////////////////////////////////////////////
class CLogger : boost::noncopyable
{
public:
    CLogger();
    ~CLogger();

    static CLogger* Get();
    void Start();
    void Stop();

    bool IsStarted() { return m_isStarted; }

private:
    typedef boost::log::sinks::text_ostream_backend backend_t;
    typedef boost::log::sinks::asynchronous_sink <
        backend_t,
        boost::log::sinks::unbounded_ordering_queue <
            boost::log::attribute_value_ordering < unsigned int, std::less< unsigned int > >
        >
    > sink_t;

    typedef boost::log::sinks::text_file_backend file_backend_t;
    typedef boost::log::sinks::asynchronous_sink <
        file_backend_t,
        boost::log::sinks::unbounded_ordering_queue <
            boost::log::attribute_value_ordering < unsigned int, std::less< unsigned int > >
        >
    > fsink_t;

private:
    std::string m_strLogName;
    boost::shared_ptr< fsink_t > m_sink;
    boost::shared_ptr< sink_t > m_sinkClog;
    ESeverity m_slvl;
    ESeverity m_slvlConsole;
    bool m_isStarted;
    DECLARE_MODULE_TAG;

    static std::unique_ptr<CLogger> s_Logger;
};

//////////////////////////////////////////////////////////////////////////
