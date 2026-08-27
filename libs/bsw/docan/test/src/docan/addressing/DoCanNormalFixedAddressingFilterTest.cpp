/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/addressing/DoCanNormalFixedAddressingFilter.h"

#include "docan/addressing/DoCanNormalFixedAddressing.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <gmock/gmock.h>

// Diagnostic addresses used throughout these tests. Defined as macros so the
// server (SA) and tester (TA) addresses can be re-targeted from a single place.
static constexpr uint8_t NF_TEST_SA = 0x2AU; // ECU / server diagnostic source address (OpenBSW SA)
static constexpr uint8_t NF_TEST_TA = 0xF3U; // tester address (TA)
static constexpr uint8_t NORMAL_FIXED_FUNCTIONAL_ADDRESS = 0x33U;

namespace
{
using namespace docan;

using DataLinkLayerType       = DoCanNormalFixedAddressing<>::DataLinkLayerType;
using CodecType               = DoCanFrameCodec<DataLinkLayerType>;
using MapperType              = DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;
using AddressingType          = DoCanNormalFixedAddressing<>;
using FilterType              = DoCanNormalFixedAddressingFilter<DataLinkLayerType>;
using DataLinkAddressPairType = FilterType::DataLinkAddressPairType;

static MapperType const mapper;
// NOLINTBEGIN(cert-err58-cpp): Lots of references to these names in this file, as such suppress was
// prefered here since it's just a test file.
static CodecType const codec(DoCanFrameCodecConfigPresets::PADDED_CLASSIC, mapper);
// NOLINTEND(cert-err58-cpp)

// functional addresses valid in this configuration, e.g. the default UDS functional address
// 0x33.
static uint8_t const functionalAddresses[] = {NORMAL_FIXED_FUNCTIONAL_ADDRESS};
// testers allowed to reach the ECU; a request whose source address is not listed here is dropped
// silently before UDS processing starts.
static uint8_t const allowedTesters[]      = {NF_TEST_TA};

TEST(DoCanNormalFixedAddressingFilterTest, testMatchAcceptsBothAddressingFormats)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);

    // physical, any target/source
    EXPECT_TRUE(cut.match(
        AddressingType::pack(AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_SA, NF_TEST_TA)));
    // functional, any target/source
    EXPECT_TRUE(cut.match(AddressingType::pack(
        AddressingType::FUNCTIONAL_ADDRESSING_ID_BASE,
        NORMAL_FIXED_FUNCTIONAL_ADDRESS,
        NF_TEST_TA)));

    // wrong addressing format must not match
    EXPECT_FALSE(cut.match(AddressingType::pack(0x18DC0000UL, NF_TEST_SA, NF_TEST_TA)));
    // base (11 bit) ids must not match
    EXPECT_FALSE(cut.match(0x7F1U));
}

TEST(DoCanNormalFixedAddressingFilterTest, testTransmissionParamsAlwaysUsesPhysicalAddressing)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    DataLinkAddressPairType dlPair;

    EXPECT_EQ(
        &codec,
        cut.getTransmissionParameters(DoCanTransportAddressPair(NF_TEST_SA, NF_TEST_TA), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(
            AddressingType::pack(
                AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_SA, NF_TEST_TA),
            AddressingType::pack(
                AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_TA, NF_TEST_SA)),
        dlPair);

    // transport ids must fit into a single address byte
    EXPECT_EQ(
        nullptr,
        cut.getTransmissionParameters(DoCanTransportAddressPair(0x100, NF_TEST_TA), dlPair));
    EXPECT_EQ(
        nullptr,
        cut.getTransmissionParameters(DoCanTransportAddressPair(NF_TEST_SA, 0x100), dlPair));
}

TEST(DoCanNormalFixedAddressingFilterTest, testReceptionParamsPhysicalRequest)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        &codec,
        cut.getReceptionParameters(
            AddressingType::pack(
                AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_SA, NF_TEST_TA),
            tPair,
            dlAddress));
    EXPECT_EQ(DoCanTransportAddressPair(NF_TEST_TA, NF_TEST_SA), tPair);
    EXPECT_EQ(
        AddressingType::pack(AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_TA, NF_TEST_SA),
        dlAddress);
}

TEST(DoCanNormalFixedAddressingFilterTest, testReceptionParamsRejectsUnknownTester)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // 0xF7 is not in the allowed-tester list, so an otherwise valid physical request is dropped
    // silently before UDS processing starts.
    EXPECT_EQ(
        nullptr,
        cut.getReceptionParameters(
            AddressingType::pack(AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_SA, 0xF7U),
            tPair,
            dlAddress));
}

TEST(DoCanNormalFixedAddressingFilterTest, testReceptionParamsRejectsPhysicalToFunctionalAddress)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // 0x33 is configured as a functional address, so it must not be accepted as a physical
    // target
    EXPECT_EQ(
        nullptr,
        cut.getReceptionParameters(
            AddressingType::pack(AddressingType::PHYSICAL_ADDRESSING_ID_BASE, 0x33U, NF_TEST_TA),
            tPair,
            dlAddress));
}

TEST(DoCanNormalFixedAddressingFilterTest, testReceptionParamsFunctionalRequest)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        &codec,
        cut.getReceptionParameters(
            AddressingType::pack(AddressingType::FUNCTIONAL_ADDRESSING_ID_BASE, 0x33U, NF_TEST_TA),
            tPair,
            dlAddress));
    EXPECT_EQ(DoCanTransportAddressPair(NF_TEST_TA, 0x33), tPair);
    // ISO 15765-2 forbids multi-frame requests to a functional target, so the transmission
    // address is reported as invalid, causing DoCanReceiver to reject any multi-frame request;
    // this has no effect on single-frame requests, whose (always physical) reply is addressed
    // independently once the request has been fully received.
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);
}

TEST(
    DoCanNormalFixedAddressingFilterTest,
    testReceptionParamsRejectsFunctionalToNonFunctionalAddress)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // NF_TEST_SA is not configured as a functional address
    EXPECT_EQ(
        nullptr,
        cut.getReceptionParameters(
            AddressingType::pack(
                AddressingType::FUNCTIONAL_ADDRESSING_ID_BASE, NF_TEST_SA, NF_TEST_TA),
            tPair,
            dlAddress));
}

TEST(DoCanNormalFixedAddressingFilterTest, testFormatDataLinkAddress)
{
    FilterType cut(functionalAddresses, allowedTesters, codec);
    char output[20];
    EXPECT_EQ(
        output,
        cut.formatDataLinkAddress(
            AddressingType::pack(
                AddressingType::PHYSICAL_ADDRESSING_ID_BASE, NF_TEST_SA, NF_TEST_TA),
            output));
    // NOTE: keep this literal in sync with NF_TEST_SA / NF_TEST_TA above.
    EXPECT_STREQ("0x2a/0xf3", output);
}

} // anonymous namespace
