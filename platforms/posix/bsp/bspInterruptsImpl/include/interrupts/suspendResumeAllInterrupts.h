// Copyright 2024 Accenture.

#ifndef GUARD_C1EF7B9A_8339_4AB2_A32F_CD5B00D8E4CE
#define GUARD_C1EF7B9A_8339_4AB2_A32F_CD5B00D8E4CE

#include <platform/estdint.h>

typedef uint32_t OldIntEnabledStatusValueType;

#define getMachineStateRegisterValueAndSuspendAllInterrupts \
    getOldIntEnabledStatusValueAndSuspendAllInterrupts

OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void);

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue);

#endif /* GUARD_C1EF7B9A_8339_4AB2_A32F_CD5B00D8E4CE */
