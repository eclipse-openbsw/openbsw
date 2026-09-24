/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "watchdog/Watchdog.h"

#include <etl/platform.h>
#include <hardware/watchdog.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <mcu/mcu.h>

namespace safety
{
namespace bsp
{
uint32_t Watchdog::watchdogServiceCounter = 0U;
uint32_t Watchdog::watchdogTimeout        = 0U;

void Watchdog::enableWatchdog(
    uint32_t const timeout, bool const interruptActive, uint32_t const clockSpeed)
{
    // The RP2040 watchdog always resets the chip, it cannot raise an interrupt instead, and it is
    // clocked from the fixed 1 us tick rather than a configurable input clock.
    (void)interruptActive;
    (void)clockSpeed;

    ETL_MAYBE_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;
    watchdogTimeout = timeout;
    watchdog_enable(timeout, true);
}

void Watchdog::disableWatchdog()
{
    ETL_MAYBE_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;
    watchdogTimeout = 0U;
    watchdog_disable();
}

void Watchdog::serviceWatchdog()
{
    ETL_MAYBE_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;
    ++watchdogServiceCounter;
    watchdog_update();
}

bool Watchdog::checkWatchdogConfiguration(uint32_t const timeout, uint32_t const clockSpeed)
{
    (void)clockSpeed;
    if (watchdogTimeout != timeout)
    {
        return false;
    }
    // A remaining time larger than the configured timeout means the watchdog was reconfigured
    // behind our back.
    return watchdog_get_time_remaining_ms() <= timeout;
}

void Watchdog::startFastTestLow() {}

void Watchdog::startFastTestHigh() {}

bool Watchdog::isWdFastTestLow() { return false; }

bool Watchdog::isWdFastTestHigh() { return false; }

bool Watchdog::executeFastTest(uint32_t const timeout)
{
    // The RP2040 watchdog provides no counter self test, so there is nothing to run here.
    (void)timeout;
    return true;
}

void Watchdog::setUserMode() {}

uint32_t Watchdog::getWatchdogServiceCounter() { return watchdogServiceCounter; }

} // namespace bsp
} // namespace safety
