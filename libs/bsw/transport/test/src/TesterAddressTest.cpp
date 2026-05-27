/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "transport/LogicalAddress.h"
#include "transport/TransportConfiguration.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace ::transport;

namespace
{

TEST(LogicalAddress, TestConverter)
{
    // Ethernet tester: 2-byte 0x0ECD -> 1-byte 0x00F0
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert2ByteAddressTo1Byte(0x0ECDU)
        == 0x00F0U);
    // CAN tester: 2-byte 0x0E10 -> 1-byte 0x00F3
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert2ByteAddressTo1Byte(0x0E10U)
        == 0x00F3U);
    // Unknown 2-byte address: passes through unchanged
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert2ByteAddressTo1Byte(0x0E99U)
        == 0x0E99U);
    // CAN tester: 1-byte 0x00F4 -> 2-byte 0x0E01
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert1ByteAddressTo2Byte(0x00F4U)
        == 0x0E01U);
    // Ethernet tester: 1-byte 0x00F2 -> 2-byte 0x0EFF
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert1ByteAddressTo2Byte(0x00F2U)
        == 0x0EFFU);
    // Unknown 1-byte address: passes through unchanged
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert1ByteAddressTo2Byte(0x00FEU)
        == 0x00FEU);
}

TEST(LogicalAddress, TestFind)
{
    EXPECT_TRUE(addressfinder::is2ByteAddressIn(
        0x0ECDU, TransportConfiguration::TESTER_ADDRESS_RANGE_ETHERNET));
    EXPECT_TRUE(
        addressfinder::is2ByteAddressIn(0x0E01U, TransportConfiguration::TESTER_ADDRESS_RANGE_CAN));
    EXPECT_FALSE(addressfinder::is2ByteAddressIn(
        0x0E01U, TransportConfiguration::TESTER_ADDRESS_RANGE_ETHERNET));
    EXPECT_FALSE(
        addressfinder::is2ByteAddressIn(0x0ECDU, TransportConfiguration::TESTER_ADDRESS_RANGE_CAN));
    EXPECT_TRUE(addressfinder::is1ByteAddressIn(
        0x00F1U, TransportConfiguration::TESTER_ADDRESS_RANGE_ETHERNET));
    EXPECT_TRUE(
        addressfinder::is1ByteAddressIn(0x00F4U, TransportConfiguration::TESTER_ADDRESS_RANGE_CAN));
    EXPECT_FALSE(addressfinder::is1ByteAddressIn(
        0x00F4U, TransportConfiguration::TESTER_ADDRESS_RANGE_ETHERNET));
    EXPECT_FALSE(
        addressfinder::is1ByteAddressIn(0x00F1U, TransportConfiguration::TESTER_ADDRESS_RANGE_CAN));
}
} // namespace
