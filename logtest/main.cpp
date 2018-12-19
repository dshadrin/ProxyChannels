#include "logclient.h"
#include "utils/strace_exception.h"
#include <iostream>

int main(int argc, char** argv)
{
    DEFINE_MODULE_TAG("TAPP");
    std::cout << "App started at   : " << TS::GetTimestampStr() << std::endl;
    CLogMessageBuilder::SetLoggingChannel(LOG_CLIENT_CHANNEL);

    try
    {
        CLogClient::Get()->LoggingMode(ELoggingMode::eLoggingToServer);
        boost::this_thread::sleep_for(boost::chrono::seconds(1));
        std::cout << "Test started at  : " << TS::GetTimestampStr() << std::endl;

        for (size_t i = 0; i < 100000; ++i)
        {
            LOG_TEST << "Test message: " << i;
        }

        std::cout << "Test finished at : " << TS::GetTimestampStr() << std::endl;
        CLogClient::Get()->Stop();
        std::cout << "App finished at  : " << TS::GetTimestampStr() << std::endl;
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

