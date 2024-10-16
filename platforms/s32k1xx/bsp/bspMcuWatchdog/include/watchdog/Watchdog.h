// Copyright 2024 Accenture.

#ifndef GUARD_6C6854A5_7D1A_4588_8349_C9D0A81543A6
#define GUARD_6C6854A5_7D1A_4588_8349_C9D0A81543A6

#include <platform/estdint.h>

namespace bsp
{
class Watchdog
{
public:
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
        enableWatchdog(timeout, clockSpeed);
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
     * Start testing the low byte of the counter. If the test is successfull the WD will trigger an
     * Ecu Reset
     */
    static void startFastTestLow();
    /**
     * Start testing the high byte of the counter. If the test is successfull the WD will trigger an
     * Ecu Reset
     */
    static void startFastTestHigh();
    /**
     * After a successfull fast test the WD can be set in user mode where CS[TST]==01
     */
    static void setUserMode();
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
};

} // namespace bsp

#endif /* GUARD_6C6854A5_7D1A_4588_8349_C9D0A81543A6 */
