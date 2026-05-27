/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
