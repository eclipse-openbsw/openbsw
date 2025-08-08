// Copyright 2025 Accenture.

#include "udp/DatagramPacket.h"

#include <gmock/gmock.h>

using namespace udp;

TEST(DatagramPacketTest, Constructor)
{
    uint8_t data[] = {0xAB, 0xBC};
    DatagramPacket packet(data, 2, ip::make_ip4(0x12345678), 80);

    ASSERT_EQ(data, packet.getData());
    ASSERT_EQ(2, packet.getLength());
    ASSERT_EQ(0x12345678U, ip4_to_u32(packet.getAddress()));
    ASSERT_EQ(80U, packet.getPort());

    ::ip::IPEndpoint endpoint(packet.getAddress(), 80);

    DatagramPacket tmp(data, 2, endpoint);
    ASSERT_EQ(0x12345678U, ip4_to_u32(packet.getAddress()));
    ASSERT_EQ(80U, packet.getPort());
    ASSERT_EQ(endpoint, packet.getEndpoint());
}

TEST(DatagramPacketTest, ComparisonOperator)
{
    uint8_t data1[] = {0xAB, 0xBC};
    uint8_t data2[] = {0xAB, 0xBD};
    uint8_t data3[] = {0xAB, 0xBC, 0xBD};

    ::ip::IPAddress address1 = ip::make_ip4(0x12345678);
    ::ip::IPAddress address2 = ip::make_ip4(0x12345679);

    // Same packet
    {
        DatagramPacket packet1(data1, 2, address1, 80);
        DatagramPacket packet2(data1, 2, address1, 80);
        ASSERT_TRUE(packet1 == packet2);
    }

    // Different endpoints
    {
        DatagramPacket packet1(data1, 2, address1, 80);
        DatagramPacket packet2(data1, 2, address2, 80);
        ASSERT_FALSE(packet1 == packet2);
    }

    // Different length
    {
        DatagramPacket packet1(data1, 2, address1, 80);
        DatagramPacket packet2(data3, 3, address1, 80);
        ASSERT_FALSE(packet1 == packet2);
    }

    // Different data
    {
        DatagramPacket packet1(data1, 2, address1, 80);
        DatagramPacket packet2(data2, 2, address1, 80);
        ASSERT_FALSE(packet1 == packet2);
    }
}
