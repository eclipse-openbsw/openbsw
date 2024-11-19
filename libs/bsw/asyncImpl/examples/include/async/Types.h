// Copyright 2024 Accenture.

#ifndef GUARD_C3E0A6F3_29D9_424D_B02E_5574DA6088DB
#define GUARD_C3E0A6F3_29D9_424D_B02E_5574DA6088DB
#include "async/IRunnable.h"

#include <platform/estdint.h>

namespace async
{
// EXAMPLE_BEGIN AsyncImplExample
struct ExampleLock
{};

using RunnableType  = IRunnable;
using ContextType   = uint8_t;
using EventMaskType = uint32_t;
using LockType      = ExampleLock;

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

// EXAMPLE_END AsyncImplExample
} // namespace async

#endif /* GUARD_C3E0A6F3_29D9_424D_B02E_5574DA6088DB */
