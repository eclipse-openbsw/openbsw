/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/StatisticsEvaluator.h"

#include "someip/NetworkMock.h"
#include "someip/QueryManager.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/TcpClientChannelValidator.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;

struct StatisticsEvaluatorTest : public ::testing::Test
{
    StatisticsEvaluatorTest()
    : evaluator(_ethernetContext, serviceRegistry, queryManager)
    , validator(network, false)
    , queryManager(validator)
    , _asyncMock()
    , _testContext(_ethernetContext)
    {
        _testContext.handleAll();
        Statistics::reset();

        EXPECT_CALL(_asyncMock, scheduleAtFixedRate(_ethernetContext, _, _, _, _));
        evaluator.init();
    }

    static uint8_t const MAX_SERVICES = 2U;

    void tick() { evaluator.cyclic(); }

    void commonChecks()
    {
        EXPECT_CALL(serviceRegistry, getCurrentNumberOfSubscriptions()).Times(1);
        EXPECT_CALL(serviceRegistry, getMaximumNumberOfSubscriptions()).Times(1);
        EXPECT_CALL(serviceRegistry, getCurrentNumberOfProvidedServices()).Times(1);
        EXPECT_CALL(serviceRegistry, getMaximumNumberOfProvidedServices()).Times(1);
        EXPECT_CALL(serviceRegistry, getCurrentNumberOfRemoteServices()).Times(1);
        EXPECT_CALL(serviceRegistry, getMaximumNumberOfRemoteServices()).Times(1);
    }

    async::ContextType _ethernetContext{0U};
    StrictMock<ServiceRegistryMock> serviceRegistry;
    StatisticsEvaluator evaluator;
    StrictMock<NetworkMock> network;
    TcpClientChannelValidator validator;
    ::declare::QueryManager<MAX_SERVICES> queryManager;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

/**
 * Make sure initializing and reseting evaluator set counter to 0.
 */
TEST_F(StatisticsEvaluatorTest, test_init_and_reset)
{
    Statistics::incCounter(Statistics::Counter::PDU_RX);
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));

    evaluator.reset();
    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::PDU_RX));

    // EXPECT_CALL(timeoutManagerMock, cancel(_)).Times(1);
    evaluator.shutdown();
}

TEST_F(StatisticsEvaluatorTest, TestNumReceivedMessagesPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumReceivedMessagesPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::PDU_RX);
    tick();
    EXPECT_EQ(1U, evaluator.getNumReceivedMessagesPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentMessagesPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumSentMessagesPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::PDU_TX);
    tick();
    EXPECT_EQ(1U, evaluator.getNumSentMessagesPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumReceivedSdMessagesPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumReceivedSdMessagesPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::SD_FRAME_RX);
    tick();
    EXPECT_EQ(1U, evaluator.getNumReceivedSdMessagesPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentSdMessagesPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumSentSdMessagesPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::SD_FRAME_TX);
    tick();
    EXPECT_EQ(1U, evaluator.getNumSentSdMessagesPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentTrainsPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumSentTrainsPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::TRAIN_FULL);
    tick();
    EXPECT_EQ(1U, evaluator.getNumSentTrainsPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentTrainsFullPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumTrainsSentBecauseFullPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::TRAIN_FULL);
    tick();
    EXPECT_EQ(1U, evaluator.getNumTrainsSentBecauseFullPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentTrainsNoFreePerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumTrainsSentBecauseNoFreeTrainPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::TRAIN_NOT_AVAILABLE);
    tick();
    EXPECT_EQ(1U, evaluator.getNumTrainsSentBecauseNoFreeTrainPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentTrainsDuplicatePerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumTrainsSentBecauseDuplicatePduPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::TRAIN_DUPLICATE_PDU);
    tick();
    EXPECT_EQ(1U, evaluator.getNumTrainsSentBecauseDuplicatePduPerSecond());
}

TEST_F(StatisticsEvaluatorTest, TestNumSentTrainsTimeoutPerSecond)
{
    EXPECT_EQ(0U, evaluator.getNumTrainsSentBecauseTimeoutPerSecond());

    commonChecks();

    Statistics::incCounter(Statistics::Counter::TRAIN_TIMEOUT);
    tick();
    EXPECT_EQ(1U, evaluator.getNumTrainsSentBecauseTimeoutPerSecond());
}
} // anonymous namespace
