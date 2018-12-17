#include "proxy.h"
#ifndef LOGGING_DIRECT
#include "logclient.h"
#else
#include "logbuilder.h"
#endif
#include "utils/strace_exception.h"
#include <iostream>

int main(int argc, char** argv)
{
    DEFINE_MODULE_TAG("MAPP");
    CLogMessageBuilder::SetLoggingChannel(0);
    std::cout << proxy::version() << std::endl;

    try
    {
        if (proxy::init())
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            LOG_INFO << "Monitor started.";
            std::string str;
            std::cin >> str;
            LOG_INFO << "Monitor finished.";
            boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
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

