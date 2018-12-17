#pragma once

//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    #ifdef STATIC_PROXY
        #define COMMAPI
    #else
        #ifdef PROXY_EXPORTS
            #define COMMAPI __declspec(dllexport)
        #else
            #define COMMAPI __declspec(dllimport)
        #endif // PROXY_EXPORTS
    #endif // STATIC_PROXY
#else
    #define COMMAPI
#endif // WIN32

#include <string>

//////////////////////////////////////////////////////////////////////////
namespace proxy
{
    COMMAPI bool init();
    COMMAPI bool reset();
    COMMAPI const char* version();
}
