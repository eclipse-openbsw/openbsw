// Copyright 2024 Accenture.

#ifndef GUARD_EEE4C214_797F_42CB_99BF_3B2CDF2ED3F4
#define GUARD_EEE4C214_797F_42CB_99BF_3B2CDF2ED3F4

#include <platform/estdint.h>

typedef uint32_t OldIntEnabledStatusValueType;

#define getMachineStateRegisterValueAndSuspendAllInterrupts \
    getOldIntEnabledStatusValueAndSuspendAllInterrupts

inline OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void)
{
    return 0;
}

inline void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue)
{
    // N/A
}

#endif // GUARD_EEE4C214_797F_42CB_99BF_3B2CDF2ED3F4
