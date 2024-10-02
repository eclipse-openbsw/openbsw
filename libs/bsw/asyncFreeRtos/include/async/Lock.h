// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_C018654B_ADA3_4244_B77C_1B663003B727
#define GUARD_C018654B_ADA3_4244_B77C_1B663003B727

#include "interrupts/suspendResumeAllInterrupts.h"

namespace async
{

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
