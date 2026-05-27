/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
