/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
