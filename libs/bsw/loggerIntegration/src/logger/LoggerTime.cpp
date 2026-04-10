// Copyright 2024 Accenture.

#include "logger/LoggerTime.h"

#include <etl/chrono.h>
#include <etl/span.h>

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
    ::std::time_t seconds   = static_cast<::std::time_t>(timestamp / 1000);
    uint32_t const mSeconds = timestamp % 1000;

    size_t const timestampBufferSize = 50;
    char timestampBuffer[timestampBufferSize];
    ::std::tm* localTime   = ::std::localtime(&seconds);
    size_t timestampLength = 0;

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

    if (timestampLength > 0)
    {
        stream.write(::etl::span<uint8_t const>(
            static_cast<uint8_t const*>(static_cast<void const*>(timestampBuffer)),
            timestampLength));
        // Append milliseconds (always 3 digits, zero-padded)
        uint8_t const msStr[]
            = {static_cast<uint8_t>('.'),
               static_cast<uint8_t>('0' + (mSeconds / 100U) % 10U),
               static_cast<uint8_t>('0' + (mSeconds / 10U) % 10U),
               static_cast<uint8_t>('0' + mSeconds % 10U)};
        stream.write(::etl::span<uint8_t const>(msStr, sizeof(msStr)));
    }
}

} // namespace logger
