/*
 * Copyright (C) 2018-2022 dshadrin@gmail.com
 * All rights reserved.
 */

#include "logbuilder.h"
#include "strace_exception.h"

namespace excpt
{
//////////////////////////////////////////////////////////////////////////
DEFINE_MODULE_TAG("EXCT");

//////////////////////////////////////////////////////////////////////////
void CSTraceException::PrintExeptionData()
{
    LOG_ERR << "ERROR: Unknown!\n\n" << boost::stacktrace::stacktrace();
}

void CSTraceException::PrintExeptionData(const std::exception& e)
{
    LOG_ERR << "ERROR: " << e.what() << "\n\n" << boost::stacktrace::stacktrace();
}

void CSTraceException::PrintExeptionData(const CSTraceException& e)
{
    std::ostringstream record;
    record << "ERROR: " << e.what();
    if (e.Severity() == EErrorSeverity::ES_ERROR)
    {
        if (e.File() != nullptr)
        {
            record << " | " << e.File() << ":" << e.Line();
        }
        record << "\n\n";
        record << e.STrace();
    }
    LOG_ERR << record.str();
}

//////////////////////////////////////////////////////////////////////////
}
