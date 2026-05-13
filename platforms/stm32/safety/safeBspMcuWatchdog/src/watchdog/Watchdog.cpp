// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include <watchdog/Watchdog.h>

#include <mcu/mcu.h>

namespace safety
{
namespace bsp
{

uint32_t Watchdog::watchdogServiceCounter = 0U;

void Watchdog::computePrescalerAndReload(
    uint32_t timeoutMs, uint32_t clockHz, uint8_t& prescalerCode, uint16_t& reload)
{
    // IWDG prescaler dividers: /4, /8, /16, /32, /64, /128, /256
    // prescalerCode 0..6 → divider = 4 << code
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

    // Set prescaler
    IWDG->PR = prescalerCode;

    // Set reload value
    IWDG->RLR = reload;

    // Wait for registers to be updated
    while ((IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) != 0U)
    {
    }

    // Start the watchdog (irreversible)
    IWDG->KR = 0xCCCCU;

    // Initial kick
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

bool Watchdog::isResetFromWatchdog()
{
#if defined(STM32F413xx)
    return (RCC->CSR & RCC_CSR_IWDGRSTF) != 0U;
#elif defined(STM32G474xx)
    return (RCC->CSR & RCC_CSR_IWDGRSTF) != 0U;
#else
    return false;
#endif
}

void Watchdog::clearResetFlag()
{
    // Writing RMVF bit clears all reset flags
    RCC->CSR |= RCC_CSR_RMVF;
}

uint32_t Watchdog::getWatchdogServiceCounter() { return watchdogServiceCounter; }

} // namespace bsp
} // namespace safety
