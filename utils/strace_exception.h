/*
* Copyright (C) 2018
* All rights reserved.
*/

#pragma once

#include <boost/stacktrace.hpp>

//////////////////////////////////////////////////////////////////////////
namespace bst = boost::stacktrace;

//////////////////////////////////////////////////////////////////////////
namespace excpt
{

enum class EErrorSeverity : uint8_t
{
    ES_WARNING,     // continue execution (print warning and continue)
    ES_ERROR        // break execution (print error message and stop execution)     
};

//////////////////////////////////////////////////////////////////////////

class MessageBuilder
{
public:
    MessageBuilder() = default;
    ~MessageBuilder() = default;

    template<typename T>
    MessageBuilder& operator<<(const T& val)
    {
        oss << val;
        return *this;
    }

    std::string Str() const { return oss.str(); }

private:
    std::ostringstream oss;
};

//////////////////////////////////////////////////////////////////////////

class CSTraceException : public std::runtime_error
{
public:
    explicit CSTraceException(MessageBuilder& mb, const bst::stacktrace& stTrace, EErrorSeverity sev = EErrorSeverity::ES_ERROR, const char * file = nullptr, int line = 0)
        : std::runtime_error(mb.Str())
        , m_stackTrace(stTrace)
        , m_file(file)
        , m_line(line)
        , m_severity(sev) {}

    explicit CSTraceException( const std::string& what_arg, const bst::stacktrace& stTrace, EErrorSeverity sev = EErrorSeverity::ES_ERROR, const char * file = nullptr, int line = 0 )
        : std::runtime_error(what_arg)
        , m_stackTrace(stTrace)
        , m_file(file)
        , m_line(line)
        , m_severity(sev) {}

    ~CSTraceException() = default;

    static void PrintExeptionData();
    static void PrintExeptionData(const std::exception& e);
    static void PrintExeptionData(const CSTraceException& e);

    const char * File() const { return m_file; }
    int Line() const { return m_line; }
    EErrorSeverity Severity() const { return m_severity; }
    const bst::stacktrace& STrace() const { return m_stackTrace; }

private:
    bst::stacktrace m_stackTrace;
    const char * m_file;
    int m_line;
    EErrorSeverity m_severity;
};

//////////////////////////////////////////////////////////////////////////
}

#define APP_EXCEPTION(msg, sev)     throw excpt::CSTraceException(msg, bst::stacktrace(), sev, __FILE__, __LINE__)
#define APP_EXCEPTION_ERROR(msg)    APP_EXCEPTION(msg, excpt::EErrorSeverity::ES_ERROR)
#define APP_EXCEPTION_WARNING(msg)  APP_EXCEPTION(msg, excpt::EErrorSeverity::ES_WARNING)
#define GMSG excpt::MessageBuilder()
