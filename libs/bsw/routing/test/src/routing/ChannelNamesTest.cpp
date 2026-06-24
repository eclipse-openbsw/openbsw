/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/ChannelNames.h"

#include "blob/configuration.h"
#include "routing/Header.h"
#include "routing/constants.h"
#include "routing/util.h"

#include <blob/Blob.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

/**
 * \desc: Loading the channel names config from a test blob gives the correct values.
 */
TEST(ChannelNamesTest, channel_names_config)
{
    uint32_t const expectedOffsets[] = {0, 5, 13, 18, 33, 48, 63, 78, 93, 108, 123, 138, 160};
    char const* expectedChannelNames[]
        = {"CAN0",
           "FLEXRAY",
           "CAN1",
           "PDU_TRANSPORT0",
           "PDU_TRANSPORT1",
           "PDU_TRANSPORT2",
           "PDU_TRANSPORT3",
           "PDU_TRANSPORT4",
           "PDU_TRANSPORT5",
           "PDU_TRANSPORT6",
           "PDU_TRANSPORT7",
           "UNNAMED_PDU_TRANSPORT"};

    ::routing::ChannelNames channelNamesConfig;
    ASSERT_TRUE(::routing::load(
        ::routing::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::CHANNEL_NAMES).data,
        channelNamesConfig));

    for (size_t i = 0; i < ::routing::NUM_CHANNELS; ++i)
    {
        auto const size = channelNamesConfig.offsets[i + 1] - channelNamesConfig.offsets[i];
        auto const channelName
            = channelNamesConfig.names.subspan(channelNamesConfig.offsets[i], size);
        EXPECT_EQ(expectedOffsets[i], channelNamesConfig.offsets[i]);
        EXPECT_THAT(channelName, ElementsAreArray(expectedChannelNames[i], channelName.size()));
    }
}

/**
 * \desc: The name function returns the correct channel name.
 */
TEST(ChannelNamesTest, name)
{
    ::routing::ChannelNames config;
    ASSERT_TRUE(::routing::load(
        ::routing::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::CHANNEL_NAMES).data,
        config));

    EXPECT_TRUE(::routing::name(config, ::routing::NUM_CHANNELS).empty());

    for (size_t i = 0; i < ::routing::NUM_CHANNELS; ++i)
    {
        auto const channelName = ::routing::name(config, i);
        auto const expectedChannelName
            = config.names.subspan(config.offsets[i], config.offsets[i + 1] - config.offsets[i]);
        EXPECT_THAT(channelName, ElementsAreArray(expectedChannelName));
    }
}

/**
 * \desc: The name function returns an empty span if next offset is not greater than current
 * offset.
 */
TEST(ChannelNamesTest, name_decreasing_offset)
{
    constexpr uint32_t NUM_CHANNELS = 3;

    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C,
           0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x0F, 0x43, 0x41, 0x4E, 0x30,
           0x00, 0x43, 0x41, 0x4E, 0x32, 0x00, 0x43, 0x41, 0x4E, 0x31, 0x00, 0xFF, 0xFF, 0xFF};

    ::routing::ChannelNames config;
    ASSERT_TRUE(::routing::load(
        ::routing::config(::etl::make_span(data), ::blob::Config::Type::CHANNEL_NAMES).data,
        config));

    EXPECT_TRUE(::routing::name(config, NUM_CHANNELS).empty());

    // next offset is greater than current offset
    auto expectedChannelName
        = config.names.subspan(config.offsets[0], config.offsets[1] - config.offsets[0]);
    EXPECT_THAT(::routing::name(config, 0), ElementsAreArray(expectedChannelName));

    // next offset is not greater than current offset
    EXPECT_TRUE(::routing::name(config, 1).empty());
}

/**
 * \desc: Loading config from an empty blob doesn't crash.
 */
TEST(ChannelNamesTest, empty_blob)
{
    ::etl::span<uint8_t const> emptyData;

    ::routing::ChannelNames config;
    ASSERT_FALSE(::routing::load(
        ::routing::config(emptyData, ::blob::Config::Type::CHANNEL_NAMES).data, config));

    EXPECT_TRUE(::routing::name(config, 0).empty());
}

} // namespace
