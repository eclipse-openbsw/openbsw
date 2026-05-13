// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   Watchdog.h
 * \brief  STM32 Independent Watchdog (IWDG) driver.
 *
 * Provides the same interface as the S32K Watchdog so SafeWatchdog and
 * SafetyManager can call enableWatchdog / serviceWatchdog / checkConfiguration
 * identically across platforms.
 *
 * STM32 IWDG specifics:
 *   - LSI clock ~32 kHz (not trimmed — 17..47 kHz depending on device)
 *   - 12-bit reload register (0..4095)
 *   - Once started, cannot be disabled (hardware limitation)
 *   - Kick sequence: write 0xAAAA to KR
 *   - Unlock sequence: write 0x5555 to KR, then configure PR + RLR
 *   - Start sequence: write 0xCCCC to KR
 */

#pragma once

#include <platform/estdint.h>

namespace safety
{
namespace bsp
{

class Watchdog
{
public:
    Watchdog() = default;

    explicit Watchdog(uint32_t const timeout, uint32_t const clockSpeed = DEFAULT_CLOCK_SPEED)
    {
        enableWatchdog(timeout, false, clockSpeed);
    }

    /**
     * \brief Enable the IWDG with the given timeout.
     * \param timeout      Timeout in milliseconds (max ~32 s at 32 kHz LSI).
     * \param interruptActive  Ignored on STM32 (IWDG has no interrupt mode).
     * \param clockSpeed   LSI clock speed in Hz (default 32000).
     */
    static void enableWatchdog(
        uint32_t timeout,
        bool interruptActive  = false,
        uint32_t clockSpeed   = DEFAULT_CLOCK_SPEED);

    /**
     * \brief Cannot disable the STM32 IWDG once started.  This is a no-op.
     */
    static void disableWatchdog();

    /**
     * \brief Kick (service) the IWDG by writing 0xAAAA to KR.
     */
    static void serviceWatchdog();

    /**
     * \brief Check that the IWDG prescaler and reload match the expected timeout.
     * \return true if the configuration is consistent.
     */
    static bool checkWatchdogConfiguration(
        uint32_t timeout, uint32_t clockSpeed = DEFAULT_CLOCK_SPEED);

    /**
     * \brief Check if the last reset was caused by the IWDG.
     * \return true if RCC_CSR_IWDGRSTF is set.
     */
    static bool isResetFromWatchdog();

    /**
     * \brief Clear the IWDG reset flag in RCC_CSR.
     */
    static void clearResetFlag();

    static uint32_t getWatchdogServiceCounter();

    static constexpr uint32_t DEFAULT_TIMEOUT     = 250U;  ///< 250 ms
    static constexpr uint32_t DEFAULT_CLOCK_SPEED = 32000U; ///< LSI ~32 kHz

private:
    static uint32_t watchdogServiceCounter;

    /// Compute prescaler divider and reload value for a given timeout.
    static void computePrescalerAndReload(
        uint32_t timeoutMs, uint32_t clockHz, uint8_t& prescalerCode, uint16_t& reload);
};

} // namespace bsp
} // namespace safety
