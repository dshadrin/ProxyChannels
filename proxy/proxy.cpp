// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdinc.h"
#include "proxy.h"
#include "manager.h"

#define COMMUNICATOR_VERSION "0.2.18.1215"

COMMAPI bool proxy::init()
{
    CManager::init();
    return true;
}

COMMAPI bool proxy::reset()
{
    CManager::reset();
    return true;
}

COMMAPI const char* proxy::version()
{
    return "(C) Communicator module version " COMMUNICATOR_VERSION;
}

