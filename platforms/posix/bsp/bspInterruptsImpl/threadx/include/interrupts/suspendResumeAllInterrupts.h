// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

void setThreadXInitialized();

typedef uint32_t OldIntEnabledStatusValueType;

#define getMachineStateRegisterValueAndSuspendAllInterrupts \
    getOldIntEnabledStatusValueAndSuspendAllInterrupts

OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void);

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue);
