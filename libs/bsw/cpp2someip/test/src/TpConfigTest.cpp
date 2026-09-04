/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpConfig.h"

#include "someip/TpReceiver.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::someip;

struct TpConfigTest : ::testing::Test
{
    static uint8_t const NO_TP_STREAMS  = 0U;
    static uint8_t const TWO_TP_STREAMS = 2U;
    static size_t const BUFFER_SIZE     = 1500U;
};

/**
 * Test construction of TpConfig with TP enabled.
 */
TEST_F(TpConfigTest, construction_with_TP)
{
    ::someip::internal::TpResources<TWO_TP_STREAMS, BUFFER_SIZE> _resources;
    TpConfig& config = _resources;
    EXPECT_EQ(1U, config.tpSenders.size());
    EXPECT_EQ(2U, config.tpReceivers.size());

    for (auto const& receiver : config.tpReceivers)
    {
        EXPECT_FALSE(receiver->isActive());
    }
}

/**
 * Test construction of TpConfig with TP disabled.
 */
TEST_F(TpConfigTest, construction_with_TP_disabled)
{
    ::someip::internal::TpResources<NO_TP_STREAMS, BUFFER_SIZE> _resourcesTpDisabled;
    TpConfig& config = _resourcesTpDisabled;
    EXPECT_EQ(0U, config.tpSenders.size());
    EXPECT_EQ(0U, config.tpReceivers.size());
}

} // anonymous namespace
