// Copyright 2025 Accenture.

#include "safeSupervisor/SafeSupervisor.h"

#include <gtest/gtest.h>

extern uint32_t lockIsrCounter;
extern uint32_t softwareSystemResetCalled;

namespace safety
{
class SafeSupervisorTest : public ::testing::Test
{
public:
    void SetUp()
    {
        lockIsrCounter            = 0U;
        softwareSystemResetCalled = 0U;
    }

    void checkReaction()
    {
        EXPECT_EQ(1U, lockIsrCounter);
        EXPECT_EQ(1U, softwareSystemResetCalled);
    }

    SafeSupervisor _supervisor;
};

TEST_F(SafeSupervisorTest, UnknownEvent)
{
    constexpr uint32_t unknown_event = 0x12449900U;
    _supervisor.handle(static_cast<safety::SafeSupervisor::Event>(unknown_event));
    checkReaction();
}

TEST_F(SafeSupervisorTest, MpuStatusOnTaskEnter)
{
    _supervisor.mpuStatusCheckOnEnterMonitor.check(false);
    checkReaction();
}

TEST_F(SafeSupervisorTest, mpuStatusCheckOnLeave)
{
    _supervisor.mpuStatusCheckOnLeaveMonitor.check(true);
    checkReaction();
}

TEST_F(SafeSupervisorTest, safeWatchdogConfig)
{
    _supervisor.safeWatchdogConfigMonitor.check(false);
    checkReaction();
}

TEST_F(SafeSupervisorTest, serviceWatchdog)
{
    _supervisor.serviceWatchdogMonitor.trigger();
    checkReaction();
}

TEST_F(SafeSupervisorTest, safeWatchdogSequence)
{
    _supervisor.safeWatchdogSequenceMonitor.hit(SafeSupervisor::EnterLeaveSequence::ENTER);
    _supervisor.safeWatchdogSequenceMonitor.hit(SafeSupervisor::EnterLeaveSequence::ENTER);
    _supervisor.safeWatchdogSequenceMonitor.hit(SafeSupervisor::EnterLeaveSequence::LEAVE);
    checkReaction();
}

TEST_F(SafeSupervisorTest, watchdogStartupCheck)
{
    _supervisor.watchdogStartupCheckMonitor.trigger();
    EXPECT_EQ(0U, softwareSystemResetCalled);
}

TEST_F(SafeSupervisorTest, safetyManagerSequence)
{
    _supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_ENTER);
    _supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_ENTER);
    _supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_LEAVE);
    checkReaction();
}

} // namespace safety
