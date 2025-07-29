// Copyright 2024 Accenture.

#pragma once

#include <etl/delegate.h>

#include <platform/estdint.h>

namespace bios
{
class IWakeupEventListener;

/**
 * This interfaces implies the following modes:
 *  - PreSleep:  startPreSleep() has been called but no call to
 *               fullPowerUp() or powerDown() yet
 *  - Sleep:     powerDown() has been called but not yet returned
 *  - PostSleep: powerDown() has returned but no call to fullPowerUp() yet
 *  - No Sleep:  either startPreSleep()/powerDown() has not been called
 *               or it has been canceled with fullPowerUp()
 *
 * Wakeup Reason Detection
 *
 * In mode "Sleep" all relevant changes at the wakeup inputs must be detected.
 * (i.e. no relevant change/edge must be lost)
 *
 * All wakeup reasons are recorded in a wakeup reason bit vector.
 * The wakeup reason vector is an "or" of all HW wakeup reasons which occur while in sleep.
 *
 * The reasons in the vector are cleared when powerDown() is called.
 *
 * The current wakeup reason vector is passed as argument to CheckWakeupDelegate (see below).
 *
 */
class IEcuPowerStateController
{
public:
    using tCheckWakeupDelegate = ::etl::delegate<bool(uint32_t)>;

    /**
     * Enter into mode "pre sleep".
     * This is a chance to switch off some power consumers (go to reduced power mode).
     */
    virtual void startPreSleep() = 0;

    enum PowerMode
    {
        POWER_SHUTDOWN = 0, // Shutdown power, don't restart
        POWER_RESTART  = 1  // Shutdown and restart
    };

    /**
     * Enter into mode "sleep".
     * During sleep the check wakeup delegate will be called on every HW wakeup event (cyclic and
     * non-cyclic) returns when a call to the wakeup delegate returned "true".
     *
     * \param mode        see enum PowerMode
     * \param delegate    optional, will be called on all wakeup events during sleep
     *                      the current HW wakeup reason vector will be passed as parameter;
     *                      in order to trigger a wakeup it must return "true"
     *
     * \return HW wakeup reason vector (an "or" of all HW wakeup reasons which occurred during
     * sleep)
     */
    virtual uint32_t powerDown(uint8_t mode, tCheckWakeupDelegate delegate) = 0;
    virtual uint32_t powerDown(uint8_t mode)                                = 0;

    /**
     * When powerDown() returns, the ECU might still be in a reduced power mode ("post sleep"),
     * this call is used to switch to full power mode.
     */
    virtual void fullPowerUp() = 0;

    /**
     * New Interface for  void setMonitorWakeUp(uint32_t wupLine, bool active);
     */
    virtual void setWakeupSourceMonitoring(uint32_t source, bool active, bool fallingEdge) = 0;
    /**
     * New Interface for  void setMonitorWakeUp(uint32_t wupLine, bool active);
     */
    virtual void clearWakeupSourceMonitoring(uint32_t source)                              = 0;

    virtual bool setWakeupDelegate(tCheckWakeupDelegate& delegate) = 0;
    /**
     * Get HW WUP lowLevel WUP.
     */
    virtual uint32_t getWakeupSource(void)                         = 0;

protected:
    IEcuPowerStateController& operator=(IEcuPowerStateController const&) = default;
};

} // namespace bios
