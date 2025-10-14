// Copyright 2024 Accenture.

#include "interrupts/SuspendResumeAllInterruptsLock.h"
#include "interrupts/SuspendResumeAllInterruptsMock.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace ::testing;

namespace
{

class InterruptTest : public testing::Test
{
public:
    InterruptTest() {}

protected:
    StrictMock<::interrupts::SuspendResumeAllInterruptsMock> _interruptsMock;
};

/**
 * \refs: SMD_bspInterrupts_SuspendResumeAllInterruptsLock,
 * SMD_bspInterrupts_SuspendResumeAllInterruptsLock::suspend,
 * SMD_bspInterrupts_SuspendResumeAllInterruptsLock::resume
 */
TEST_F(InterruptTest, SuspendResumeAllInterruptsLock)
{
    EXPECT_CALL(_interruptsMock, getMachineStateRegisterValueAndSuspendAllInterrupts())
        .Times(1U)
        .WillOnce(Return(0x42));
    EXPECT_CALL(_interruptsMock, resumeAllInterrupts(0x42)).Times(1U);

    ::interrupts::SuspendResumeAllInterruptsLock lock;
    lock.suspend();
    lock.resume();
}

/**
 * \refs: SMD_bspInterrupts_SuspendResumeAllInterruptsScopedLock,
 * SMD_bspInterrupts_SuspendResumeAllInterruptsScopedLock::SuspendResumeAllInterruptsScopedLock,
 * SMD_bspInterrupts_SuspendResumeAllInterruptsScopedLock~SuspendResumeAllInterruptsScopedLock
 */
TEST_F(InterruptTest, SuspendResumeAllInterruptsScopedLock)
{
    EXPECT_CALL(_interruptsMock, getMachineStateRegisterValueAndSuspendAllInterrupts())
        .Times(1U)
        .WillOnce(Return(0x12345678));
    EXPECT_CALL(_interruptsMock, resumeAllInterrupts(0x12345678)).Times(1U);

    {
        ::interrupts::SuspendResumeAllInterruptsScopedLock lock;
    }
}

} // anonymous namespace
