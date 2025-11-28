// Copyright 2024 Accenture.

#pragma once

#include "logger/EntryBuffer.h"
#include "logger/EntrySerializer.h"
#include "logger/IEntryOutput.h"
#include "logger/ILoggerListener.h"
#include "logger/ILoggerTime.h"

#include <etl/intrusive_list.h>
#include <etl/span.h>
#include <util/logger/IComponentMapping.h>
#include <util/logger/ILoggerOutput.h>

namespace logger
{
template<
    class Lock,
    uint8_t MaxEntrySize    = 64,
    class T                 = uint16_t,
    class E                 = uint32_t,
    class Timestamp         = uint32_t,
    class ReadOnlyPredicate = SectionPredicate>
class BufferedLoggerOutput : public ::util::logger::ILoggerOutput
{
public:
    using EntryIndexType = E;
    using EntryRefType   = typename EntryBuffer<MaxEntrySize, E>::EntryRef;
    using TimestampType  = Timestamp;

    BufferedLoggerOutput(
        ::util::logger::IComponentMapping& componentMapping,
        ILoggerTime<Timestamp>& timestamp,
        ::etl::span<uint8_t> outputBuffer);
    BufferedLoggerOutput(
        ::util::logger::IComponentMapping& componentMapping,
        ILoggerTime<Timestamp>& timestamp,
        ::etl::span<uint8_t> outputBuffer,
        ReadOnlyPredicate const& readOnlyPredicate);

    void addListener(ILoggerListener& listener);
    void removeListener(ILoggerListener& listener);

    bool outputEntry(IEntryOutput<E, Timestamp>& output, EntryRefType& entryRef) const;

    void logOutput(
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        va_list ap) override;

private:
    class EntryOutputAdapter : public IEntrySerializerCallback<Timestamp>
    {
    public:
        EntryOutputAdapter(
            ::util::logger::IComponentMapping& componentMapping,
            E entryIndex,
            IEntryOutput<E, Timestamp>& output);

        void onEntry(
            Timestamp timestamp,
            uint8_t componentIndex,
            ::util::logger::Level level,
            char const* str,
            ::util::format::IPrintfArgumentReader& argReader) override;

    private:
        ::util::logger::IComponentMapping& _parentComponentMapping;
        E _entryIndex;
        IEntryOutput<E, Timestamp>& _output;
    };

