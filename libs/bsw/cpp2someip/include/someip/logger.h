/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

#if defined(DEBUG_LOG) || defined(INFO_LOG) || defined(WARN_LOG) || defined(ERROR_LOG) \
    || defined(CRITICAL_LOG)
#error \
    "The DEBUG_LOG, INFO_LOG, WARN_LOG, ERROR_LOG or (and) CRITICAL_LOG lexemes are defined elsewhere."
#endif // defined(DEBUG_LOG) || defined(INFO_LOG) || defined(WARN_LOG) || defined(ERROR_LOG) ||
       // defined(CRITICAL_LOG)

#ifdef CPP2SOMEIP_EXTERNAL_LOGGER // use default logger
#include <util/logger/Logger.h>
/*
 * Define
 * bool logSomeip(uint8_t, const char*)
 * function and link it to make your logger implementation work.
 */
DECLARE_LOGGER_COMPONENT(SOMEIP)

namespace someip
{
namespace logger
{

void debug(uint8_t componentIndex, char const* str, ...);
void info(uint8_t componentIndex, char const* str, ...);
void warn(uint8_t componentIndex, char const* str, ...);
void error(uint8_t componentIndex, char const* str, ...);
void critical(uint8_t componentIndex, char const* str, ...);
} // namespace logger
} // namespace someip

#define DEBUG_LOG(...)    ::someip::logger::debug(__VA_ARGS__)
#define INFO_LOG(...)     ::someip::logger::info(__VA_ARGS__)
#define WARN_LOG(...)     ::someip::logger::warn(__VA_ARGS__)
#define ERROR_LOG(...)    ::someip::logger::error(__VA_ARGS__)
#define CRITICAL_LOG(...) ::someip::logger::critical(__VA_ARGS__)
#elif defined(CPP2SOMEIP_DISABLE_LOGGING) // logging is disabled
#define DEBUG_LOG(...)
#define INFO_LOG(...)
#define WARN_LOG(...)
#define ERROR_LOG(...)
#define CRITICAL_LOG(...)
#else // use default logger
#include "someip/defaultlogger.h"
#define DEBUG_LOG(...) ::util::logger::Logger::debug(__VA_ARGS__)

#define INFO_LOG(...)     ::util::logger::Logger::info(__VA_ARGS__)
#define WARN_LOG(...)     ::util::logger::Logger::warn(__VA_ARGS__)
#define ERROR_LOG(...)    ::util::logger::Logger::error(__VA_ARGS__)
#define CRITICAL_LOG(...) ::util::logger::Logger::critical(__VA_ARGS__)
#endif // CPP2SOMEIP_EXTERNAL_LOGGER

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
