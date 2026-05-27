/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup async
 */
#pragma once

#include "async/IRunnable.h"
#include "async/Lock.h"
#include "async/ModifiableLock.h"
#include "tx_api.h"

#include <timer/Timeout.h>

#include <platform/estdint.h>

namespace async
{
using RunnableType       = IRunnable;
using ContextType        = uint8_t;
using EventMaskType      = ULONG;
using LockType           = Lock;
using ModifiableLockType = ModifiableLock;

ContextType const CONTEXT_INVALID = 0xFFU;
ContextType const CONTEXT_TIMER   = 0x00U;

struct TimeoutType : public ::timer::Timeout
{
public:
    TimeoutType();

    void cancel();

    void expired() override;

    IRunnable* _runnable;
    ContextType _context;
};

struct TimeUnit
{
    enum Type
    {
        MICROSECONDS = 1,
        MILLISECONDS = 1000,
        SECONDS      = 1000000
    };
};

using TimeUnitType = TimeUnit::Type;

} // namespace async
