/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <platform/estdint.h>

namespace safety
{
namespace bsp
{
class Watchdog
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Default constructor (disables the watchdog)
     */
    Watchdog() { disableWatchdog(); }

    /**
     * Constructor (starts the watchdog)
     * \param[in] timeout Watchdog timeout in milliseconds
     * \param[in] clockSpeed Speed of WDOG input clock in hertz
     */
    explicit Watchdog(uint32_t const timeout, uint32_t const clockSpeed = DEFAULT_CLOCK_SPEED)
    {
        enableWatchdog(timeout, false, clockSpeed);
    }

    /**
     * Enables the watchdog
     * \param[in] timeout Watchdog timeout in milliseconds
     * \param[in] interruptActive Unused on RP2040, the hardware watchdog always resets the chip
     * \param[in] clockSpeed Speed of WDOG input clock in hertz
     */
    static void enableWatchdog(
        uint32_t const timeout,
        bool const interruptActive = false,
        uint32_t const clockSpeed  = DEFAULT_CLOCK_SPEED);
    /**
     * Disables the watchdog
     */
    static void disableWatchdog();
    /**
     * Services the watchdog
     */
    static void serviceWatchdog();
    /**
     * Check that the configuration of the watchdog as set when enabled has not been changed
     * \param[in] timeout Watchdog timeout in milliseconds
     * \param[in] clockSpeed Speed of WDOG input clock in hertz
     */
    static bool checkWatchdogConfiguration(
        uint32_t const timeout, uint32_t const clockSpeed = DEFAULT_CLOCK_SPEED);
    /**
     * Start testing the low byte of the counter. Not supported by the RP2040 watchdog.
     */
    static void startFastTestLow();
    /**
     * Start testing the high byte of the counter. Not supported by the RP2040 watchdog.
     */
    static void startFastTestHigh();
    /**
     * Execute the WD fast test.
     *
     * The RP2040 watchdog has no counter self test, so there is nothing to execute and the
     * check counts as passed.
     *
     * \param timeout delay for the watchdog to timeout
     *
     * \return always true, as no check is needed for this cycle
     */
    static bool executeFastTest(uint32_t timeout);
    /**
     * After a successful fast test the WD can be set in user mode. Not supported by the RP2040
     * watchdog.
     */
    static void setUserMode();
    /**
     * Returns the number of times the watchdog has been serviced
     */
    static uint32_t getWatchdogServiceCounter();
    // [PUBLIC_API_END]
    /**
     * Default timeout in milliseconds
     */
    static uint32_t const DEFAULT_TIMEOUT     = 500U;
    /**
     * Default clock speed in hertz
     */
    static uint32_t const DEFAULT_CLOCK_SPEED = 128000U;

private:
    static uint32_t watchdogServiceCounter;
    static uint32_t watchdogTimeout;

    static bool isWdFastTestLow();
    static bool isWdFastTestHigh();
};

} // namespace bsp
} // namespace safety
