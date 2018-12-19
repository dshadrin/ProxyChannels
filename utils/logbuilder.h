#pragma once

#include "lockqueue.h"
#include "timestamp.h"
#include <cstdint>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>

//////////////////////////////////////////////////////////////////////////
#define LOG_UNKNOWN_CHANNEL        -1
#define LOG_INTERNAL_CHANNEL        0
#define LOG_UART_FILTERED_CHANNEL   1
#define LOG_UART_RAW_CHANNEL        2
#define LOG_CLIENT_CHANNEL          3

//////////////////////////////////////////////////////////////////////////
enum class ESeverity : uint8_t
{
    eTrace,
    eDebug,
    eInfo,
    eTest,
    eWarn,
    eError,
    eCrit
};

enum class ELoggingMode : uint8_t
{
    eLoggingToServer,
    eNoLogging
};

enum class ELogCommand : uint8_t
{
    eMessage,
    eChangeFile,
    eStop
};

//////////////////////////////////////////////////////////////////////////
struct SLogPackage
{
    std::string message;    // prefix
    std::string tag;        // suffix
    timespec timestamp;
    ELogCommand command;
    uint8_t severity;
    int8_t lchannel;
};

inline bool operator<(const timespec& lhd, const timespec& rhd)
{
    bool status = lhd.tv_sec < rhd.tv_sec;
    if (!status && lhd.tv_sec == rhd.tv_sec)
    {
        status = lhd.tv_nsec < rhd.tv_nsec;
    }
    return status;
}

//////////////////////////////////////////////////////////////////////////
struct CCompareTimestamps
{
    bool operator()(const timespec& lhd, const timespec& rhd) const
    {
        return lhd < rhd;
    }
};

//////////////////////////////////////////////////////////////////////////
struct CCompareLogPackages
{
    bool operator()(const std::shared_ptr<SLogPackage>& lhd, const std::shared_ptr<SLogPackage>& rhd) const
    {
        return lhd->timestamp < rhd->timestamp;
    }
};

#ifdef LOGGING_DIRECT
extern void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage);
#else
extern void NetSendToLogger(std::shared_ptr<SLogPackage> logPackage);
#endif

//////////////////////////////////////////////////////////////////////////
extern size_t G_TagSize;
extern size_t G_MaxMessageSize;

//////////////////////////////////////////////////////////////////////////
class CLogMessageBuilder : boost::noncopyable
{
public:
    CLogMessageBuilder(ESeverity sev, const std::string& tag) :
#ifdef LOGGING_DIRECT
        m_Sender(&DirectSendToLogger),
#else
        m_Sender(&NetSendToLogger),
#endif
        m_package(new SLogPackage)
    {
        Init(sev, tag);
    }

    ~CLogMessageBuilder();
    static void SetLoggingChannel(int8_t ch);
    static int8_t GetLoggingChannel();

    template<typename _Type>
    CLogMessageBuilder& operator<<(const _Type& data)
    {
        oss << data;
        return *this;
    }

private:
    void Init(ESeverity sev, const std::string& tag);
    std::shared_ptr<SLogPackage> m_package;
    std::ostringstream oss;
    typedef void (*SendToLogger_t)(std::shared_ptr<SLogPackage>);
    SendToLogger_t m_Sender;
    static int8_t s_logChannel;
};

//////////////////////////////////////////////////////////////////////////
ESeverity StringToSeverity(const std::string& sSev);
std::string SeverityToString(uint8_t sev);

//////////////////////////////////////////////////////////////////////////
/// Helper macros for logging
//////////////////////////////////////////////////////////////////////////
#define MODULE_TAG_NAME logger_module_tag
#define DECLARE_MODULE_TAG static const std::string MODULE_TAG_NAME
#define DEFINE_MODULE_TAG(value) static const std::string MODULE_TAG_NAME = value
#define IMPLEMENT_MODULE_TAG(ClassName, value) const std::string ClassName::MODULE_TAG_NAME = value
//////////////////////////////////////////////////////////////////////////
#define LOG_DEBUG CLogMessageBuilder( ESeverity::eDebug, MODULE_TAG_NAME )
#define LOG_INFO  CLogMessageBuilder( ESeverity::eInfo, MODULE_TAG_NAME )
#define LOG_TEST  CLogMessageBuilder( ESeverity::eTest, MODULE_TAG_NAME )
#define LOG_WARN  CLogMessageBuilder( ESeverity::eWarn, MODULE_TAG_NAME )
#define LOG_ERR   CLogMessageBuilder( ESeverity::eError, MODULE_TAG_NAME )
#define LOG_CRIT  CLogMessageBuilder( ESeverity::eCrit, MODULE_TAG_NAME )
#define LOG_TRACE CLogMessageBuilder( ESeverity::eTrace, MODULE_TAG_NAME )
