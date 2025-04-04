// Copyright 2024 Accenture.

#pragma once

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
