/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/RxAdapterTable.h"

#include "blob/configuration.h"
#include "routing/constants.h"
#include "routing/util.h"

#include <blob/Blob.h>

#include <gmock/gmock.h>

#include <vector>

namespace
{
using namespace ::testing;

/**
 * \desc
 * Loading the RX adapter configs from a test blob gives the correct values.
 */
TEST(RxAdapterTableTest, rx_adapter_config)
{
    uint8_t const channelIds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    uint32_t const firstIds[]  = {0, 7, 10, 10, 17, 18, 18, 20, 20, 20, 21, 22};

    std::vector<std::vector<uint32_t>> const messageIds
        = {{1, 2, 3, 4, 257},
           {2561, 4242, 3435969450},
           {},
           {2, 17, 18, 19, 20, 21, 257},
           {20480},
           {},
           {444, 445},
           {},
           {},
           {777},
           {888},
           {}};

    std::vector<std::vector<uint32_t>> const messageLengths
        = {{}, {}, {}, {8, 8, 8, 8, 8, 8, 8}, {8}, {}, {8, 8}, {}, {}, {8}, {8}, {}};

    std::vector<std::vector<uint32_t>> const pduLengthsOffsets
        = {{0, 1, 4, 5, 6, 7},
           {0, 1, 2, 3},
           {0},
           {0, 1, 2, 3, 4, 5, 6, 7},
           {0, 1},
           {0},
           {0, 1, 2},
           {0},
           {0},
           {0, 1},
           {0, 1},
           {0}};

    std::vector<std::vector<uint32_t>> const pduLengths
        = {{8, 8, 4, 4, 8, 8, 8},
           {64, 64, 64},
           {},
           {8, 8, 8, 8, 8, 8, 8},
           {8},
           {},
           {8, 8},
           {},
           {},
           {8},
           {8},
           {}};

    std::vector<std::vector<uint32_t>> const pduOffsets
        = {{0, 0, 0, 4, 0, 0, 0},
           {0, 0, 0},
           {},
           {0, 0, 0, 0, 0, 0, 0},
           {0},
           {},
           {0, 0},
           {},
           {},
           {0},
           {0},
           {}};

    ::blob::Blob const blob(::blob::CONFIGURATION_BLOB);

    for (size_t i = 0; i < ::routing::NUM_CHANNELS; ++i)
    {
        auto const adapterConfig = ::routing::config(
            ::blob::CONFIGURATION_BLOB, ::blob::Config::Type::RX_ADAPTER, channelIds[i]);
        EXPECT_EQ(::blob::Config::Type::RX_ADAPTER, adapterConfig.type);

        ::routing::RxAdapterTable table;
        EXPECT_TRUE(routing::load(adapterConfig.data, table));

        EXPECT_EQ(firstIds[i], table.firstId);

        EXPECT_EQ(messageIds[i].size(), table.messageIds.size());
        EXPECT_THAT(table.messageIds, ElementsAreArray(messageIds[i]));

        EXPECT_EQ(messageLengths[i].size(), table.messageLengths.size());
        EXPECT_THAT(table.messageLengths, ElementsAreArray(messageLengths[i]));

        EXPECT_EQ(pduLengthsOffsets[i].size(), table.pduLengthsOffsets.size());
        EXPECT_THAT(table.pduLengthsOffsets, ElementsAreArray(pduLengthsOffsets[i]));

        EXPECT_EQ(pduLengths[i].size(), table.pduLengths.size());
        EXPECT_THAT(table.pduLengths, ElementsAreArray(pduLengths[i]));

        EXPECT_EQ(pduOffsets[i].size(), table.pduOffsets.size());
        EXPECT_THAT(table.pduOffsets, ElementsAreArray(pduOffsets[i]));
    }
}

/**
 * \desc
 * Loading from a corrupted config returns a default-constructed table.
 */
TEST(RxAdapterTableTest, corrupted_rx_adapter_config)
{
    uint8_t corruptedRxAdapterConfigData[]
        = {0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x5D, 0xD1, 0x9A};

    ::etl::span<uint8_t> rxAdapterConfigData       = ::etl::make_span(corruptedRxAdapterConfigData);
    rxAdapterConfigData.take<::etl::be_uint32_t>() = 3U; // config type

    ::routing::RxAdapterTable table;

    EXPECT_FALSE(routing::load(corruptedRxAdapterConfigData, table));
}

/**
 * \desc
 * Loading config from an empty blob doesn't crash.
 */
TEST(RxAdapterTableTest, empty_blob)
{
    ::etl::span<uint8_t const> emptyBlob;
    auto const rxAdapterConfig = ::routing::config(emptyBlob, ::blob::Config::Type::RX_ADAPTER, 42);
    EXPECT_EQ(0, rxAdapterConfig.data.size());

    ::routing::RxAdapterTable table;
    EXPECT_FALSE(routing::load(rxAdapterConfig.data, table));
}

} // namespace
