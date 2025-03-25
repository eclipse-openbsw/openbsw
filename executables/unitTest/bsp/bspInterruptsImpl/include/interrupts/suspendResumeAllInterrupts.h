// Copyright 2024 Accenture.

#pragma once

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

