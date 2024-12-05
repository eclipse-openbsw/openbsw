// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_6CE67238_E66B_4D47_A69C_685DEC0453AD
#define GUARD_6CE67238_E66B_4D47_A69C_685DEC0453AD

#include "async/IRunnable.h"
#include "async/Lock.h"
#include "async/ModifiableLock.h"

#include <timer/Timeout.h>

#include <platform/estdint.h>

namespace async
{
using RunnableType       = IRunnable;
using ContextType        = uint8_t;
using EventMaskType      = uint32_t;
using LockType           = Lock;
using ModifiableLockType = ModifiableLock;

ContextType const CONTEXT_INVALID = 0xFFU;

/**
 * This class stores information about the Runnable
 * object and its context, as well implements the
 * "expired" function executed on timer time out.
 */
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

#endif // GUARD_6CE67238_E66B_4D47_A69C_685DEC0453AD
