/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/TxAdapterTable.h"

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
 * Loading the TX adapter configs from a test blob gives the correct values.
 */
TEST(TxAdapterTableTest, tx_adapter_config)
{
    uint8_t const channelIds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::vector<std::vector<uint32_t>> const messageIds
        = {{257, 512, 1, 2, 5},
           {768},
           {},
           {256, 257, 258, 259, 2, 4242, 4242, 4242, 22, 111},
           {16384, 4096, 4098, 4099, 4100, 222},
           {333},
           {},
           {555},
           {666},
           {},
           {},
           {999}};

    std::vector<std::vector<uint32_t>> const messageLengths
        = {{8, 8, 8, 8, 8},
           {8},
           {},
           {8, 8, 4, 8, 8, 64, 64, 64, 8, 8},
           {8, 8, 8, 8, 8, 8},
           {8},
           {},
           {8},
           {8},
           {},
           {},
           {8}};

    std::vector<std::vector<uint32_t>> const pduOffsets
        = {{0, 0, 0, 0, 0},
           {0},
           {},
           {0, 0, 0, 4, 0, 0, 0, 0, 0, 0},
           {0, 0, 0, 0, 0, 0},
           {0},
           {},
           {0},
           {0},
           {},
           {},
           {0}};

    ::blob::Blob const blob(::blob::CONFIGURATION_BLOB);

    for (size_t i = 0; i < ::routing::NUM_CHANNELS; ++i)
    {
        auto const adapterConfig = ::routing::config(
            ::blob::CONFIGURATION_BLOB, ::blob::Config::Type::TX_ADAPTER, channelIds[i]);
        EXPECT_EQ(::blob::Config::Type::TX_ADAPTER, adapterConfig.type);

        ::routing::TxAdapterTable table;
        EXPECT_TRUE(routing::load(adapterConfig.data, table));

        EXPECT_EQ(messageIds[i].size(), table.messageIds.size());
        EXPECT_THAT(table.messageIds, ElementsAreArray(messageIds[i]));

        EXPECT_EQ(messageLengths[i].size(), table.messageLengths.size());
        EXPECT_THAT(table.messageLengths, ElementsAreArray(messageLengths[i]));

        EXPECT_EQ(pduOffsets[i].size(), table.pduOffsets.size());
        EXPECT_THAT(table.pduOffsets, ElementsAreArray(pduOffsets[i]));
    }
}

/**
 * \desc
 * Loading from a corrupted config returns an empty span.
 */
TEST(TxAdapterTableTest, corrupted_tx_adapter_config)
{
    uint8_t corruptedTxAdapterConfigData[]
        = {0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x09, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA4, 0x14, 0x59, 0xED};

    ::etl::span<uint8_t> txAdapterConfigData       = ::etl::make_span(corruptedTxAdapterConfigData);
    txAdapterConfigData.take<::etl::be_uint32_t>() = 3U; // config type

    ::routing::TxAdapterTable table;

    EXPECT_FALSE(routing::load(corruptedTxAdapterConfigData, table));
}

/**
 * \desc
 * Loading config from an empty blob doesn't crash.
 */
TEST(TxAdapterTableTest, empty_blob)
{
    ::etl::span<uint8_t const> emptyBlob;
    auto const txAdapterConfig = ::routing::config(emptyBlob, ::blob::Config::Type::TX_ADAPTER, 42);
    EXPECT_EQ(0, txAdapterConfig.data.size());

    ::routing::TxAdapterTable table;
    EXPECT_FALSE(routing::load(txAdapterConfig.data, table));
}

} // namespace
