/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "udp/DatagramPacket.h"

#include <etl/array.h>
#include <gmock/gmock.h>

using namespace udp;

TEST(DatagramPacketTest, Constructor)
{
    ::etl::array<uint8_t, 2U> const data = {0xAB, 0xBC};
    DatagramPacket packet(data.data(), 2, ip::make_ip4(0x12345678), 80);

    ASSERT_EQ(data.data(), packet.getData());
    ASSERT_EQ(2, packet.getLength());
    ASSERT_EQ(0x12345678U, ip4_to_u32(packet.getAddress()));
    ASSERT_EQ(80U, packet.getPort());

    ::ip::IPEndpoint endpoint(packet.getAddress(), 80);

    DatagramPacket tmp(data.data(), 2, endpoint);
    ASSERT_EQ(0x12345678U, ip4_to_u32(packet.getAddress()));
    ASSERT_EQ(80U, packet.getPort());
    ASSERT_EQ(endpoint, packet.getEndpoint());
}

TEST(DatagramPacketTest, ComparisonOperator)
{
    ::etl::array<uint8_t, 2U> const data1 = {0xAB, 0xBC};
    ::etl::array<uint8_t, 2U> const data2 = {0xAB, 0xBD};
    ::etl::array<uint8_t, 3U> const data3 = {0xAB, 0xBC, 0xBD};

    ::ip::IPAddress address1 = ip::make_ip4(0x12345678);
    ::ip::IPAddress address2 = ip::make_ip4(0x12345679);

    // Same packet
    {
        DatagramPacket packet1(data1.data(), 2, address1, 80);
        DatagramPacket packet2(data1.data(), 2, address1, 80);
        ASSERT_TRUE(packet1 == packet2);
    }

    // Different endpoints
    {
        DatagramPacket packet1(data1.data(), 2, address1, 80);
        DatagramPacket packet2(data1.data(), 2, address2, 80);
        ASSERT_FALSE(packet1 == packet2);
    }

    // Different length
    {
        DatagramPacket packet1(data1.data(), 2, address1, 80);
        DatagramPacket packet2(data3.data(), 3, address1, 80);
        ASSERT_FALSE(packet1 == packet2);
    }

    // Different data
    {
        DatagramPacket packet1(data1.data(), 2, address1, 80);
        DatagramPacket packet2(data2.data(), 2, address1, 80);
        ASSERT_FALSE(packet1 == packet2);
    }
}
