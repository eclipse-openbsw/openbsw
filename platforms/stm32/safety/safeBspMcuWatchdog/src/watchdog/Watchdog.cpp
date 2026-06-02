/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <watchdog/Watchdog.h>

#include <mcu/mcu.h>

namespace safety
{
namespace bsp
{

uint32_t Watchdog::watchdogServiceCounter = 0U;

// Bounded busy-wait for the IWDG PR/RLR update handshake. A PR/RLR update
// synchronizes into the LSI clock domain within 5 LSI cycles (~156 us at
// 32 kHz); 100k iterations is orders of magnitude above that. A plain cycle
// count is used instead of bsp::isEqualAfterTimeout so this register-level
// driver stays free of the bsp timer dependency - enableWatchdog() runs
// during early board bring-up, before the system timer is guaranteed to be
// running.
static constexpr uint32_t SYNC_TIMEOUT_CYCLES = 100000U;

void Watchdog::computePrescalerAndReload(
    uint32_t timeoutMs, uint32_t clockHz, uint8_t& prescalerCode, uint16_t& reload)
{
    // IWDG prescaler dividers: /4, /8, /16, /32, /64, /128, /256
    // prescalerCode 0..6 -> divider = 4 << code
    //
    // timeout_ms = (reload + 1) * divider * 1000 / clockHz
    // reload = (timeout_ms * clockHz) / (divider * 1000) - 1

    static uint16_t const dividers[] = {4, 8, 16, 32, 64, 128, 256};

    for (uint8_t code = 0U; code < 7U; code++)
    {
        uint32_t ticks = (timeoutMs * (clockHz / 1000U)) / dividers[code];
        if (ticks == 0U)
        {
            ticks = 1U;
        }
        if (ticks <= 4096U)
        {
            prescalerCode = code;
            reload        = static_cast<uint16_t>(ticks - 1U);
            return;
        }
    }

    // Largest possible: prescaler /256, reload 4095
    prescalerCode = 6U;
    reload        = 4095U;
}

void Watchdog::enableWatchdog(uint32_t timeout, bool /*interruptActive*/, uint32_t clockSpeed)
{
    uint8_t prescalerCode;
    uint16_t reload;
    computePrescalerAndReload(timeout, clockSpeed, prescalerCode, reload);

    // Unlock IWDG registers
    IWDG->KR = 0x5555U;

    // A write to PR/RLR is ignored while the corresponding update flag is
    // still set from a previous write, so wait (bounded) for the flag to
    // clear before each write. On timeout proceed anyway - the watchdog must
    // be started; checkWatchdogConfiguration() exposes a configuration that
    // did not take effect.
    uint32_t syncTimeout = SYNC_TIMEOUT_CYCLES;
    while (((IWDG->SR & IWDG_SR_PVU) != 0U) && (syncTimeout != 0U))
    {
        --syncTimeout;
    }
    IWDG->PR = prescalerCode;

    syncTimeout = SYNC_TIMEOUT_CYCLES;
    while (((IWDG->SR & IWDG_SR_RVU) != 0U) && (syncTimeout != 0U))
    {
        --syncTimeout;
    }
    IWDG->RLR = reload;

    // Start the watchdog (irreversible)
    IWDG->KR = 0xCCCCU;

    serviceWatchdog();
}

void Watchdog::disableWatchdog()
{
    // STM32 IWDG cannot be disabled once started.
    // This is intentionally a no-op.
}

void Watchdog::serviceWatchdog()
{
    IWDG->KR = 0xAAAAU;
    watchdogServiceCounter++;
}

bool Watchdog::checkWatchdogConfiguration(uint32_t timeout, uint32_t clockSpeed)
{
    uint8_t expectedPrescaler;
    uint16_t expectedReload;
    computePrescalerAndReload(timeout, clockSpeed, expectedPrescaler, expectedReload);

    uint32_t actualPrescaler = IWDG->PR & 0x7U;
    uint32_t actualReload    = IWDG->RLR & 0xFFFU;

    return (actualPrescaler == expectedPrescaler) && (actualReload == expectedReload);
}

bool Watchdog::isResetFromWatchdog() { return (RCC->CSR & RCC_CSR_IWDGRSTF) != 0U; }

void Watchdog::clearResetFlag()
{
    // Writing RMVF bit clears all reset flags
    RCC->CSR |= RCC_CSR_RMVF;
}

uint32_t Watchdog::getWatchdogServiceCounter() { return watchdogServiceCounter; }

} // namespace bsp
} // namespace safety
