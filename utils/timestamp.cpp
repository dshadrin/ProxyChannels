// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "timestamp.h"
#include <boost/thread.hpp>

//////////////////////////////////////////////////////////////////////////
const long ONE_SECOND_IN_MICROSECONDS = 1000000l;

//////////////////////////////////////////////////////////////////////////
void TS::TimestampAdjust(timespec& ts, long deltaFromNowMcs)
{
    if (deltaFromNowMcs != 0)
    {
        int64_t mcs = ((ts.tv_sec % ONE_SECOND_IN_MICROSECONDS) * ONE_SECOND_IN_MICROSECONDS) + ts.tv_nsec + deltaFromNowMcs;
        int64_t sec =(ts.tv_sec / ONE_SECOND_IN_MICROSECONDS) * ONE_SECOND_IN_MICROSECONDS;

        ts.tv_sec = sec + mcs / ONE_SECOND_IN_MICROSECONDS;

        long part = mcs % ONE_SECOND_IN_MICROSECONDS;
        while (part < 0)
        {
            part += ONE_SECOND_IN_MICROSECONDS;
            --ts.tv_sec;
        }

        ts.tv_nsec = part;
    }
}

//////////////////////////////////////////////////////////////////////////
void TS::ConvertTimestamp(timespec tv, tm* tmStruct, long* us, long deltaFromNowMcs /*= 0*/)
{
    TimestampAdjust(tv, deltaFromNowMcs);

#ifdef WIN32
    localtime_s(tmStruct, &tv.tv_sec);
#else
    static boost::mutex mtx;
    {
        boost::mutex::scoped_lock lock(mtx);
        memcpy(tmStruct, localtime(&tv.tv_sec), sizeof(struct tm));
    }
#endif
    *us = tv.tv_nsec;
}

//////////////////////////////////////////////////////////////////////////
struct timespec TS::GetTimestamp()
{
    timespec tv;
#ifdef MINGW
    clock_gettime(CLOCK_MONOTONIC, &tv);
#else
    timespec_get(&tv, TIME_UTC);
#endif
    tv.tv_nsec /= 1000;
    return tv;
}

//////////////////////////////////////////////////////////////////////////
std::string TS::GetTimestampStr()
{
    return GetTimestampStr(GetTimestamp());
}

//////////////////////////////////////////////////////////////////////////
std::string TS::GetTimestampStr(struct tm& tmStruct, long us)
{
    std::string buffer(16, '\0');
    snprintf(&buffer[0], 16, "%02d:%02d:%02d.%06ld", tmStruct.tm_hour, tmStruct.tm_min, tmStruct.tm_sec, us);
    buffer.pop_back();
    return std::move(buffer);
}

//////////////////////////////////////////////////////////////////////////
std::string TS::GetTimestampStr(const timespec& tv)
{
    tm tmStruct;
    long us;
    ConvertTimestamp(tv, &tmStruct, &us);
    return GetTimestampStr(tmStruct, us);
}

//////////////////////////////////////////////////////////////////////////
