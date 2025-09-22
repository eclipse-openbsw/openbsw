// Copyright 2024 Accenture.

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
     * \param[in] interruptActive Activates the Watchdog interrupt WDOG_EWM_IRQn
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
     * Check that the configuration of the watchdog as set when enables has not been changed
     * \param[in] timeout Watchdog timeout in milliseconds
     * \param[in] clockSpeed Speed of WDOG input clock in hertz
     */
    static bool checkWatchdogConfiguration(
        uint32_t const timeout, uint32_t const clockSpeed = DEFAULT_CLOCK_SPEED);
    /**
     * Start testing the low byte of the counter. If the test is successful the WD will trigger an
     * Ecu Reset
     */
    static void startFastTestLow();
    /**
     * Start testing the high byte of the counter. If the test is successful the WD will trigger an
     * Ecu Reset
     */
    static void startFastTestHigh();
    /**
     * Execute the WD fast test. On first call this will trigger the test for the low byte, then on
     * second call it will trigger the test for the high byte of the counter.
     * Afterwards it should always return true, indicating that the WD test was successful.
     *
     * \param timeout delay for the watchdog to timeout
     *
     * \return
     * - false = the check was started, but it was unsuccessful
     * - true = the check executed successfully since the last power-on reset, no further check is
     *          needed for this cycle
     * - does not return = the check was started and watchdog has reset the ECU as expected
     */
    static bool executeFastTest(uint32_t timeout);
    /**
     * After a successful fast test the WD can be set in user mode where CS[TST]==01
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
    static uint32_t const WATCHDOG_DISABLE = 0x00002924U;
    static uint32_t watchdogServiceCounter;

    static bool isWdFastTestLow();
    static bool isWdFastTestHigh();
};

} // namespace bsp
} // namespace safety
