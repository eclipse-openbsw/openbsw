// Copyright 2024 Accenture.

#ifndef GUARD_25D8D44E_A47E_44FA_97F5_504950251177
#define GUARD_25D8D44E_A47E_44FA_97F5_504950251177

#include "platform/estdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t OldIntEnabledStatusValueType;

uint32_t getMachineStateRegisterValueAndSuspendAllInterrupts(void);
OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts();

void resumeAllInterrupts(uint32_t oldMachineStateRegisterValue);

#ifdef __cplusplus
}
#endif

#endif /* GUARD_25D8D44E_A47E_44FA_97F5_504950251177 */
