// Copyright 2024 Accenture.

#include "transport/FunctionalTransportMessageSendJob.h"

#include <estd/assert.h>
#include <platform/estdint.h>

#include <gmock/gmock.h>

#include <cstdlib>

using namespace transport;

using namespace ::testing;

namespace
{
/**
 * Tests for TransportJob
 */
class FunctionalTransportMessageSendJobTest : public ::testing::Test
{
public:
    void SetUp() {}

    void TearDown() {}

protected:
};

TEST_F(FunctionalTransportMessageSendJobTest, TestConstructor)
{
    transport::FunctionalTransportMessageSendJob job;
    ASSERT_EQ(0, job.getTransportMessageProcessedCounter());

    job.getTransportMessageSendJob().setTransportMessageProcessedListener(0L);
    job.getTransportMessageSendJob().setTransportMessage(0L);
    EXPECT_EQ(0L, job.getTransportMessageSendJob().getTransportMessage());
}

TEST_F(FunctionalTransportMessageSendJobTest, TestSetTransportMessageProcessedCounter)
{
    transport::FunctionalTransportMessageSendJob job;
    ASSERT_EQ(0, job.getTransportMessageProcessedCounter());
    job.setTransportMessageProcessedCounter(0x01);
    ASSERT_EQ(1, job.getTransportMessageProcessedCounter());
    job.setTransportMessageProcessedCounter(0xFF);
    ASSERT_EQ(255, job.getTransportMessageProcessedCounter());
}

TEST_F(FunctionalTransportMessageSendJobTest, TestDecreaseTransportMessageProcessedCounter)
{
    transport::FunctionalTransportMessageSendJob job;
    job.setTransportMessageProcessedCounter(0xFF);
    ASSERT_EQ(255, job.getTransportMessageProcessedCounter());
    uint8_t counter = 255;
    while (counter > 0)
    {
        job.decreaseTransportMessageProcessedCounter();
        --counter;
        ASSERT_EQ(counter, job.getTransportMessageProcessedCounter());
    }
    job.decreaseTransportMessageProcessedCounter();
    ASSERT_EQ(0, job.getTransportMessageProcessedCounter());
}

} // anonymous namespace
