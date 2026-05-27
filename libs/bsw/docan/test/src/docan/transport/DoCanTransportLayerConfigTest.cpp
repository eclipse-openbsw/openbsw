/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/transport/DoCanTransportLayerConfig.h"

#include "docan/datalink/DoCanDataLinkLayer.h"

#include <etl/delegate.h>

#include <gmock/gmock.h>

namespace
{
using namespace docan;

using DataLinkLayerType = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0U>;

uint32_t systemUs() { return 0; }

TEST(DoCanTransportLayerConfigTest, testConstructedParametersAndSetter)
{
    DoCanParameters parameters(
        ::etl::delegate<decltype(systemUs)>::create<&systemUs>(),
        23425,
        3543,
        1232,
        3442,
        98,
        153,
        750,
        122);
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 2U, 3U, 64U> cut(parameters);
    EXPECT_EQ(2U, cut.getMessageReceiverPool().max_size());
    EXPECT_EQ(3U, cut.getMessageTransmitterPool().max_size());
    EXPECT_EQ(
        sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayerType, 64U>),
        cut.getMessageReceiverPool().max_item_size());
    EXPECT_EQ(&parameters, &cut.getParameters());
}

} // anonymous namespace
