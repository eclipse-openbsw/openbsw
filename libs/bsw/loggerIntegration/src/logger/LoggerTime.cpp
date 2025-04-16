// Copyright 2024 Accenture.

#include "logger/LoggerTime.h"

#include <etl/chrono.h>
#include <etl/span.h>
#include <util/format/StringWriter.h>

#include <cstdio>
#include <ctime>

namespace
{
int64_t const NO_INIT_BOUNDARY
    = ::etl::chrono::duration_cast<::etl::chrono::milliseconds>(::etl::chrono::hours(1)).count();

char const TIME_UNINITIALIZED_FORMAT[] = "0000-00-00 %H:%M:%S";

} // namespace

namespace logger
{
LoggerTime::LoggerTime(char const* const timestampFormat) : _timestampFormat(timestampFormat) {}

int64_t LoggerTime::getTimestamp() const
{
    using namespace ::etl::chrono;
    return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

void LoggerTime::formatTimestamp(
    ::util::stream::IOutputStream& stream, int64_t const& timestamp) const
{
    ::util::format::StringWriter writer(stream);

    ::std::time_t seconds   = static_cast<::std::time_t>(timestamp / 1000);
    uint32_t const mSeconds = timestamp % 1000;

    size_t const timestampBufferSize = 50;
    char timestampBuffer[timestampBufferSize];
    ::std::tm* localTime = ::std::localtime(&seconds);
    int timestampLength  = 0;

    if (timestamp < NO_INIT_BOUNDARY)
    {
        timestampLength = ::std::strftime(
            timestampBuffer, timestampBufferSize, TIME_UNINITIALIZED_FORMAT, localTime);
    }
    else
    {
        timestampLength
            = ::std::strftime(timestampBuffer, timestampBufferSize, _timestampFormat, localTime);
    }

    if (timestampLength != 0)
    {
        int n = snprintf(
            timestampBuffer + timestampLength,
            static_cast<uint32_t>(timestampBufferSize),
            ".%03u",
            static_cast<unsigned int>(mSeconds));
        stream.write(::etl::span<uint8_t const>(
            reinterpret_cast<uint8_t const*>(timestampBuffer), timestampLength + n));
    }
}

} // namespace logger
