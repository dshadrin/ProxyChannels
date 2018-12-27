// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#include "stdinc.h"
#include "signals.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////////
namespace
{
#define DEFSIGSTR(value) {value, #value}
#define DEFSTRSIG(value) {#value, value}


    const std::map<int, std::string> mapSignalToString =
    {
        DEFSIGSTR(SIGINT),
        DEFSIGSTR(SIGILL),
        DEFSIGSTR(SIGABRT),
        DEFSIGSTR(SIGFPE),
        DEFSIGSTR(SIGSEGV),
#ifndef WIN32
        DEFSIGSTR(SIGHUP),
        DEFSIGSTR(SIGQUIT),
        DEFSIGSTR(SIGBUS),
        DEFSIGSTR(SIGKILL),
        DEFSIGSTR(SIGUSR1),
        DEFSIGSTR(SIGUSR2),
        DEFSIGSTR(SIGPIPE),
        DEFSIGSTR(SIGALRM),
        DEFSIGSTR(SIGSTKFLT),
        DEFSIGSTR(SIGCHLD),
        DEFSIGSTR(SIGCONT),
        DEFSIGSTR(SIGSTOP),
        DEFSIGSTR(SIGTSTP),
        DEFSIGSTR(SIGTTIN),
        DEFSIGSTR(SIGTTOU),
        DEFSIGSTR(SIGURG),
        DEFSIGSTR(SIGXCPU),
        DEFSIGSTR(SIGXFSZ),
        DEFSIGSTR(SIGVTALRM),
        DEFSIGSTR(SIGPROF),
        DEFSIGSTR(SIGWINCH),
        DEFSIGSTR(SIGPWR),
        DEFSIGSTR(SIGSYS),
#endif
        DEFSIGSTR(SIGTERM)
    };

    const std::map<std::string, int> mapStringToSignal =
    {
        DEFSTRSIG(SIGINT),
        DEFSTRSIG(SIGILL),
        DEFSTRSIG(SIGABRT),         // GIGTTOU
        DEFSTRSIG(SIGFPE),
        DEFSTRSIG(SIGSEGV),
#ifdef WIN32
        DEFSTRSIG(SIGBREAK),        // SIGTTIN
        DEFSTRSIG(SIGABRT_COMPAT),  // SIGABRT
#else
        DEFSTRSIG(SIGHUP),
        DEFSTRSIG(SIGQUIT),
        DEFSTRSIG(SIGBUS),
        DEFSTRSIG(SIGKILL),
        DEFSTRSIG(SIGUSR1),
        DEFSTRSIG(SIGUSR2),
        DEFSTRSIG(SIGPIPE),
        DEFSTRSIG(SIGALRM),
        DEFSTRSIG(SIGSTKFLT),
        DEFSTRSIG(SIGCHLD),
        DEFSTRSIG(SIGCONT),
        DEFSTRSIG(SIGSTOP),
        DEFSTRSIG(SIGTSTP),
        DEFSTRSIG(SIGTTIN),
        DEFSTRSIG(SIGTTOU),
        DEFSTRSIG(SIGURG),
        DEFSTRSIG(SIGXCPU),
        DEFSTRSIG(SIGXFSZ),
        DEFSTRSIG(SIGVTALRM),
        DEFSTRSIG(SIGPROF),
        DEFSTRSIG(SIGWINCH),
        DEFSTRSIG(SIGPWR),
        DEFSTRSIG(SIGSYS),
#endif
        DEFSTRSIG(SIGTERM)
    };
}

std::string SignalToString(int sigId)
{
    auto it = mapSignalToString.find(sigId);
    return it != mapSignalToString.cend() ? it->second : "UNKNOWN";
}

int StringToSignal(const std::string& sigName)
{
    auto it = mapStringToSignal.find(sigName);
    return it != mapStringToSignal.cend() ? it->second : NSIG;
}

//////////////////////////////////////////////////////////////////////////
void CSignalHandler::Handler(int sig)
{
    auto strace = bst::stacktrace();
    std::cerr << "Got signal: " << SignalToString(sig) << std::endl << std::endl;

    switch (sig)
    {
        case SIGSEGV:
        case SIGILL:
        case SIGFPE:
        case SIGABRT:
            std::cerr << strace << std::endl;
            break;
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
//////////////////////////////////////////////////////////////////////////
// SIGINT          2   // interrupt
// SIGILL          4   // illegal instruction - invalid function image
// SIGFPE          8   // floating point exception
// SIGSEGV         11  // segment violation
// SIGTERM         15  // Software termination signal from kill
// SIGBREAK        21  // Ctrl-Break sequence
// SIGABRT         22  // abnormal termination triggered by abort call
// SIGABRT_COMPAT  6   // SIGABRT compatible with other platforms, same as SIGABRT
//////////////////////////////////////////////////////////////////////////
        case SIGABRT_COMPAT:
            std::cerr << strace << std::endl;
            break;
        case SIGINT: // ignore
        case SIGBREAK:
            break;
//////////////////////////////////////////////////////////////////////////
#else // Linux
//////////////////////////////////////////////////////////////////////////
        case SIGINT: // ignore
            break;
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////
        case SIGTERM:
        default:
            break;
    }
    exit(128 - sig);
}

void CSignalHandler::SetHandler()
{
    for (int sig : m_signals)
    {
        if (signal(sig, &CSignalHandler::Handler) == SIG_ERR)
        {
            throw CSignalException(sig);
        }
    }
}

void CSignalHandler::UnsetHandler()
{
    for (int sig : m_signals)
    {
        signal(sig, SIG_DFL);
    }
}
