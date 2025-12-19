// Copyright 2025 Accenture.

#include "doip/common/DoIpCyclicTaskGenerator.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

struct DoIpCyclicTaskGeneratorTest : Test
{
    DoIpCyclicTaskGeneratorTest() : asyncMock(), testContext(1U) {}

    void SetUp() override { testContext.setNow(1000U); }

    void TearDown() override { testContext.setNow(0U); }

    MOCK_METHOD0(cyclicTask, void());

    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::TestContext testContext;
};

TEST_F(DoIpCyclicTaskGeneratorTest, TestAll)
{
    DoIpCyclicTaskGenerator cut(
        DoIpCyclicTaskGenerator::CyclicTaskType::
            create<DoIpCyclicTaskGeneratorTest, &DoIpCyclicTaskGeneratorTest::cyclicTask>(*this),
        testContext,
        100U);
    testContext.handleSchedule();
    cut.start();

    testContext.expire();

    testContext.elapse(static_cast<uint64_t>(99U * 1000U));
    testContext.expire();

    testContext.elapse(1000U);
    EXPECT_CALL(*this, cyclicTask());
    testContext.expire();
    Mock::VerifyAndClearExpectations(this);

    testContext.elapse(static_cast<uint64_t>(80U * 1000U));
    testContext.expire();

    testContext.elapse(static_cast<uint64_t>(80U * 1000U));
    EXPECT_CALL(*this, cyclicTask());
    testContext.expire();
    Mock::VerifyAndClearExpectations(this);

    cut.shutdown();

    testContext.elapse(static_cast<uint64_t>(200U * 1000U));
    testContext.expire();
}

} // namespace test
} // namespace doip
