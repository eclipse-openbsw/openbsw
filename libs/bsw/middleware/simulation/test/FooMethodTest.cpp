/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "foo/FooProxyWrapper.h"
#include "foo/FooSkeletonWrapper.h"
#include "middleware/ClusterCluster0.h"
#include "middleware/ClusterCluster1.h"
#include "middleware/shm/Config.h"
#include "middleware/time/SystemTimerProvider.h"

#include <gtest/gtest.h>

#include <limits>
#include <new>
#include <type_traits>

namespace
{
uint32_t currentTimeMs{};
}

namespace middleware::time
{
uint32_t getCurrentTimeInMs() { return currentTimeMs; }

uint32_t getCurrentTimeInUs() { return currentTimeMs * 1000U; }
} // namespace middleware::time

namespace middleware::shm
{
MemoryLayout& getMemoryLayout()
{
    static MemoryLayout layout;
    return layout;
}
} // namespace middleware::shm

namespace
{
using Consumer = FooProxyWrapper<middleware::core::ClusterId::Cluster1>;
using State    = Consumer::FooMethodState;
using middleware::core::HRESULT;

static_assert(std::is_same_v<Consumer::FooMethodResult, etl::array<uint32_t, 4>>);

class FooMethodTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& layout = middleware::shm::getMemoryLayout();
        layout.~MemoryLayout();
        new (&layout) middleware::shm::MemoryLayout();
        currentTimeMs = 0U;
        middleware::initializeCluster0ClusterConnection();
        middleware::initializeCluster1ClusterConnection();
        provider.init();
        consumer.init();
    }

    void TearDown() override
    {
        consumer.deInit();
        provider.deInit();
    }

    void roundTrip()
    {
        middleware::processCluster0Cluster(0);
        middleware::processCluster1Cluster(0);
    }

    void expectResult(Consumer::FooMethodResult const& expected)
    {
        auto const result = consumer.pollFooMethod();
        EXPECT_EQ(result.state, State::Ready);
        EXPECT_EQ(result.value, expected);
        EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    }

    FooSkeletonWrapper provider;
    Consumer consumer;
};

TEST_F(FooMethodTest, IdleAndPendingPollingDoNotConsumeRequest)
{
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    ASSERT_EQ(consumer.requestFooMethod(42U), HRESULT::Ok);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Pending);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Pending);
    roundTrip();
    expectResult({42U, 43U, 44U, 45U});
}

TEST_F(FooMethodTest, ReturnsExactlyFourValuesIncludingModuloOverflow)
{
    constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
    Consumer::FooMethodResult const inputs{0U, 42U, max - 2U, max};
    etl::array<Consumer::FooMethodResult, 4> const expected{{
        {0U, 1U, 2U, 3U},
        {42U, 43U, 44U, 45U},
        {max - 2U, max - 1U, max, 0U},
        {max, 0U, 1U, 2U},
    }};

    for (size_t i = 0U; i < inputs.size(); ++i)
    {
        ASSERT_EQ(consumer.requestFooMethod(inputs[i]), HRESULT::Ok);
        roundTrip();
        expectResult(expected[i]);
    }
}

TEST_F(FooMethodTest, BusyUntilCompletedResultIsConsumed)
{
    ASSERT_EQ(consumer.requestFooMethod(1U), HRESULT::Ok);
    EXPECT_EQ(consumer.requestFooMethod(9U), HRESULT::ServiceBusy);
    roundTrip();
    EXPECT_EQ(consumer.requestFooMethod(9U), HRESULT::ServiceBusy);
    expectResult({1U, 2U, 3U, 4U});

    ASSERT_EQ(consumer.requestFooMethod(9U), HRESULT::Ok);
    roundTrip();
    expectResult({9U, 10U, 11U, 12U});
}

TEST_F(FooMethodTest, ResultSurvivesReuseOfResponseQueue)
{
    ASSERT_EQ(consumer.requestFooMethod(123U), HRESULT::Ok);
    roundTrip();
    for (size_t i = 0U; i < 20U; ++i)
    {
        provider.sendBroadcast();
        consumer.requestGet();
        roundTrip();
    }
    expectResult({123U, 124U, 125U, 126U});
}

TEST_F(FooMethodTest, UnregisteredSendFailsImmediatelyWithoutPendingState)
{
    consumer.deInit();
    EXPECT_EQ(consumer.requestFooMethod(1U), HRESULT::NotRegistered);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    consumer.init();
    ASSERT_EQ(consumer.requestFooMethod(2U), HRESULT::Ok);
    roundTrip();
    expectResult({2U, 3U, 4U, 5U});
}

