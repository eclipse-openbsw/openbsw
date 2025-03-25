// Copyright 2024 Accenture.

#pragma once

#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <logger/BufferedLoggerOutput.h>

namespace logger
{
using EntryIndexType = uint32_t;
using TimestampType  = uint32_t;

using BufferedLoggerOutputType = ::logger::declare::BufferedLoggerOutput<
    1024u * 8u,                                         // BufferSize
    ::interrupts::SuspendResumeAllInterruptsScopedLock, // Lock
    128,                                                // MaxEntrySize
    uint16_t,                                           // T
    EntryIndexType,                                     // EntryIndexType
    TimestampType>;                                     // TimestampType

} // namespace logger
