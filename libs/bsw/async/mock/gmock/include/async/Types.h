// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_E24360C6_00A9_42FF_B33B_F82F9E9A64B8
#define GUARD_E24360C6_00A9_42FF_B33B_F82F9E9A64B8

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
    virtual void execute() = 0;
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

#endif // GUARD_E24360C6_00A9_42FF_B33B_F82F9E9A64B8
