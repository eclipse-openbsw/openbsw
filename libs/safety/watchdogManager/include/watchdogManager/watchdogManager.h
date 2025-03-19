// Copyright 2024 Accenture.

#pragma once

namespace safety
{
class WatchdogManager
{
public:
    /**
     * Fast testing of the watchdog.
     *
     * Checks if the watchdog functions correctly. It tests the watchdog more quickly by splitting
     * the watchdog counter into its constituent byte-wide stages. The low and high bytes of counter
     * are run independently and tested for timeout against the corresponding byte of the timeout
     * value register. During the test, the watchdog reset occurs twice, once for the fast testing
     * of low byte and again for the fast testing of high byte of the watchdog counter.
     *
     * \return
     * - false = the check was started, but it was unsuccessful
     * - true = the check executed successfully since the last power-on reset, no further check is
     *          needed for this cycle
     * - does not return = the check was started and watchdog has reset the ECU as expected
     */
    static bool startTest();

private:
    static bool isWDFastTestLow();
    static bool isWDFastTestHigh();
};

} // namespace safety
