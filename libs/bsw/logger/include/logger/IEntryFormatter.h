// Copyright 2024 Accenture.

#ifndef GUARD_0AA82C77_8DC8_4A1D_9C80_E8BDD1C7A950
#define GUARD_0AA82C77_8DC8_4A1D_9C80_E8BDD1C7A950

#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace util
{
namespace logger
{
class ComponentInfo;
class LevelInfo;
} // namespace logger

namespace format
{
class IPrintfArgumentReader;
}

namespace stream
{
class IOutputStream;
}

} // namespace util

namespace logger
{
template<class E = uint32_t, class Timestamp = uint32_t>
class IEntryFormatter : private ::estd::uncopyable
{
public:
    using EntryIndexType = E;
    using TimestampType  = Timestamp;

    IEntryFormatter();

    virtual void formatEntry(
        ::util::stream::IOutputStream& outputStream,
        E entryIndex,
        Timestamp timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) const
        = 0;
};

template<class E, class Timestamp>
inline IEntryFormatter<E, Timestamp>::IEntryFormatter() : ::estd::uncopyable()
{}

} // namespace logger

#endif // GUARD_0AA82C77_8DC8_4A1D_9C80_E8BDD1C7A950
