/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportConfig.h"

#include "blob/configuration.h"
#include "routing/constants.h"
#include "routing/util.h"

#include <blob/Blob.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

/**
 * \desc: Loading the PDU transport config from a test blob gives the correct values.
 */
TEST(PduTransportConfigTest, pdu_transport_config)
{
    constexpr uint8_t FIRST_PDU_TRANSPORT_CHANNEL_ID
        = ::routing::NUM_CAN_CHANNELS + ::routing::NUM_FLEXRAY_CHANNELS;
    routing::PduTransportConfig::Mode const modes[::routing::NUM_PDU_TRANSPORT_CHANNELS]
        = {routing::PduTransportConfig::Mode::RX_TX,
           routing::PduTransportConfig::Mode::RX_TX,
           routing::PduTransportConfig::Mode::TX,
           routing::PduTransportConfig::Mode::RX,
           routing::PduTransportConfig::Mode::TX,
           routing::PduTransportConfig::Mode::TX,
           routing::PduTransportConfig::Mode::RX,
           routing::PduTransportConfig::Mode::RX,
           routing::PduTransportConfig::Mode::TX};

    uint8_t const pcps[::routing::NUM_PDU_TRANSPORT_CHANNELS] = {0, 1, 0, 0, 2, 0, 3, 0, 7};
    uint16_t const trasmissionTimeouts[::routing::NUM_PDU_TRANSPORT_CHANNELS]
        = {0, 0, 1, 0, 2, 3, 0, 0, 5};

    std::vector<std::vector<uint8_t>> remoteIpAddresses
        = {{0, 0, 0, 0},
           {0, 0, 0, 0, 1, 1, 1, 1},
           {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2},
           {},
           {},
           {},
           {},
           {},
           {}};

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        routing::PduTransportConfig pduTransportConfig;
        auto const channelConfig = ::routing::config(
            ::blob::CONFIGURATION_BLOB,
            ::blob::Config::Type::CHANNEL,
            FIRST_PDU_TRANSPORT_CHANNEL_ID + i);
        EXPECT_TRUE(routing::load(channelConfig.data, pduTransportConfig));

        EXPECT_EQ(ip::make_ip4(239, 192, 255, 253), pduTransportConfig.ipAddress);
        EXPECT_EQ(modes[i], pduTransportConfig.mode);
        EXPECT_EQ(1, pduTransportConfig.type);
        EXPECT_EQ(4446, pduTransportConfig.localPort);
        EXPECT_EQ(4446, pduTransportConfig.remotePort);
        EXPECT_EQ(pcps[i], pduTransportConfig.pcp);
        EXPECT_EQ(trasmissionTimeouts[i], pduTransportConfig.transmissionTimeout);
        EXPECT_THAT(pduTransportConfig.remoteIpAddresses, ElementsAreArray(remoteIpAddresses[i]));
    }
}

/**
 * \desc: Loading PDU transport config with address family IPv6 gives the correct values.
 */
