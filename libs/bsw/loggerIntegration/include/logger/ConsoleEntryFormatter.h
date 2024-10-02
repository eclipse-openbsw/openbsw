// Copyright 2024 Accenture.

#ifndef GUARD_1191192F_3EC4_46BF_9186_734B3854DA74
#define GUARD_1191192F_3EC4_46BF_9186_734B3854DA74

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
class ConsoleEntryFormatter : public IEntryFormatter<E, Timestamp>
{
public:
    explicit ConsoleEntryFormatter(ILoggerTime<Timestamp>& loggerTime, char const* name);

    virtual void formatEntry(
        ::util::stream::IOutputStream& outputStream,
        E entryIndex,
        Timestamp timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) const;

private:
    ILoggerTime<Timestamp>& _loggerTime;
    char const* _name;
};

template<class E, class Timestamp>
ConsoleEntryFormatter<E, Timestamp>::ConsoleEntryFormatter(
    ILoggerTime<Timestamp>& loggerTime, char const* name)
: IEntryFormatter<E, Timestamp>(), _loggerTime(loggerTime), _name(name)
{}

template<class E, class Timestamp>
void ConsoleEntryFormatter<E, Timestamp>::formatEntry(
    ::util::stream::IOutputStream& outputStream,
    E const entryIndex,
    Timestamp const timestamp,
    ::util::logger::ComponentInfo const& componentInfo,
    ::util::logger::LevelInfo const& levelInfo,
    char const* const str,
    ::util::format::IPrintfArgumentReader& argReader) const
{
    ::util::format::StringWriter writer(outputStream);
    ::util::format::Vt100AttributedStringFormatter vt100Formatter;
    _loggerTime.formatTimestamp(outputStream, timestamp);
    (void)writer.write(": ")
        .write(_name)
        .write(": ")
        .apply(vt100Formatter.write(componentInfo.getName()))
        .write(": ")
        .apply(vt100Formatter.write(levelInfo.getName()))
        .write(": ")
        .vprintf(str, argReader);
}

} // namespace logger

#endif /* GUARD_1191192F_3EC4_46BF_9186_734B3854DA74 */
