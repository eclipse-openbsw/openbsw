/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/addressing/DoCanRangeExtendedAddressingFilter.h"

#include "docan/addressing/DoCanExtendedAddressing.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;

using DataLinkLayerType = DoCanExtendedAddressing<>::DataLinkLayerType;
using CodecType         = DoCanFrameCodec<DataLinkLayerType>;
using MapperType        = DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;
using AddressingType    = DoCanExtendedAddressing<>;
using DoCanRangeExtendedAddressingFilterType
    = DoCanRangeExtendedAddressingFilter<DataLinkLayerType>;
using DataLinkAddressPairType = DoCanRangeExtendedAddressingFilterType::DataLinkAddressPairType;

static MapperType const mapper;
// NOLINTBEGIN(cert-err58-cpp): Lots of references to these names in this file, as such suppress was
// prefered here since it's just a test file.
static CodecType const codec(DoCanFrameCodecConfigPresets::PADDED_CLASSIC, mapper);

// NOLINTEND(cert-err58-cpp)

/*
 * init will assert if the range is empty.
 */
TEST(DoCanRangeExtendedAddressingFilterTest, initAssertsIfRangeIsEmpty)
{
    ASSERT_THROW(
        { DoCanRangeExtendedAddressingFilterType cut1(0x700U, 0x00U, 0U, {}, codec); },
        ::etl::exception);
    DoCanRangeExtendedAddressingFilterType cut2;
    ASSERT_THROW(cut2.init(0x700U, 0x00U, 0U, {}, codec), ::etl::exception);
}

TEST(DoCanRangeExtendedAddressingFilterTest, initAssertsIfCanIdRangeExceedsBaseId)
{
    // 0x700 + 0x100 - 1 = 0x7ff fits, but one more does not
    ASSERT_THROW(
        { DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x00U, 0x101U, {}, codec); },
        ::etl::exception);
}

TEST(DoCanRangeExtendedAddressingFilterTest, initAssertsIfTransportIdRangeExceedsAddressByte)
{
    ASSERT_THROW(
        { DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x80U, 0x81U, {}, codec); },
        ::etl::exception);
}

TEST(DoCanRangeExtendedAddressingFilterTest, testTransmissionParams)
{
    DoCanRangeExtendedAddressingFilterType cut1(0x700U, 0x00U, 0x100U, {}, codec);
    DoCanRangeExtendedAddressingFilterType cut2;
    cut2.init(0x700U, 0x00U, 0x100U, {}, codec);
    for (uint8_t idx = 0U; idx < 2U; ++idx)
    {
        DoCanRangeExtendedAddressingFilterType& cut = (idx == 0U) ? cut1 : cut2;
        DataLinkAddressPairType dlPair;

        EXPECT_EQ(
            &codec, cut.getTransmissionParameters(DoCanTransportAddressPair(0xF1, 0x2A), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(
                AddressingType::pack(0x72AU, 0xF1U), AddressingType::pack(0x7F1U, 0x2AU)),
            dlPair);

        // out of range source/target transport ids
        EXPECT_EQ(
            nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x101, 0x2A), dlPair));
        EXPECT_EQ(
            nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0xF1, 0x101), dlPair));
    }
}

TEST(DoCanRangeExtendedAddressingFilterTest, testReceptionParams)
{
    DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x00U, 0x100U, {}, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // request from node 0xF1 to node 0x2A, arriving on node 0xF1's CAN identifier 0x7F1, carrying
    // the target address (0x2A) in the address extension byte.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x7F1U, 0x2AU), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x72AU, 0xF1U), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0xF1, 0x2A), tPair);

    // CAN identifier out of the configured range -> no match
    EXPECT_EQ(
        nullptr, cut.getReceptionParameters(AddressingType::pack(0x2U, 0xF1U), tPair, dlAddress));
}

TEST(DoCanRangeExtendedAddressingFilterTest, testFunctionalTargetReportsInvalidTransmissionAddress)
{
    // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target. Configuring
    // 0xDF as a functional transport address causes requests targeting it (still within the
    // configured 0x00-0xFF range, and thus otherwise indistinguishable from any other target) to
    // report an invalid transmission address, so DoCanReceiver rejects them if they are
    // multi-frame requests.
    static uint16_t const functionalAddresses[] = {0xDFU};
    DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x00U, 0x100U, functionalAddresses, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // request to the functional target (0xDF) reports an invalid transmission address...
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x7F1U, 0xDFU), tPair, dlAddress));
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0xF1, 0xDF), tPair);

    // ...while a request to a real physical target (0x2A) is unaffected.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x7F1U, 0x2AU), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x72AU, 0xF1U), dlAddress);
}

TEST(DoCanRangeExtendedAddressingFilterTest, testBaseTransportIdOffset)
{
    // CAN ids 0x700-0x70F map to transport ids 0x10-0x1F
    DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x10U, 0x10U, {}, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x700U, 0x1FU), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x70FU, 0x10U), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x10, 0x1F), tPair);

    // transport id below the configured range -> no match
    EXPECT_EQ(
        nullptr, cut.getReceptionParameters(AddressingType::pack(0x700U, 0x0FU), tPair, dlAddress));
}

TEST(DoCanRangeExtendedAddressingFilterTest, testMultipleFunctionalAddressesSupported)
{
    // ISO 15765-2 does not guarantee that a node only ever has a single functional (broadcast)
    // address, so more than one may be configured at once.
    static uint16_t const functionalAddresses[] = {0xDFU, 0xE0U};
    DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x00U, 0x100U, functionalAddresses, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // both configured functional targets report an invalid transmission address...
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x7F1U, 0xDFU), tPair, dlAddress));
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);

    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x7F1U, 0xE0U), tPair, dlAddress));
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);

    // ...while a request to a real physical target (0x2A) is unaffected.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x7F1U, 0x2AU), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x72AU, 0xF1U), dlAddress);
}

TEST(DoCanRangeExtendedAddressingFilterTest, testMatchUsesCanIdRangeOnly)
{
    DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x00U, 0x100U, {}, codec);
    EXPECT_FALSE(cut.match(0x6FFU));
    EXPECT_TRUE(cut.match(0x700U));
    EXPECT_TRUE(cut.match(0x7FFU));
    EXPECT_FALSE(cut.match(0x800U));
}

TEST(DoCanRangeExtendedAddressingFilterTest, testFormatDataLinkAddress)
{
    DoCanRangeExtendedAddressingFilterType cut(0x700U, 0x00U, 0x100U, {}, codec);
    char output[20];
    EXPECT_EQ(output, cut.formatDataLinkAddress(AddressingType::pack(0x7F1U, 0x2AU), output));
    EXPECT_STREQ("0x7f1/0x2a", output);
}

} // anonymous namespace
