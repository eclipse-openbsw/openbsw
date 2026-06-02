/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/timer/SystemTimer.h"

#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

namespace
{
#if defined(STM32_FAMILY_F4)
uint32_t const DWT_FREQ_MHZ = 96U; // F413ZH: 96 MHz HCLK
#elif defined(STM32_FAMILY_G4)
uint32_t const DWT_FREQ_MHZ = 170U; // G474RE: 170 MHz HCLK
#else
#error "Define STM32_FAMILY_F4 or STM32_FAMILY_G4"
#endif

// One tick equals one microsecond.
struct
{
    uint64_t ticks;
    uint32_t lastDwt;
} state = {0, 0};

uint64_t updateTicks()
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;
    uint32_t const curDwt    = DWT->CYCCNT;
    uint32_t const elapsedUs = (curDwt - state.lastDwt) / DWT_FREQ_MHZ;
    state.ticks += elapsedUs;
    // Consume only whole microseconds so the sub-microsecond cycle remainder
    // carries over to the next call. Setting lastDwt to curDwt would discard
    // up to DWT_FREQ_MHZ - 1 cycles per call, freezing the clock for callers
    // polling faster than once per microsecond.
    state.lastDwt += elapsedUs * DWT_FREQ_MHZ;
    return state.ticks;
}

} // namespace

extern "C"
{
void initSystemTimer()
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    state.ticks   = 0;
    state.lastDwt = 0;
}

uint64_t getSystemTicks(void) { return updateTicks(); }

uint32_t getSystemTicks32Bit(void) { return static_cast<uint32_t>(updateTicks()); }

uint64_t getSystemTimeNs(void) { return updateTicks() * 1000U; }

uint64_t getSystemTimeUs(void) { return updateTicks(); }

uint32_t getSystemTimeUs32Bit(void) { return static_cast<uint32_t>(updateTicks()); }

uint64_t getSystemTimeMs(void) { return updateTicks() / 1000U; }

uint32_t getSystemTimeMs32Bit(void) { return static_cast<uint32_t>(updateTicks() / 1000U); }

uint64_t systemTicksToTimeUs(uint64_t const ticks) { return ticks; }

uint64_t systemTicksToTimeNs(uint64_t const ticks) { return ticks * 1000U; }

uint32_t getFastTicks(void) { return DWT->CYCCNT; }

uint32_t getFastTicksPerSecond(void) { return DWT_FREQ_MHZ * 1000000U; }

void sysDelayUs(uint32_t const delay)
{
    uint64_t const start = getSystemTimeUs();
    while (getSystemTimeUs() < start + delay) {}
}

} // extern "C"
