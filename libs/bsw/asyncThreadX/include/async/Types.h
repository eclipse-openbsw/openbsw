// Copyright 2025 Accenture.

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
