/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <climits>
#include <cstdarg>
#include <cstdio>

#include <etl/span.h>
#include <middleware/logger/Logger.h>

#include "Logger.h"

namespace middleware::logger
{

static char const* get_log_level_name(LogLevel level)
{
    static char const* level_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
    auto const idx                   = static_cast<uint8_t>(level);
    return (idx > 0U && idx < 7U) ? level_names[idx - 1U] : "UNKNOWN";
}

// NOLINTBEGIN(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg,cert-err33-c,clang-analyzer-valist.Uninitialized)
void log(LogLevel const level, char const* const f, ...)
{
    std::va_list ap;
    va_start(ap, f);
    char msg[PIPE_BUF];
    (void)vsnprintf(msg, sizeof(msg), f, ap);
    va_end(ap);
    // NOLINTEND(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg,cert-err33-c,clang-analyzer-valist.Uninitialized)

    if (level > LogLevel::None)
    {
        simulation::Logger::log("[", get_log_level_name(level), "] ", msg);
    }
    else
    {
        simulation::Logger::log(msg);
    }
}

void logBinary(LogLevel const level, ::etl::span<uint8_t const> const data)
{
    (void)level;
    (void)data;
}

uint32_t getMessageId(Error const id)
{
    (void)id;
    return 0U;
}

} // namespace middleware::logger
