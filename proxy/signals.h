/*
* Copyright (C) 2018 Rhonda Software.
* All rights reserved.
*/

#pragma once
#include <csignal>
#include <vector>

//////////////////////////////////////////////////////////////////////////
extern std::string SignalToString(int sigId);
extern int StringToSignal(std::string sigName);

//////////////////////////////////////////////////////////////////////////
class CSignalException : public std::runtime_error
{
public:
    CSignalException(int sig)
        : std::runtime_error("Error setting up signal handlers: " + SignalToString(sig))
    {}

private:
};

//////////////////////////////////////////////////////////////////////////
class CSignalHandler
{
public:
    CSignalHandler(const std::vector<int>& sig) :
        m_signals(sig)
    {
        SetHandler();
    }

    ~CSignalHandler()
    {
        UnsetHandler();
    }

    static void Handler(int sig);

private:
    void SetHandler();
    void UnsetHandler();

private:
    std::vector<int> m_signals;
};