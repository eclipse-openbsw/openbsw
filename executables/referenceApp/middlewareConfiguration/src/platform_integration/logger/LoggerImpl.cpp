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

#include <etl/span.h>
#include <util/logger/Logger.h>

#include <app/DemoLogger.h>
#include <middleware/logger/Logger.h>

namespace middleware::logger
{

// NOLINTBEGIN(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg)
void log(LogLevel const level, char const* const f, ...)
{
    std::va_list ap;
    va_start(ap, f);
    auto const logLevel = [&]()
    {
        switch (level)
        {
            case LogLevel::Critical: return ::util::logger::LEVEL_CRITICAL;
            case LogLevel::Error:    return ::util::logger::LEVEL_ERROR;
            case LogLevel::Warning:  return ::util::logger::LEVEL_WARN;
            case LogLevel::Info:     return ::util::logger::LEVEL_INFO;
            case LogLevel::Debug:
            case LogLevel::Trace:    return ::util::logger::LEVEL_DEBUG;
            case LogLevel::None:     break;
        }
        return ::util::logger::LEVEL_NONE;
    }();
    ::util::logger::Logger::log(::util::logger::DEMO, logLevel, f, ap);
    va_end(ap);
}

// NOLINTEND(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg)

void logBinary(LogLevel const /*level*/, etl::span<uint8_t const> const /*data*/) {}

uint32_t getMessageId(Error const /*id*/) { return 0U; }

} // namespace middleware::logger
