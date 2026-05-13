// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include <stdint.h>

// clang-format off

static inline __attribute__((always_inline))
void disableAllInterrupts(void)
{
asm(
"cpsid   i;"
"ISB;"
"DSB;"
"DMB;"
);
}

static inline __attribute__((always_inline))
void enableAllInterrupts(void)
{
asm(
"ISB;"
"DSB;"
"DMB;"
"cpsie   i;"
);
}

static inline __attribute__((always_inline))
uint32_t getMachineStateRegisterValueAndSuspendAllInterrupts(void)
{
    uint32_t _PRIMASK;
    asm volatile ("mrs %0, PRIMASK\n cpsid i\n" : "=r" (_PRIMASK));
    return(_PRIMASK);
}

static inline __attribute__((always_inline))
void resumeAllInterrupts(uint32_t oldMachineStateRegisterValue)
{
    asm volatile ("msr PRIMASK,%[Input]\n" :: [Input] "r" (oldMachineStateRegisterValue));
}

// clang-format on
