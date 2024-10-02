// Copyright 2024 Accenture.

#include "transport/LogicalAddress.h"
#include "transport/TransportConfiguration.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace ::transport;

namespace
{

TEST(LogicalAddress, TestConverter)
{
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convertDoipAddressTo8Bit(0xF1F2U)
        == 0x00FFU);
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convertDoipAddressTo8Bit(0xA11DU)
        == 0x0010U);
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convertDoipAddressTo8Bit(0xF1F3U)
        == 0xF1F3U);
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert8BitAddressToDoip(0x0001)
        == 0xDF01U);
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert8BitAddressToDoip(0x00FFU)
        == 0xF1F2U);
    EXPECT_TRUE(
        TransportConfiguration::LogicalAddressConverterUT::convert8BitAddressToDoip(0x00FEU)
        == 0x00FEU);
}

TEST(LogicalAddress, TestFind)
{
    EXPECT_TRUE(
        addressfinder::isDoipAddressIn(0xF1F2U, TransportConfiguration::INTERNAL_ADDRESS_RANGE));
    EXPECT_TRUE(
        addressfinder::isDoipAddressIn(0xDF01U, TransportConfiguration::EXTERNAL_ADDRESS_RANGE));
    EXPECT_FALSE(
        addressfinder::isDoipAddressIn(0xDF01U, TransportConfiguration::INTERNAL_ADDRESS_RANGE));
    EXPECT_FALSE(
        addressfinder::isDoipAddressIn(0xF1F2U, TransportConfiguration::EXTERNAL_ADDRESS_RANGE));
    EXPECT_TRUE(
        addressfinder::is8BitAddressIn(0x0012U, TransportConfiguration::INTERNAL_ADDRESS_RANGE));
    EXPECT_TRUE(
        addressfinder::is8BitAddressIn(0x0001U, TransportConfiguration::EXTERNAL_ADDRESS_RANGE));
    EXPECT_FALSE(
        addressfinder::is8BitAddressIn(0x0001U, TransportConfiguration::INTERNAL_ADDRESS_RANGE));
    EXPECT_FALSE(
        addressfinder::is8BitAddressIn(0x1200U, TransportConfiguration::EXTERNAL_ADDRESS_RANGE));
}
} // namespace
