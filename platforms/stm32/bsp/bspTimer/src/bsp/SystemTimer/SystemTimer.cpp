// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file SystemTimer.cpp
 * \brief DWT-based system timer for STM32 Cortex-M4 (F4 and G4 families)
 *
 * Uses the ARM DWT cycle counter (CYCCNT) for high-resolution timing.
 * No chip-specific peripherals — DWT is part of the Cortex-M4 debug unit.
 */

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

uint32_t const TICK_FREQ_MHZ = 1U; // 1 tick = 1 us

struct
{
    uint64_t ticks;
    uint32_t lastDwt;
} state = {0, 0};

// DWT registers (ARMv7-M architecture reference)
uint32_t volatile& DWT_CTRL   = *reinterpret_cast<uint32_t volatile*>(0xE0001000U);
uint32_t volatile& DWT_CYCCNT = *reinterpret_cast<uint32_t volatile*>(0xE0001004U);
uint32_t volatile& DEMCR      = *reinterpret_cast<uint32_t volatile*>(0xE000EDFCU);

uint64_t updateTicks()
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;
    uint32_t const curDwt = DWT_CYCCNT;
    state.ticks += static_cast<uint32_t>((curDwt - state.lastDwt) / DWT_FREQ_MHZ);
    state.lastDwt = curDwt;
    return state.ticks;
}

} // namespace

extern "C"
{
void initSystemTimer()
{
    const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;

    // Enable DWT cycle counter
    DEMCR      = DEMCR | 0x01000000U; // TRCENA
    DWT_CYCCNT = 0;
    DWT_CTRL   = DWT_CTRL | 0x00000001U; // CYCCNTENA

    state.ticks   = 0;
    state.lastDwt = 0;
}

uint64_t getSystemTicks(void) { return updateTicks(); }

uint32_t getSystemTicks32Bit(void) { return static_cast<uint32_t>(updateTicks()); }

uint64_t getSystemTimeNs(void) { return updateTicks() * 1000U / TICK_FREQ_MHZ; }

uint64_t getSystemTimeUs(void) { return updateTicks() / TICK_FREQ_MHZ; }

uint32_t getSystemTimeUs32Bit(void) { return static_cast<uint32_t>(updateTicks() / TICK_FREQ_MHZ); }

uint64_t getSystemTimeMs(void) { return updateTicks() / TICK_FREQ_MHZ / 1000U; }

uint32_t getSystemTimeMs32Bit(void)
{
    return static_cast<uint32_t>(updateTicks() / TICK_FREQ_MHZ / 1000U);
}

uint64_t systemTicksToTimeUs(uint64_t const ticks) { return ticks / TICK_FREQ_MHZ; }

uint64_t systemTicksToTimeNs(uint64_t const ticks) { return ticks * 1000U / TICK_FREQ_MHZ; }

uint32_t getFastTicks(void) { return DWT_CYCCNT; }

uint32_t getFastTicksPerSecond(void) { return DWT_FREQ_MHZ * 1000000U; }

void sysDelayUs(uint32_t const delay)
{
    uint64_t const start = getSystemTimeUs();
    while (getSystemTimeUs() < start + delay) {}
}

} // extern "C"
