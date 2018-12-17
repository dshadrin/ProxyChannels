/*
 * Copyright (C) 2014-2017 Rhonda Software.
 * All rights reserved.
 */

//////////////////////////////////////////////////////////////////////////

#include "stdinc.h"
#include "logger.h"
#include "configurator.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>

//////////////////////////////////////////////////////////////////////////
namespace boost
{
    BOOST_LOG_OPEN_NAMESPACE

    namespace attributes
    {
        template <typename date_time_type>
        void format_time_ms(std::ostringstream& formatter, const date_time_type& date_time)
        {
            auto time = date_time.time_of_day();
            using namespace std;
            formatter
                << setfill('0') << setw(2) << time.hours() << ':'
                << setfill('0') << setw(2) << time.minutes() << ':'
                << setfill('0') << setw(2) << time.seconds() << '.'
                << setfill('0') << setw(6) << time.fractional_seconds();
        }

        template <typename date_time_type>
        std::string format_time_ms(const date_time_type& date_time)
        {
            std::ostringstream formatter;
            format_time_ms(formatter, date_time);
            auto time = date_time.time_of_day();
            return formatter.str();
        }

        template <typename date_time_type>
        std::string format_date_time_ms(const date_time_type& date_time, const char date_time_sep = ' ')
        {
            using namespace std;
            ostringstream formatter;
            auto date = date_time.date();
            formatter
                << date.year() << '-'
                << setfill('0') << setw(2) << int(date.month()) << '-'
                << setfill('0') << setw(2) << date.day() << date_time_sep;
            format_time_ms(formatter, date_time);
            return formatter.str();
        }

        template <typename date_time_type, const char date_time_sep = ' '>
        struct date_time_ms_formatter
        {
            std::string operator () (const date_time_type& date_time) { return format_date_time_ms(date_time, date_time_sep); }
        };

        struct time_ms_formatter
        {
            template <typename date_time_type>
            std::string operator () (const date_time_type& date_time) { return format_time_ms(date_time); }
        };

        template <typename time_type>
        struct local_clock_source
        {
            time_type operator () () const
            {
                return date_time::microsec_clock<time_type>::local_time();
            }
        };

        template <typename time_type>
        struct universal_clock_source
        {
            time_type operator () () const
            {
                return date_time::microsec_clock<time_type>::universal_time();
            }
        };

        template <typename time_type, typename clock_source_type, typename formater_type>
        class custom_clock : public attribute
        {
        public:
            class impl : public attribute::impl
            {
            public:
                attribute_value get_value()
                {
                    auto str = formater_type()(clock_source_type()());
                    return make_attribute_value(str);
                }
            };

            custom_clock() : attribute(new impl()) {}

            explicit custom_clock(const cast_source& source) : attribute(source.as<impl>()) {}
        };


        typedef custom_clock<boost::posix_time::ptime, local_clock_source<boost::posix_time::ptime>, date_time_ms_formatter<boost::posix_time::ptime, ' '> >       local_date_time_ms_clock;
        typedef custom_clock<boost::posix_time::ptime, universal_clock_source<boost::posix_time::ptime>, date_time_ms_formatter<boost::posix_time::ptime, ' '> >   universal_date_time_ms_clock;

        typedef custom_clock<boost::posix_time::ptime, local_clock_source<boost::posix_time::ptime>, time_ms_formatter>     local_time_ms_clock;
        typedef custom_clock<boost::posix_time::ptime, universal_clock_source<boost::posix_time::ptime>, time_ms_formatter> universal_time_ms_clock;
    }

    BOOST_LOG_CLOSE_NAMESPACE // namespace log
}

BOOST_LOG_ATTRIBUTE_KEYWORD(dateTimeStamp, "DateTime", boost::posix_time::ptime);
BOOST_LOG_ATTRIBUTE_KEYWORD(timeStamp, "Time", boost::posix_time::ptime);

//////////////////////////////////////////////////////////////////////////

struct severity_tag;

std::unique_ptr<CLogger> CLogger::s_Logger;

//////////////////////////////////////////////////////////////////////////