TEST(PduTransportConfigTest, ipv6_pdu_transport_config)
{
    constexpr ip::IPAddress IP_ADDRESS               = ip::make_ip6(0x0, 0x0, 0x0, 0xEFC0FFFD);
    constexpr routing::PduTransportConfig::Mode MODE = routing::PduTransportConfig::Mode::RX_TX;
    constexpr uint8_t CONNECTION_TYPE                = 1U;
    constexpr uint16_t LOCAL_PORT                    = 4445U;
    constexpr uint16_t REMOTE_PORT                   = 4446U;
    constexpr uint8_t PCP                            = 0U;
    constexpr uint16_t TRANSMISSION_TIMEOUT          = 0U;
    constexpr etl::array<uint8_t, 32> REMOTE_IP_ADDRESSES
        = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    uint8_t channelConfigData[]
        = {0x00, 0x00, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x48, 0x01, 0x11, 0x00, 0x00, 0x11, 0x5D, 0x11, 0x5E, 0x00, 0x06, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0xC0, 0xFF, 0xFD,
           0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    ::routing::PduTransportConfig pduTransportConfig;

    EXPECT_TRUE(routing::load(channelConfigData, pduTransportConfig));

    EXPECT_EQ(IP_ADDRESS, pduTransportConfig.ipAddress);
    EXPECT_EQ(MODE, pduTransportConfig.mode);
    EXPECT_EQ(CONNECTION_TYPE, pduTransportConfig.type);
    EXPECT_EQ(LOCAL_PORT, pduTransportConfig.localPort);
    EXPECT_EQ(REMOTE_PORT, pduTransportConfig.remotePort);
    EXPECT_EQ(PCP, pduTransportConfig.pcp);
    EXPECT_EQ(TRANSMISSION_TIMEOUT, pduTransportConfig.transmissionTimeout);
    EXPECT_THAT(pduTransportConfig.remoteIpAddresses, ElementsAreArray(REMOTE_IP_ADDRESSES));
}

/**
 * \desc: Loading from a corrupted config returns a default-constructed config.
 */
TEST(PduTransportConfigTest, corrupted_pdu_transport_config)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x20, 0x01, 0x11, 0x00, 0x00, 0x11, 0x5D, 0x11, 0x5E,
                      0x00, 0x04, 0xEF, 0xC0, 0xFF, 0xFD, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
                      0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x69, 0xBF, 0x16, 0xEA};

    uint8_t corruptedPduTransportConfigData[sizeof(data)];
    ::etl::copy(::etl::make_span(data), ::etl::make_span(corruptedPduTransportConfigData));

    {
        ::etl::span<uint8_t> pduTransportConfigData
            = ::etl::make_span(corruptedPduTransportConfigData);
        pduTransportConfigData.take<::etl::be_uint32_t>() = 3U; // config type
    }

    ::routing::PduTransportConfig table;

    EXPECT_FALSE(routing::load(corruptedPduTransportConfigData, table));

    ::etl::copy(::etl::make_span(data), ::etl::make_span(corruptedPduTransportConfigData));

    {
        ::etl::span<uint8_t> pduTransportConfigData
            = ::etl::make_span(corruptedPduTransportConfigData);
        (void)pduTransportConfigData.take<::etl::be_uint32_t>(); // config type
        pduTransportConfigData.take<::etl::be_uint16_t>() = 3U;  // channel type
    }

    EXPECT_FALSE(routing::load(corruptedPduTransportConfigData, table));

    ::etl::copy(::etl::make_span(data), ::etl::make_span(corruptedPduTransportConfigData));

    {
        ::etl::span<uint8_t> pduTransportConfigData
            = ::etl::make_span(corruptedPduTransportConfigData);
        (void)pduTransportConfigData.take<::etl::be_uint32_t>(); // config type
        (void)pduTransportConfigData.take<::etl::be_uint16_t>(); // channel type
        (void)pduTransportConfigData.take<uint8_t>();            // unused
        (void)pduTransportConfigData.take<uint8_t>();            // channel ID
        (void)pduTransportConfigData.take<::etl::be_uint32_t>(); // reserved
        (void)pduTransportConfigData.take<::etl::be_uint32_t>(); // size
        (void)pduTransportConfigData.take<uint8_t>();            // connection type
        (void)pduTransportConfigData.take<uint8_t>();            // mode
        (void)pduTransportConfigData.take<::etl::be_uint16_t>(); // vlan ID
        (void)pduTransportConfigData.take<::etl::be_uint16_t>(); // local port
        (void)pduTransportConfigData.take<::etl::be_uint16_t>(); // remote port
        pduTransportConfigData.take<::etl::be_uint16_t>() = 3U;  // IP version
    }

    EXPECT_FALSE(routing::load(corruptedPduTransportConfigData, table));
}

/**
 * \desc: Loading config from an empty blob doesn't crash.
 */
TEST(PduTransportConfigTest, empty_blob)
{
    ::etl::span<uint8_t const> emptyBlob;

    routing::PduTransportConfig pduTransportConfig;
    auto const channelConfig = ::routing::config(emptyBlob, ::blob::Config::Type::CHANNEL, 42);

    EXPECT_EQ(0, channelConfig.data.size());
    EXPECT_FALSE(routing::load(channelConfig.data, pduTransportConfig));
}
} // namespace
