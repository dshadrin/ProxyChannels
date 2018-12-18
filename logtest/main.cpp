#include "proxy.h"
#include "logclient.h"
#include "utils/strace_exception.h"
#include <iostream>

int main(int argc, char** argv)
{
    DEFINE_MODULE_TAG("TAPP");
    CLogMessageBuilder::SetLoggingChannel(3);

    try
    {
        CLogClient::Get()->LoggingMode(ELoggingMode::eLoggingToServer);
        boost::this_thread::sleep_for(boost::chrono::seconds(1));
        LOG_INFO << "Test started.";

        for (size_t i = 0; i < 1000000; ++i)
        {
            LOG_TEST << "Test message: " << i;
        }

        LOG_INFO << "Test finished.";
        boost::this_thread::sleep_for(boost::chrono::seconds(1));
        CLogClient::Get()->Stop();
        return 0;
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
    return -1;
}

