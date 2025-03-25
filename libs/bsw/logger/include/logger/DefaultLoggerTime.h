// Copyright 2024 Accenture.

#pragma once

#include "logger/ILoggerTime.h"

#include <util/format/StringWriter.h>

#include <estd/chrono.h>

namespace logger
{
template<
    class Timepoint = ::estd::chrono::time_point<
        ::estd::chrono::high_resolution_clock,
        ::estd::chrono::duration<uint32_t, ::estd::milli>>>
class DefaultLoggerTime : public ILoggerTime<typename Timepoint::duration::rep>
{
public:
    using TimestampType = typename Timepoint::duration::rep;

    explicit DefaultLoggerTime(char const* timeFormat);

    TimestampType getTimestamp() const override;
    void formatTimestamp(
        ::util::stream::IOutputStream& outputStream, TimestampType const& timestamp) const override;

private:
    char const* _timeFormat;
};

template<class Timepoint>
DefaultLoggerTime<Timepoint>::DefaultLoggerTime(char const* const timeFormat)
: ILoggerTime<typename Timepoint::duration::rep>(), _timeFormat(timeFormat)
{}

template<class Timepoint>
typename DefaultLoggerTime<Timepoint>::TimestampType
DefaultLoggerTime<Timepoint>::getTimestamp() const
{
    return ::estd::chrono::time_point_cast<typename Timepoint::duration>(Timepoint::clock::now())
        .time_since_epoch()
        .count();
}

template<class Timepoint>
void DefaultLoggerTime<Timepoint>::formatTimestamp(
    ::util::stream::IOutputStream& outputStream, TimestampType const& timestamp) const
{
    ::util::format::StringWriter writer(outputStream);
    (void)writer.printf(_timeFormat, timestamp);
}

} /* namespace logger */

