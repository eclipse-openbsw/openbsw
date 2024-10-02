// Copyright 2024 Accenture.

#ifndef GUARD_3EAC2075_EA4F_4702_86EB_1EBE06CFB8B2
#define GUARD_3EAC2075_EA4F_4702_86EB_1EBE06CFB8B2

#include "logger/IEntryFormatter.h"
#include "logger/IEntryOutput.h"

#include <util/stream/ISharedOutputStream.h>

namespace logger
{
template<class E = uint32_t, class Timestamp = uint32_t>
class SharedStreamEntryOutput : public IEntryOutput<E, Timestamp>
{
public:
    explicit SharedStreamEntryOutput(
        ::util::stream::ISharedOutputStream& sharedstream,
        IEntryFormatter<E, Timestamp>& formatter);

    void outputEntry(
        E entryIndex,
        Timestamp timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) override;

private:
    ::util::stream::ISharedOutputStream& _sharedstream;
    IEntryFormatter<E, Timestamp>& _formatter;
};

template<class E, class Timestamp>
SharedStreamEntryOutput<E, Timestamp>::SharedStreamEntryOutput(
    ::util::stream::ISharedOutputStream& sharedstream, IEntryFormatter<E, Timestamp>& formatter)
: IEntryOutput<E, Timestamp>(), _sharedstream(sharedstream), _formatter(formatter)
{}

template<class E, class Timestamp>
void SharedStreamEntryOutput<E, Timestamp>::outputEntry(
    E const entryIndex,
    Timestamp const timestamp,
    ::util::logger::ComponentInfo const& componentInfo,
    ::util::logger::LevelInfo const& levelInfo,
    char const* const str,
    ::util::format::IPrintfArgumentReader& argReader)
{
    ::util::stream::IOutputStream& outstream = _sharedstream.startOutput();
    _formatter.formatEntry(
        outstream, entryIndex, timestamp, componentInfo, levelInfo, str, argReader);
    _sharedstream.endOutput();
}

} // namespace logger

#endif // GUARD_3EAC2075_EA4F_4702_86EB_1EBE06CFB8B2
