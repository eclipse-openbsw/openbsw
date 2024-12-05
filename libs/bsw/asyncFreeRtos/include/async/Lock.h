// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_C018654B_ADA3_4244_B77C_1B663003B727
#define GUARD_C018654B_ADA3_4244_B77C_1B663003B727

#include "interrupts/suspendResumeAllInterrupts.h"

namespace async
{
/**
 * A synchronization mechanism that blocks threads from accessing a resource.
 *
 * The Lock class ensures mutual exclusion, allowing only one thread to access a
 * protected resource or function at a time. When a thread acquires the lock,
 * any other thread attempting to acquire it is blocked until the lock is released.
 * The lock is automatically released in the destructor (RAII idiom).
 */
class Lock
{
public:
    Lock();
    ~Lock();

private:
    OldIntEnabledStatusValueType _oldIntEnabledStatusValue;
};

/**
 * Inline implementations.
 */
inline Lock::Lock()
: _oldIntEnabledStatusValue(getOldIntEnabledStatusValueAndSuspendAllInterrupts())
{}

inline Lock::~Lock() { resumeAllInterrupts(_oldIntEnabledStatusValue); }

} // namespace async

#endif // GUARD_C018654B_ADA3_4244_B77C_1B663003B727
