// Copyright 2024 Accenture.

#include "bsp/timer/SystemTimerMock.h"

#include "bsp/timer/SystemTimer.h"

using ::testing::SystemTimerMock;

extern "C"
{
void sysDelayUs(uint32_t delay) { SystemTimerMock::instance().sysDelayUs(delay); }

uint64_t getSystemTicks(void) { return SystemTimerMock::instance().getSystemTicks(); }

uint32_t getSystemTimeUs32Bit(void) { return SystemTimerMock::instance().getSystemTimeUs32Bit(); }

uint32_t getSystemTimeMs32Bit(void) { return SystemTimerMock::instance().getSystemTimeMs32Bit(); }

uint32_t getSystemTicks32Bit(void) { return SystemTimerMock::instance().getSystemTicks32Bit(); }

uint64_t getSystemTimeNs(void) { return SystemTimerMock::instance().getSystemTimeNs(); }

uint64_t getSystemTimeUs(void) { return SystemTimerMock::instance().getSystemTimeUs(); }

uint64_t getSystemTimeMs(void) { return SystemTimerMock::instance().getSystemTimeMs(); }

uint64_t systemTicksToTimeUs(uint64_t ticks)
{
    return SystemTimerMock::instance().systemTicksToTimeUs(ticks);
}

uint64_t systemTicksToTimeNs(uint64_t ticks)
{
    return SystemTimerMock::instance().systemTicksToTimeNs(ticks);
}

void initSystemTimer() { SystemTimerMock::instance().initSystemTimer(); }
}