TEST_F(FooMethodTest, FullRequestQueueFailsImmediatelyAndAllowsNextCall)
{
    consumer.requestGet();
    // Getter has a one-request limit; fill the remaining slots with copies
    // of its inline-payload request instead of adding production test switches.
    auto& queue = *middleware::shm::getQueueToCluster0();
    using Queue = etl::remove_reference_t<decltype(queue)>;
    Queue::Receiver receiver(queue);
    Queue::Sender sender(queue);
    queue.takeSnapshot();
    auto const request = receiver.peek();
    while (!sender.isFull())
    {
        ASSERT_TRUE(sender.write(request));
    }

    EXPECT_EQ(consumer.requestFooMethod(1U), HRESULT::QueueFull);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    roundTrip();
    ASSERT_EQ(consumer.requestFooMethod(2U), HRESULT::Ok);
    roundTrip();
    expectResult({2U, 3U, 4U, 5U});
}

TEST_F(FooMethodTest, TimeoutIsPolledOnceAndLateResponseDoesNotCompleteNextCall)
{
    ASSERT_EQ(consumer.requestFooMethod(1U), HRESULT::Ok);
    currentTimeMs = 4999U;
    middleware::updateCluster1ClusterTimeouts();
    EXPECT_EQ(consumer.pollFooMethod().state, State::Pending);
    currentTimeMs = 5000U;
    middleware::updateCluster1ClusterTimeouts();
    EXPECT_EQ(consumer.pollFooMethod().state, State::Pending);
    currentTimeMs = 5001U;
    middleware::updateCluster1ClusterTimeouts();
    EXPECT_EQ(consumer.requestFooMethod(9U), HRESULT::ServiceBusy);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Timeout);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);

    ASSERT_EQ(consumer.requestFooMethod(9U), HRESULT::Ok);
    roundTrip();
    expectResult({9U, 10U, 11U, 12U});
}

TEST_F(FooMethodTest, ResponseErrorIsPolledOnceAndAllowsRecovery)
{
    ASSERT_EQ(consumer.requestFooMethod(1U), HRESULT::Ok);
    auto& requestQueue = *middleware::shm::getQueueToCluster0();
    using RequestQueue = etl::remove_reference_t<decltype(requestQueue)>;
    RequestQueue::Receiver receiver(requestQueue);
    requestQueue.takeSnapshot();
    ASSERT_FALSE(receiver.isEmpty());
    auto const header = receiver.peek().getHeader();
    receiver.advance();

    auto const error = middleware::core::Message::createErrorResponse(
        header.serviceId,
        header.memberId,
        header.requestId,
        header.serviceInstanceId,
        header.tgtClusterId,
        header.srcClusterId,
        header.addressId,
        middleware::core::ErrorState::UserDefinedError);
    auto& responseQueue = *middleware::shm::getQueueToCluster1();
    using ResponseQueue = etl::remove_reference_t<decltype(responseQueue)>;
    ResponseQueue::Sender sender(responseQueue);
    ASSERT_TRUE(sender.write(error));
    middleware::processCluster1Cluster(0);

    EXPECT_EQ(consumer.requestFooMethod(9U), HRESULT::ServiceBusy);
    EXPECT_EQ(consumer.pollFooMethod().state, State::UserError);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    ASSERT_EQ(consumer.requestFooMethod(9U), HRESULT::Ok);
    roundTrip();
    expectResult({9U, 10U, 11U, 12U});
}

TEST_F(FooMethodTest, MissingProviderErrorReachesConsumer)
{
    provider.deInit();
    ASSERT_EQ(consumer.requestFooMethod(1U), HRESULT::Ok);
    roundTrip();
    EXPECT_EQ(consumer.pollFooMethod().state, State::ServiceNotFound);
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    provider.init();
    ASSERT_EQ(consumer.requestFooMethod(9U), HRESULT::Ok);
    roundTrip();
    expectResult({9U, 10U, 11U, 12U});
}

TEST_F(FooMethodTest, DeinitializationDropsPendingRequest)
{
    ASSERT_EQ(consumer.requestFooMethod(1U), HRESULT::Ok);
    consumer.deInit();
    roundTrip();
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
    consumer.init();
    ASSERT_EQ(consumer.requestFooMethod(9U), HRESULT::Ok);
    roundTrip();
    expectResult({9U, 10U, 11U, 12U});
}

TEST_F(FooMethodTest, DeinitializationDropsUnconsumedResult)
{
    ASSERT_EQ(consumer.requestFooMethod(1U), HRESULT::Ok);
    roundTrip();
    consumer.deInit();
    EXPECT_EQ(consumer.pollFooMethod().state, State::Invalid);
}
} // namespace
