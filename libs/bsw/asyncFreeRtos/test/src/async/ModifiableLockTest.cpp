// Copyright 2024 Accenture.

#include "async/ModifiableLock.h"

#include "interrupts/SuspendResumeAllInterruptsMock.h"

namespace
{
using namespace ::async;
using namespace ::testing;

class ModifiableLockTest : public Test
{
protected:
    StrictMock<::interrupts::SuspendResumeAllInterruptsMock> _suspendResumeAllInterruptsMock;
};

/**
 * \req: [BSW_ASFR_63]
 * \refs: SMD_asyncFreeRtos_ModifiableLock
 * \desc: To test the lock guard functionality
 */
TEST_F(ModifiableLockTest, testConstructorAndDestructor)
{
    EXPECT_CALL(
        _suspendResumeAllInterruptsMock, getOldIntEnabledStatusValueAndSuspendAllInterrupts())
        .WillOnce(Return(12348943U));
    ModifiableLock const cut;
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    EXPECT_CALL(_suspendResumeAllInterruptsMock, resumeAllInterrupts(12348943U));
}

/**
 * \req: [BSW_ASFR_63]
 * \refs: SMD_asyncFreeRtos_ModifiableLock
 * \desc: To test the unlock functionality
 */
TEST_F(ModifiableLockTest, testDestructorIsNotUnlockingIfUnlocked)
{
    EXPECT_CALL(
        _suspendResumeAllInterruptsMock, getOldIntEnabledStatusValueAndSuspendAllInterrupts())
        .WillOnce(Return(12348943U));
    ModifiableLock cut;
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    EXPECT_CALL(_suspendResumeAllInterruptsMock, resumeAllInterrupts(12348943U));
    cut.unlock();
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
}

/**
 * \req: [BSW_ASFR_63]
 * \refs: SMD_asyncFreeRtos_ModifiableLock
 * \desc: To test unlock and lock functionality of lock guard
 */
TEST_F(ModifiableLockTest, testDestructorIsUnlockingIfUnlockedAndLockedAgain)
{
    EXPECT_CALL(
        _suspendResumeAllInterruptsMock, getOldIntEnabledStatusValueAndSuspendAllInterrupts())
        .WillOnce(Return(12348943U));
    ModifiableLock cut;
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    // lock again is neutral
    cut.lock();
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    // unlock should cause a call
    EXPECT_CALL(_suspendResumeAllInterruptsMock, resumeAllInterrupts(12348943U));
    cut.unlock();
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    // unlocking again is neutral
    cut.unlock();
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    // lock should cause a call
    EXPECT_CALL(
        _suspendResumeAllInterruptsMock, getOldIntEnabledStatusValueAndSuspendAllInterrupts())
        .WillOnce(Return(12343343U));
    cut.lock();
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    EXPECT_CALL(_suspendResumeAllInterruptsMock, resumeAllInterrupts(12343343U));
}

} // namespace