// The operator is used when putting the severity level to log
boost::log::formatting_ostream& operator<<
(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip< ESeverity, severity_tag > const& manip
    )
{
    strm << SeverityToString((uint8_t)manip.get());
    return strm;
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(CLogger, "LOG ");

CLogger::CLogger() :
    m_slvl(ESeverity::eInfo),
    m_slvlConsole(ESeverity::eInfo),
    m_isStarted(false)
{
}

CLogger::~CLogger()
{
    if (m_isStarted)
    {
        Stop();
    }
}

void CLogger::Start()
{
    m_slvl = StringToSeverity(CConfigurator::instance()->get<std::string>("proxy.logger.severity", "DEBUG"));
    m_slvlConsole = StringToSeverity(CConfigurator::instance()->get<std::string>("proxy.logger.severity_console", "INFO"));
    m_strLogName = CConfigurator::instance()->get<std::string>("proxy.logger.log_name", "%Y%m%d_%H%M%S.log");
    if (m_strLogName.empty())
    {
        APP_EXCEPTION_ERROR("Logger error: No log file name defined.");
    }

//     boost::log::core::get()->add_global_attribute(dateTimeStamp.get_name(), boost::log::attributes::local_date_time_ms_clock());
    boost::log::core::get()->add_global_attribute(timeStamp.get_name(), boost::log::attributes::local_time_ms_clock());

    // Add some attributes too
    boost::log::core::get()->add_global_attribute("RecordID", boost::log::attributes::counter< unsigned int >());
    boost::log::core::get()->add_global_attribute("LineID", boost::log::attributes::counter< unsigned int >(1));

    // create a text file sink
    m_sink.reset(new fsink_t(
        boost::make_shared< boost::log::sinks::text_file_backend >(
            boost::log::keywords::file_name = m_strLogName
        ),
        boost::log::keywords::order =
            boost::log::make_attr_ordering("RecordID", std::less< unsigned int >())));

    m_sink->locked_backend()->auto_flush(true);

    m_sink->set_formatter
        (
        boost::log::expressions::format("[%1%][%2%][%3%] - %4%")
//             % boost::log::expressions::attr<std::string>(dateTimeStamp.get_name())
            % boost::log::expressions::attr<std::string>(timeStamp.get_name())
            % boost::log::expressions::attr<std::string>("Channel")
            % boost::log::expressions::attr<ESeverity, severity_tag>("Severity")
            % boost::log::expressions::smessage
        );

    m_sink->set_filter(boost::log::expressions::attr< ESeverity >("Severity") >= m_slvl);

    // create console stream sink
    m_sinkClog.reset(new sink_t(
        boost::make_shared<backend_t>(),
        boost::log::keywords::order =
            boost::log::make_attr_ordering("RecordID", std::less< unsigned int >())));

    m_sinkClog->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cerr, boost::null_deleter()));

    m_sinkClog->set_formatter
        (
        boost::log::expressions::format("[%1%][%2%] %3%")
            % boost::log::expressions::attr<std::string>("Channel")
            % boost::log::expressions::attr<ESeverity, severity_tag>("Severity")
            % boost::log::expressions::smessage
        );

    m_sinkClog->set_filter(boost::log::expressions::attr< ESeverity >("Severity") >= m_slvlConsole);

    // Add it to the core
    boost::log::core::get()->add_sink(m_sink);
    boost::log::core::get()->add_sink(m_sinkClog);
    m_isStarted = true;
    LOG_WARN << "Logger was started";
}

void CLogger::Stop()
{
    LOG_WARN << "Logger was finished";
    m_isStarted = false;

    boost::shared_ptr< boost::log::core > core = boost::log::core::get();

    // Remove the sink from the core, so that no records are passed to it
    core->remove_all_sinks();

    // Flush all buffered records
    m_sink->stop();
    m_sink->flush();
    m_sinkClog->stop();
    m_sinkClog->flush();
}

CLogger* CLogger::Get()
{
    static boost::mutex mtx;

    if (!s_Logger)
    {
        boost::mutex::scoped_lock lock(mtx);
        if (!s_Logger)
        {
            s_Logger.reset(new CLogger());
        }
    }
    return s_Logger.get();
}

//////////////////////////////////////////////////////////////////////////
typedef boost::log::sources::severity_channel_logger_mt< ESeverity, std::string > TRpcLoggerType;
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(rpcLg, TRpcLoggerType);
//////////////////////////////////////////////////////////////////////////
void DirectSendToLogger(std::shared_ptr<SLogPackage> logPackage)
{
    BOOST_LOG_CHANNEL_SEV(rpcLg::get(), logPackage->tag, (ESeverity)logPackage->severity) << logPackage->message;
}
size_t G_TagSize = 4;
size_t G_MaxMessageSize = 4096;
//////////////////////////////////////////////////////////////////////////