    ::util::logger::IComponentMapping& _componentMapping;
    ILoggerTime<Timestamp>& _timestamp;
    EntryBuffer<MaxEntrySize, E> _entryBuffer;
    EntrySerializer<T, Timestamp, ReadOnlyPredicate> _entrySerializer;
    ::etl::intrusive_list<ILoggerListener, ::etl::bidirectional_link<0>> _listeners;
};

namespace declare
{

template<
    uint32_t BufferSize,
    class Lock,
    uint8_t MaxEntrySize    = 64,
    class T                 = uint16_t,
    class E                 = uint32_t,
    class Timestamp         = uint32_t,
    class ReadOnlyPredicate = SectionPredicate>
class BufferedLoggerOutput
: public ::logger::BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>
{
public:
    BufferedLoggerOutput(
        ::util::logger::IComponentMapping& componentMapping, ILoggerTime<Timestamp>& timestamp);
    BufferedLoggerOutput(
        ::util::logger::IComponentMapping& componentMapping,
        ILoggerTime<Timestamp>& timestamp,
        ReadOnlyPredicate const readOnlyPredicate);

private:
    uint8_t _buffer[BufferSize];
};

template<
    uint32_t BufferSize,
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
inline BufferedLoggerOutput<BufferSize, Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::
    BufferedLoggerOutput(
        ::util::logger::IComponentMapping& componentMapping, ILoggerTime<Timestamp>& timestamp)
: ::logger::BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>(
    componentMapping, timestamp, _buffer)
, _buffer()
{}

template<
    uint32_t BufferSize,
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
inline BufferedLoggerOutput<BufferSize, Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::
    BufferedLoggerOutput(
        ::util::logger::IComponentMapping& componentMapping,
        ILoggerTime<Timestamp>& timestamp,
        ReadOnlyPredicate const readOnlyPredicate)
: ::logger::BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>(
    componentMapping, timestamp, _buffer, readOnlyPredicate)
, _buffer()
{}

} // namespace declare

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::BufferedLoggerOutput(
    ::util::logger::IComponentMapping& componentMapping,
    ILoggerTime<Timestamp>& timestamp,
    ::etl::span<uint8_t> const outputBuffer)
: ::util::logger::ILoggerOutput()
, _componentMapping(componentMapping)
, _timestamp(timestamp)
, _entryBuffer(outputBuffer)
, _entrySerializer(ReadOnlyPredicate())
, _listeners()
{}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::BufferedLoggerOutput(
    ::util::logger::IComponentMapping& componentMapping,
    ILoggerTime<Timestamp>& timestamp,
    ::etl::span<uint8_t> const outputBuffer,
    ReadOnlyPredicate const& readOnlyPredicate)
: ::util::logger::ILoggerOutput()
, _componentMapping(componentMapping)
, _timestamp(timestamp)
, _entryBuffer(outputBuffer)
, _entrySerializer(readOnlyPredicate)
, _listeners()
{}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
void BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::addListener(
    ILoggerListener& listener)
{
    _listeners.push_back(listener);
}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
void BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::removeListener(
    ILoggerListener& listener)
{
    _listeners.erase(listener);
}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
bool BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::outputEntry(
    IEntryOutput<E, Timestamp>& output, EntryRefType& entryRef) const
{
    uint8_t entryBuffer[MaxEntrySize];
    uint32_t size;
    {
        Lock const lock;
        size = _entryBuffer.getNextEntry(entryBuffer, entryRef);
    }
    if (size > 0U)
    {
        EntryOutputAdapter outputAdapter(_componentMapping, entryRef.getIndex(), output);

        _entrySerializer.deserialize(::etl::span<uint8_t>(entryBuffer).first(size), outputAdapter);
    }
    return size > 0U;
}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
void BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::logOutput(
    ::util::logger::ComponentInfo const& componentInfo,
    ::util::logger::LevelInfo const& levelInfo,
    char const* const str,
    va_list ap)
{
    uint8_t entryBuffer[MaxEntrySize];
    Timestamp const timestamp = _timestamp.getTimestamp();
    T const size              = _entrySerializer.serialize(
        entryBuffer, timestamp, componentInfo.getIndex(), levelInfo.getLevel(), str, ap);
    {
        Lock const lock;
        _entryBuffer.addEntry(::etl::span<uint8_t>(entryBuffer).first(size));
    }
    for (auto& it : _listeners)
    {
        it.logAvailable();
    }
}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::EntryOutputAdapter::
    EntryOutputAdapter(
        ::util::logger::IComponentMapping& componentMapping,
        E const entryIndex,
        IEntryOutput<E, Timestamp>& output)
: IEntrySerializerCallback<Timestamp>()
, _parentComponentMapping(componentMapping)
, _entryIndex(entryIndex)
, _output(output)
{}

template<
    class Lock,
    uint8_t MaxEntrySize,
    class T,
    class E,
    class Timestamp,
    class ReadOnlyPredicate>
void BufferedLoggerOutput<Lock, MaxEntrySize, T, E, Timestamp, ReadOnlyPredicate>::
    EntryOutputAdapter::onEntry(
        Timestamp timestamp,
        uint8_t componentIndex,
        ::util::logger::Level level,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader)
{
    _output.outputEntry(
        _entryIndex,
        timestamp,
        _parentComponentMapping.getComponentInfo(componentIndex),
        _parentComponentMapping.getLevelInfo(level),
        str,
        argReader);
}

} /* namespace logger */
