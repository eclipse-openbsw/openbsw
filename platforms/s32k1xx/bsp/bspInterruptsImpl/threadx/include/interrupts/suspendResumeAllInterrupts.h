// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

#include <tx_api.h>
#include <unistd.h>

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
