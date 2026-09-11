/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains
 * \file
 * \ingroup bspConfig
 */
#include "bsp/timer/SystemTimer.h"

#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

#include <etl/chrono.h>
#include <etl/platform.h>
#include <pico/stdlib.h>

namespace
{
uint32_t const DWT_FREQ_MHZ_RUN  = 80U;
uint32_t const DWT_FREQ_MHZ_IDLE = 48U;
uint32_t const TICK_FREQ_MHZ     = 1U; // Common divider of all DWT frequencies

struct
{
    uint64_t ticks; // Main ticks counter, never overflows
    uint32_t lastDwt;
    uint32_t dwtTicksRatio;
    uint32_t dwtFreqMhz;
} state = {0, 0, DWT_FREQ_MHZ_RUN / TICK_FREQ_MHZ, DWT_FREQ_MHZ_RUN};

// DWT registers (refer to ARMv7-M architecture reference manual)
#if 0
uint32_t volatile& DWT_CTRL   = *reinterpret_cast<uint32_t volatile*>(0xE0001000U);
uint32_t volatile& DWT_CYCCNT = *reinterpret_cast<uint32_t volatile*>(0xE0001004U);
uint32_t volatile& DEMCR      = *reinterpret_cast<uint32_t volatile*>(0xE000EDFCU);
#endif

// Must be called more than once per DWT period (~53 sec for 80 MHz frequency). Called implicitly
// through all getSomething() functions
uint64_t updateTicks()
{
#if 0
    const ETL_MAYBE_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;
    uint32_t const curDwt = DWT_CYCCNT;
    state.ticks += static_cast<uint32_t>((curDwt - state.lastDwt) / state.dwtTicksRatio);
    state.lastDwt = curDwt;
    return state.ticks;
#endif
    return to_us_since_boot(get_absolute_time());
}

} // namespace

extern "C"
{
void initSystemTimer()
{
    ETL_MAYBE_UNUSED const interrupts::SuspendResumeAllInterruptsScopedLock lock;

    state.ticks         = 0; // General ticks counter, never overflows
    state.lastDwt       = 0;
    state.dwtTicksRatio = DWT_FREQ_MHZ_RUN / TICK_FREQ_MHZ;
    state.dwtFreqMhz    = DWT_FREQ_MHZ_RUN;
}

void initSystemTimerHelper(bool const sleep)
{
    (void)updateTicks();
    {
        ETL_MAYBE_UNUSED const interrupts::SuspendResumeAllInterruptsScopedLock lock;
        state.dwtFreqMhz    = sleep ? DWT_FREQ_MHZ_IDLE : DWT_FREQ_MHZ_RUN;
        state.dwtTicksRatio = state.dwtFreqMhz / TICK_FREQ_MHZ;
    }
}

uint64_t getSystemTicks(void) { return to_us_since_boot(get_absolute_time()); }

uint32_t getSystemTicks32Bit(void)
{
    return static_cast<uint32_t>(to_us_since_boot(get_absolute_time()));
}

uint64_t getSystemTimeNs(void) { return to_us_since_boot(get_absolute_time()) * 1000U; }

uint64_t getSystemTimeUs(void) { return to_us_since_boot(get_absolute_time()); }

uint32_t getSystemTimeUs32Bit(void)
{
    return static_cast<uint32_t>(to_us_since_boot(get_absolute_time()));
}

uint64_t getSystemTimeMs(void) { return to_ms_since_boot(get_absolute_time()); }

etl::chrono::high_resolution_clock::rep etl_get_high_resolution_clock()
{
    return etl::chrono::high_resolution_clock::rep{static_cast<int64_t>(getSystemTimeNs())};
}

etl::chrono::system_clock::rep etl_get_system_clock()
{
    return etl::chrono::system_clock::rep(static_cast<int64_t>(getSystemTimeMs()));
}

etl::chrono::steady_clock::rep etl_get_steady_clock()
{
    return etl::chrono::steady_clock::rep(static_cast<int64_t>(getSystemTimeMs() / 1000));
}

uint32_t getSystemTimeMs32Bit(void) { return static_cast<uint32_t>(getSystemTimeMs()); }

uint64_t systemTicksToTimeUs(uint64_t const ticks) { return ticks / TICK_FREQ_MHZ; }

uint64_t systemTicksToTimeNs(uint64_t const ticks) { return ticks * 1000U / TICK_FREQ_MHZ; }

uint32_t getFastTicks(void) { return {}; }

uint32_t getFastTicksPerSecond(void)
{
    ETL_MAYBE_UNUSED const interrupts::SuspendResumeAllInterruptsScopedLock lock;
    return state.dwtFreqMhz * 1000000;
}

void sysDelayUs(uint32_t const delay) { sleep_us(delay); }

} // extern "C"
