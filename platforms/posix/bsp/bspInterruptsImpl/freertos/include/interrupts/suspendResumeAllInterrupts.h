// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

typedef uint32_t OldIntEnabledStatusValueType;

#define getMachineStateRegisterValueAndSuspendAllInterrupts \
    getOldIntEnabledStatusValueAndSuspendAllInterrupts

OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void);

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue);
