/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/StaticRunnable.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class TestRunnable : public StaticRunnable<TestRunnable>
{
public:
    MOCK_METHOD(void, execute, ());
};

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTaskImpl
 * \desc: To test static runnable functionality
 */
TEST(StaticRunnableTest, testAll)
{
    StrictMock<TestRunnable> runnableMock1;
    StrictMock<TestRunnable> runnableMock2;
    EXPECT_CALL(runnableMock1, execute());
    EXPECT_CALL(runnableMock2, execute());
    TestRunnable::run();
    Mock::VerifyAndClearExpectations(&runnableMock1);
    Mock::VerifyAndClearExpectations(&runnableMock2);

    TestRunnable::run();
    Mock::VerifyAndClearExpectations(&runnableMock1);
    Mock::VerifyAndClearExpectations(&runnableMock2);
}

} // namespace
