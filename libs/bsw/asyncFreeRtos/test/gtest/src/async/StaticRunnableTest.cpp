// Copyright 2024 Accenture.

#include "async/StaticRunnable.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class TestRunnable : public StaticRunnable<TestRunnable>
{
public:
    MOCK_METHOD0(execute, void());
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
