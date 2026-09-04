/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/Statistics.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

TEST(Statistics, MessageReceivedTest)
{
    Statistics::reset();

    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::PDU_RX));
    Statistics::incCounter(Statistics::Counter::PDU_RX);
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST(Statistics, MessageSentTest)
{
    Statistics::reset();

    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::PDU_TX));
    Statistics::incCounter(Statistics::Counter::PDU_TX);
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_TX));
}

TEST(Statistics, SdMessageReceivedTest)
{
    Statistics::reset();

    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::SD_FRAME_RX));
    Statistics::incCounter(Statistics::Counter::SD_FRAME_RX);
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::SD_FRAME_RX));
}

TEST(Statistics, SdMessageSentTest)
{
    Statistics::reset();

    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::SD_FRAME_TX));
    Statistics::incCounter(Statistics::Counter::SD_FRAME_TX);
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::SD_FRAME_TX));
}

TEST(Statistics, TrainSent)
{
    Statistics::reset();

    EXPECT_EQ(0U, Statistics::getNumSentTrains());
    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::TRAIN_FULL));
    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::TRAIN_NOT_AVAILABLE));
    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::TRAIN_DUPLICATE_PDU));
    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::TRAIN_TIMEOUT));

    Statistics::incCounter(Statistics::Counter::TRAIN_FULL);
    EXPECT_EQ(1U, Statistics::getNumSentTrains());
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::TRAIN_FULL));

    Statistics::incCounter(Statistics::Counter::TRAIN_NOT_AVAILABLE);
    EXPECT_EQ(2U, Statistics::getNumSentTrains());
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::TRAIN_NOT_AVAILABLE));

    Statistics::incCounter(Statistics::Counter::TRAIN_DUPLICATE_PDU);
    EXPECT_EQ(3U, Statistics::getNumSentTrains());
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::TRAIN_DUPLICATE_PDU));

    Statistics::incCounter(Statistics::Counter::TRAIN_TIMEOUT);
    EXPECT_EQ(4U, Statistics::getNumSentTrains());
    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::TRAIN_TIMEOUT));
}

} // anonymous namespace
