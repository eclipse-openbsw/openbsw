/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <cstdarg>
#include <cstdio>

#include <etl/span.h>
#include <util/logger/Logger.h>

#include <app/DemoLogger.h>
#include <middleware/logger/Logger.h>

namespace middleware::logger
{

// NOLINTBEGIN(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg,cert-err33-c,clang-analyzer-valist.Uninitialized)
void log(LogLevel const level, char const* const f, ...)
{
    std::va_list ap;
    va_start(ap, f);
    char buffer[256];
    (void)vsnprintf(buffer, sizeof(buffer), f, ap);
    va_end(ap);
    // NOLINTEND(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg,cert-err33-c,clang-analyzer-valist.Uninitialized)

    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    switch (level)
    {
        case LogLevel::Critical:
            ::util::logger::Logger::critical(::util::logger::DEMO, "%s", buffer);
            break;
        case LogLevel::Error:
            ::util::logger::Logger::error(::util::logger::DEMO, "%s", buffer);
            break;
        case LogLevel::Warning:
            ::util::logger::Logger::warn(::util::logger::DEMO, "%s", buffer);
            break;
        case LogLevel::Info:
            ::util::logger::Logger::info(::util::logger::DEMO, "%s", buffer);
            break;
        case LogLevel::Debug:
        case LogLevel::Trace:
            ::util::logger::Logger::debug(::util::logger::DEMO, "%s", buffer);
            break;
        default: break;
    }
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

void logBinary(LogLevel const /*level*/, etl::span<uint8_t const> const /*data*/) {}

uint32_t getMessageId(Error const /*id*/) { return 0U; }

} // namespace middleware::logger
