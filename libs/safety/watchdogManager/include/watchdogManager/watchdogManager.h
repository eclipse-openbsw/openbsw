// Copyright 2024 Accenture.

#pragma once

namespace safety
{
class WatchdogManager
{
public:
    /**
     * Checks if the watchdog functions correctly and resets the ECU if the timeout occurs.
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
