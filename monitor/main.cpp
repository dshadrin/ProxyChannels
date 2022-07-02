/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "proxy.h"
#ifndef LOGGING_DIRECT
#include "logclient.h"
#else
#include "logbuilder.h"
#endif
#include "utils/strace_exception.h"
#include <iostream>
#ifdef WIN32
#include <clocale>
#endif

int main(int argc, char** argv)
{
#ifdef WIN32
    setlocale( LC_ALL, "Russian" );
#endif
    DEFINE_MODULE_TAG("MAPP");
    CLogMessageBuilder::SetLoggingChannel(0);
    std::cout << proxy::version() << std::endl;

    try
    {
        if (proxy::init())
        {
#ifndef LOGGING_DIRECT
            CLogClient::Get()->Stop();
#endif
            proxy::reset();
            return 0;
        }
    }
    catch (const excpt::CSTraceException& e)
    {
        excpt::CSTraceException::PrintExeptionData(e);
    }
    catch (std::exception& e)
    {
        excpt::CSTraceException::PrintExeptionData(e);
    }
    catch (...)
    {
        excpt::CSTraceException::PrintExeptionData();
    }
    return 1;
}

