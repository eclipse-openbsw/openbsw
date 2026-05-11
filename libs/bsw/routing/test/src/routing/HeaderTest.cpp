/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/Header.h"

#include "blob/configuration.h"
#include "routing/constants.h"
#include "routing/util.h"

#include <blob/Blob.h>
#include <blob/Config.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

/**
 * \desc: Loading the header of a routing config from a test blob gives the correct values.
 */
TEST(HeaderTest, routing_config)
{
    auto const config
        = ::routing::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::ROUTING);

    EXPECT_EQ(::blob::Config::Type::ROUTING, config.type);
    auto data                      = config.data;
    ::routing::Header const header = ::routing::load(data);
    EXPECT_EQ(::routing::ChannelType::UNKNOWN, header.channelType);
    EXPECT_EQ(::routing::INVALID_CHANNEL_ID, header.channelId);
}

/**
 * \desc: Loading the header of a channel names config from a test blob gives the correct values.
 */
TEST(HeaderTest, channel_names_config)
{
    auto const config
        = ::routing::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::CHANNEL_NAMES);

    EXPECT_EQ(::blob::Config::Type::CHANNEL_NAMES, config.type);
    auto data                      = config.data;
    ::routing::Header const header = ::routing::load(data);
    EXPECT_EQ(::routing::ChannelType::UNKNOWN, header.channelType);
    EXPECT_EQ(::routing::INVALID_CHANNEL_ID, header.channelId);
}

/**
 * \desc: Loading the header of channel configs from a test blob gives the correct values.
 */
TEST(HeaderTest, channel_config)
{
    constexpr auto NUM_CHANNELS
        = ::routing::NUM_FLEXRAY_CHANNELS + ::routing::NUM_PDU_TRANSPORT_CHANNELS;
    uint8_t const channelIds[] = {1, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    ::routing::ChannelType const expectedChannelTypes[NUM_CHANNELS] = {
        ::routing::ChannelType::FR,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
    };

    for (size_t i = 0; i < NUM_CHANNELS; ++i)
    {
        auto const channelConfig = ::routing::config(
            ::blob::CONFIGURATION_BLOB, ::blob::Config::Type::CHANNEL, channelIds[i]);

        EXPECT_EQ(::blob::Config::Type::CHANNEL, channelConfig.type);
        auto data                      = channelConfig.data;
        ::routing::Header const header = ::routing::load(data);
        EXPECT_EQ(expectedChannelTypes[i], header.channelType) << " i = " << i;
        ;
        EXPECT_EQ(channelIds[i], header.channelId);
    }
}

/**
 * \desc: Loading the header of RX adapter configs from a test blob gives the correct values.
 */
TEST(HeaderTest, rx_adapter_config)
{
    ::routing::ChannelType const expectedChannelTypes[::routing::NUM_CHANNELS] = {
        ::routing::ChannelType::CAN,
        ::routing::ChannelType::FR,
        ::routing::ChannelType::CAN,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
    };

    ::blob::Blob blob(::blob::CONFIGURATION_BLOB);

    for (size_t i = 0; i < ::routing::NUM_CHANNELS; ++i)
    {
        auto const channelConfig
            = ::routing::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::RX_ADAPTER, i);

        EXPECT_EQ(::blob::Config::Type::RX_ADAPTER, channelConfig.type);
        auto data                      = channelConfig.data;
        ::routing::Header const header = ::routing::load(data);
        EXPECT_EQ(expectedChannelTypes[i], header.channelType) << " i = " << i;
        EXPECT_EQ(i, header.channelId);
    }
}

/**
 * \desc: Loading the header of TX adapter configs from a test blob gives the correct values.
 */
TEST(HeaderTest, tx_adapter_config)
{
    ::routing::ChannelType const expectedChannelTypes[::routing::NUM_CHANNELS] = {
        ::routing::ChannelType::CAN,
        ::routing::ChannelType::FR,
        ::routing::ChannelType::CAN,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
        ::routing::ChannelType::PDU_TRANSPORT,
    };

    ::blob::Blob blob(::blob::CONFIGURATION_BLOB);

    for (size_t i = 0; i < ::routing::NUM_CHANNELS; ++i)
    {
        auto const channelConfig
            = ::routing::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::TX_ADAPTER, i);

        EXPECT_EQ(::blob::Config::Type::TX_ADAPTER, channelConfig.type);
        auto data                      = channelConfig.data;
        ::routing::Header const header = ::routing::load(data);
        EXPECT_EQ(i, header.channelId);
        EXPECT_EQ(expectedChannelTypes[i], header.channelType) << " i = " << i;
    }
}

/**
 * \desc: Loading header from an empty blob doesn't crash.
 */
TEST(HeaderTest, empty_config)
{
    ::etl::span<uint8_t const> emptyData;
    auto const config = ::routing::config(emptyData, ::blob::Config::Type::CHANNEL_NAMES);

    EXPECT_EQ(0, config.data.size());

    auto data                      = config.data;
    ::routing::Header const header = ::routing::load(data);
    (void)header;
}

/**
 * \desc: Extracting config from a test blob with invalid config type returns an empty span.
 */
TEST(HeaderTest, incorrect_config_type)
{
    constexpr uint32_t INVALID_CONFIG_TYPE = 3;

    uint8_t const data[] = {0x00, 0x00, 0x00, 0x0B, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00,
                            0x14, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x8B, 0x31, 0x3D, 0x45};

    EXPECT_EQ(
        0,
        ::routing::config(data, static_cast<::blob::Config::Type>(INVALID_CONFIG_TYPE))
            .data.size());
}
} // namespace
