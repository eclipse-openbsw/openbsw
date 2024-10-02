// Copyright 2024 Accenture.

#ifndef GUARD_9A4E28AA_3816_4B33_A616_2AA7F29B0AE3
#define GUARD_9A4E28AA_3816_4B33_A616_2AA7F29B0AE3

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

#endif /* GUARD_9A4E28AA_3816_4B33_A616_2AA7F29B0AE3 */
