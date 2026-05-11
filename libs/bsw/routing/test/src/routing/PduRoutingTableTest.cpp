/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduRoutingTable.h"
#include "blob/configuration.h"
#include "routing/pduRouting.h"
#include "routing/util.h"

#include <blob/Blob.h>
#include <blob/util.h>
#include <etl/array.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

/**
 * \desc
 * Loading the routing config from a test blob gives the correct values.
 */
TEST(PduRoutingTableTest, routing_config)
{
    ::etl::array<::etl::be_uint32_t, 26> const expectedOutputMessageIds{
        ::etl::be_uint32_t(0x100U),  ::etl::be_uint32_t(0x101U),  ::etl::be_uint32_t(0x102U),
        ::etl::be_uint32_t(0x103U),  ::etl::be_uint32_t(0x300),   ::etl::be_uint32_t(0x4000U),
        ::etl::be_uint32_t(0x2U),    ::etl::be_uint32_t(0x1092U), ::etl::be_uint32_t(0x1092U),
        ::etl::be_uint32_t(0x1092U), ::etl::be_uint32_t(0x101U),  ::etl::be_uint32_t(0x200U),
        ::etl::be_uint32_t(0x1U),    ::etl::be_uint32_t(0x1000U), ::etl::be_uint32_t(0x1002U),
        ::etl::be_uint32_t(0x1003U), ::etl::be_uint32_t(0x1004U), ::etl::be_uint32_t(0x2U),
        ::etl::be_uint32_t(0x5U),    ::etl::be_uint32_t(0x16U),   ::etl::be_uint32_t(0x14DU),
        ::etl::be_uint32_t(0x22BU),  ::etl::be_uint32_t(0x6FU),   ::etl::be_uint32_t(0x29AU),
        ::etl::be_uint32_t(0xDEU),   ::etl::be_uint32_t(0x3E7U),
    };

    ::etl::array<uint8_t, 26> const expectedDestinations
        = {3, 3, 3, 3, 1, 4, 3, 3, 3, 3, 0, 0, 0, 4, 4, 4, 4, 0, 0, 3, 5, 7, 3, 8, 4, 11};

    ::etl::array<::etl::be_uint32_t, 23> const expectedOffsets
        = {::etl::be_uint32_t(0U),
           ::etl::be_uint32_t(1U),
           ::etl::be_uint32_t(2U),
           ::etl::be_uint32_t(3U),
           ::etl::be_uint32_t(4U),
           ::etl::be_uint32_t(5U),
           ::etl::be_uint32_t(6U),
           ::etl::be_uint32_t(7U),
           ::etl::be_uint32_t(8U),
           ::etl::be_uint32_t(9U),
           ::etl::be_uint32_t(10U),
           ::etl::be_uint32_t(11U),
           ::etl::be_uint32_t(12U),
           ::etl::be_uint32_t(14U),
           ::etl::be_uint32_t(15U),
           ::etl::be_uint32_t(16U),
           ::etl::be_uint32_t(17U),
           ::etl::be_uint32_t(18U),
           ::etl::be_uint32_t(20U),
           ::etl::be_uint32_t(21U),
           ::etl::be_uint32_t(22U),
           ::etl::be_uint32_t(24U),
           ::etl::be_uint32_t(expectedDestinations.size())};

    auto const routingConfig
        = ::blob::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::ROUTING);
    EXPECT_EQ(::blob::Config::Type::ROUTING, routingConfig.type);

    ::routing::PduRoutingTable table;
    EXPECT_TRUE(::routing::load(routingConfig.data, table));

    EXPECT_THAT(table.destinationOffsets, ElementsAreArray(expectedOffsets));
    EXPECT_THAT(table.destinations, ElementsAreArray(expectedDestinations));
    EXPECT_THAT(table.outputMessageIds, ElementsAreArray(expectedOutputMessageIds));
}

/**
 * \desc
 * Loading from a corrupted config returns an empty span.
 */
TEST(PduRoutingTableTest, corrupted_routing_config)
{
    uint8_t corruptedRoutingConfigData[]
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    ::etl::span<uint8_t> routingConfigData       = ::etl::make_span(corruptedRoutingConfigData);
    routingConfigData.take<::etl::be_uint32_t>() = 3U; // config type

    ::routing::PduRoutingTable table;

    EXPECT_FALSE(routing::load(corruptedRoutingConfigData, table));
}

/**
 * \desc
 * Loading config from an empty blob doesn't crash.
 */
TEST(PduRoutingTableTest, empty_blob)
{
    ::etl::span<uint8_t const> emptyBlob;
    auto const routingConfig = ::routing::config(emptyBlob, ::blob::Config::Type::ROUTING);
    EXPECT_EQ(0, routingConfig.data.size());

    ::routing::PduRoutingTable table;
    EXPECT_FALSE(routing::load(routingConfig.data, table));
}

} // namespace
