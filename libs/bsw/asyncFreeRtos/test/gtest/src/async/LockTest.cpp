// Copyright 2024 Accenture.

#include "async/Lock.h"

#include "interrupts/SuspendResumeAllInterruptsMock.h"

namespace
{
using namespace ::async;
using namespace ::testing;

class LockTest : public Test
{
protected:
    StrictMock<::interrupts::SuspendResumeAllInterruptsMock> _suspendResumeAllInterruptsMock;
};

/**
 * \req: [BSW_ASFR_63]
 * \refs: SMD_asyncFreeRtos_Lock
 * \desc: To test the lock guard functionality
 */
TEST_F(LockTest, testLocksAndUnlocks)
{
    EXPECT_CALL(
        _suspendResumeAllInterruptsMock, getOldIntEnabledStatusValueAndSuspendAllInterrupts())
        .WillOnce(Return(12348943U));
    Lock const cut;
    Mock::VerifyAndClearExpectations(&_suspendResumeAllInterruptsMock);
    EXPECT_CALL(_suspendResumeAllInterruptsMock, resumeAllInterrupts(12348943U));
}

} // namespace
