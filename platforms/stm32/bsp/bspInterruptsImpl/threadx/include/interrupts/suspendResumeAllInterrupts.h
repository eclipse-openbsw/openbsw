/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <platform/estdint.h>

#include <tx_api.h>

static inline __attribute__((always_inline)) void setThreadXInitialized(){};

typedef uint32_t OldIntEnabledStatusValueType;

#define getMachineStateRegisterValueAndSuspendAllInterrupts \
    getOldIntEnabledStatusValueAndSuspendAllInterrupts

static inline __attribute__((always_inline)) OldIntEnabledStatusValueType
getOldIntEnabledStatusValueAndSuspendAllInterrupts(void)
{
    return tx_interrupt_control(TX_INT_DISABLE);
}

static inline __attribute__((always_inline)) void
resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue)
{
    tx_interrupt_control(oldIntEnabledStatusValue);
}
