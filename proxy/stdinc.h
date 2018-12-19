/*
* Copyright (C) 2018
* All rights reserved.
*/

#ifndef __RPC_API_STORAGE_H
#define __RPC_API_STORAGE_H
#pragma once

//////////////////////////////////////////////////////////////////////////

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef _MSC_VER
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _MSC_VER

#ifdef MINGW
#ifndef __STRICT_ANSI__
#define __STRICT_ANSI__
#endif

#endif

//////////////////////////////////////////////////////////////////////////

#include <string>
#include <stdexcept>
#include <cstdint>
#include <queue>
#include <vector>
#include <list>
#include <map>
#include <sstream>

#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <boost/chrono.hpp>
#include <boost/stacktrace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////////
struct SLogPackage;
typedef std::shared_ptr<std::vector<char>> PMessage;
typedef std::shared_ptr<SLogPackage> PLog;
typedef boost::signals2::signal<void(PMessage)> signal_msg_t;
typedef boost::signals2::signal<void(size_t)> signal_erase_t;
typedef boost::signals2::signal<void(const SLogPackage&)> signal_log_t;

//////////////////////////////////////////////////////////////////////////
#include "utils/strace_exception.h"
#include "utils/logbuilder.h"

//////////////////////////////////////////////////////////////////////////
#endif // __RPC_API_STORAGE_H
