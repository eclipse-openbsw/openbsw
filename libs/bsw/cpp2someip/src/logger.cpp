/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#ifdef CPP2SOMEIP_EXTERNAL_LOGGER

#include "someip/logger.h"

#include "util/logger/Logger.h"

#include <util/format/StringWriter.h>
#include <util/stream/ByteBufferOutputStream.h>

#include <cstdarg>
#include <cstdint>

extern "C"
{
extern bool logSomeip(uint8_t lvl, char const* message);
}

namespace someip
{

namespace logger
{

namespace
{

static constexpr bool (*isEnabled)(uint8_t const, ::util::logger::Level const)
    = ::util::logger::Logger::isEnabled;

char const* componentToString(uint8_t c)
{
    if (c == ::util::logger::SOMEIP)
    {
        return "SOMEIP";
    }
    return "UNKNOWN";
}

bool doLog(uint8_t lvl, uint8_t c, char const* const fmtstring, va_list ap)
{
    uint8_t buffer[100];
    ::util::stream::ByteBufferOutputStream outputStream(buffer);
    ::util::format::StringWriter formatter(outputStream);
    formatter.write(componentToString(c));
    formatter.write(": ");
    formatter.vprintf(fmtstring, ap);
    return logSomeip(static_cast<uint8_t>(lvl), reinterpret_cast<char*>(&buffer[0]));
}

} // namespace

using namespace ::util::logger;

void debug(uint8_t const componentIndex, char const* const str, ...) { LOGGER_DOLOG(LEVEL_DEBUG) }

void info(uint8_t const componentIndex, char const* const str, ...) { LOGGER_DOLOG(LEVEL_INFO) }

void warn(uint8_t const componentIndex, char const* const str, ...) { LOGGER_DOLOG(LEVEL_WARN) }

void error(uint8_t const componentIndex, char const* const str, ...) { LOGGER_DOLOG(LEVEL_ERROR) }

void critical(uint8_t const componentIndex, char const* const str, ...)
{
    LOGGER_DOLOG(LEVEL_CRITICAL)
}

} // namespace logger
} // namespace someip

#endif // CPP2SOMEIP_EXTERNAL_LOGGER
