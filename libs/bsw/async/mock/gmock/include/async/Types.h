// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "platform/estdint.h"

namespace async
{
using ContextType = uint8_t;

ContextType const CONTEXT_INVALID = 0xFFU;

class LockType
{
public:
    LockType();
    ~LockType();
};

class ModifiableLockType
{
public:
    ModifiableLockType();
    ~ModifiableLockType();
    void unlock();
    void lock();

private:
    bool _isLocked;
};

class RunnableType
{
public:
    virtual ~RunnableType() = default;
    virtual void execute()  = 0;
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

class TimeoutType
{
public:
    void cancel();
};

} // namespace async

