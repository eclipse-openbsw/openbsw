// Copyright 2024 Accenture.

#pragma once

#include "logger/IEntryFormatter.h"
#include "logger/ILoggerTime.h"

#include <util/format/PrintfFormatter.h>
#include <util/format/StringWriter.h>
#include <util/format/Vt100AttributedStringFormatter.h>
#include <util/logger/ComponentInfo.h>
#include <util/logger/LevelInfo.h>

namespace logger
{
template<class E = uint32_t, class Timestamp = uint32_t>
class DefaultEntryFormatter : public IEntryFormatter<E, Timestamp>
{
public:
    explicit DefaultEntryFormatter(ILoggerTime<Timestamp>& loggerTime);

    void formatEntry(
        ::util::stream::IOutputStream& outputStream,
        E entryIndex,
        Timestamp timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) const override;

private:
    ILoggerTime<Timestamp>& _loggerTime;
};

template<class E, class Timestamp>
DefaultEntryFormatter<E, Timestamp>::DefaultEntryFormatter(ILoggerTime<Timestamp>& loggerTime)
: IEntryFormatter<E, Timestamp>(), _loggerTime(loggerTime)
{}

template<class E, class Timestamp>
void DefaultEntryFormatter<E, Timestamp>::formatEntry(
    ::util::stream::IOutputStream& outputStream,
    E const /* entryIndex */,
    Timestamp const timestamp,
    ::util::logger::ComponentInfo const& componentInfo,
    ::util::logger::LevelInfo const& levelInfo,
    char const* const str,
    ::util::format::IPrintfArgumentReader& argReader) const
{
    ::util::format::StringWriter writer(outputStream);
    ::util::format::Vt100AttributedStringFormatter vt100Formatter;
    _loggerTime.formatTimestamp(outputStream, timestamp);

    auto const compFormat  = vt100Formatter.write(componentInfo.getName());
    auto const levelFormat = vt100Formatter.write(levelInfo.getName());
    (void)writer.write(": ")
        .apply(compFormat)
        .write(": ")
        .apply(levelFormat)
        .write(": ")
        .vprintf(str, argReader);
}

} // namespace logger
