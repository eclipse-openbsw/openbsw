// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_5D6895FC_B608_4C8E_87FB_B600671CA5B0
#define GUARD_5D6895FC_B608_4C8E_87FB_B600671CA5B0

#include "interrupts/suspendResumeAllInterrupts.h"

namespace async
{
/**
 * A synchronization mechanism that blocks threads from accessing a resource.
 *
 * The `ModifiableLock` class ensures mutual exclusion,
 * allowing only one thread to access a protected
 * resource or function at a time. When a thread acquires the lock,
 * any other thread attempting to acquire it is blocked until the lock is released.
 * The lock can be acquired either in the constructor or on demand using the `lock` function.
 * Similarly, the lock can be released either in the destructor or on demand using the `unlock`
 * function.
 */
class ModifiableLock final
{
public:
    ModifiableLock();
    ~ModifiableLock();

    void unlock();
    void lock();

private:
    ::OldIntEnabledStatusValueType _oldIntEnabledStatusValue;
    bool _isLocked;
};

/**
 * Inline implementations.
 */
inline ModifiableLock::ModifiableLock()
: _oldIntEnabledStatusValue(::getOldIntEnabledStatusValueAndSuspendAllInterrupts()), _isLocked(true)
{}

inline ModifiableLock::~ModifiableLock()
{
    if (_isLocked)
    {
        ::resumeAllInterrupts(_oldIntEnabledStatusValue);
    }
}

inline void ModifiableLock::unlock()
{
    if (_isLocked)
    {
        ::resumeAllInterrupts(_oldIntEnabledStatusValue);
        _isLocked = false;
    }
}

inline void ModifiableLock::lock()
{
    if (!_isLocked)
    {
        _oldIntEnabledStatusValue = ::getOldIntEnabledStatusValueAndSuspendAllInterrupts();
        _isLocked                 = true;
    }
}

} // namespace async

#endif // GUARD_5D6895FC_B608_4C8E_87FB_B600671CA5B0
