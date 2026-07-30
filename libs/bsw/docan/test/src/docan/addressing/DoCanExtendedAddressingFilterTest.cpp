/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/addressing/DoCanExtendedAddressingFilter.h"

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
using DoCanExtendedAddressingFilterType = DoCanExtendedAddressingFilter<DataLinkLayerType>;
using DataLinkAddressPairType = DoCanExtendedAddressingFilterType::DataLinkAddressPairType;

static MapperType const mapper;
// NOLINTBEGIN(cert-err58-cpp): Lots of references to these names in this file, as such suppress was
// prefered here since it's just a test file.
static CodecType const codec(DoCanFrameCodecConfigPresets::PADDED_CLASSIC, mapper);
// NOLINTEND(cert-err58-cpp)

// A flat table of three independent nodes and the (raw, base) CAN identifier each of them
// transmits on. Any pair of nodes below can address each other: the address extension byte of a
// frame always identifies its target, and looking its transport id up in this table gives the CAN
// identifier to expect/send flow control on.
static DoCanExtendedAddressingFilterType::AddressEntryType const testEntries[] = {
    {0x513U, 0x11U},
    {0x638U, 0x22U},
    {0x745U, 0x33U},
};

/*
 * init will assert if the input entries slice is empty.
 */
TEST(DoCanExtendedAddressingFilterTest, initAssertsIfEntriesIsEmpty)
{
    ASSERT_THROW({ DoCanExtendedAddressingFilterType cut1({}, {}, codec); }, ::etl::exception);
    DoCanExtendedAddressingFilterType cut2;
    ASSERT_THROW(cut2.init({}, {}, codec), ::etl::exception);
}

TEST(DoCanExtendedAddressingFilterTest, testTransmissionParams)
{
    DoCanExtendedAddressingFilterType cut1(testEntries, {}, codec);
    DoCanExtendedAddressingFilterType cut2;
    cut2.init(testEntries, {}, codec);
    for (uint8_t idx = 0U; idx < 2U; ++idx)
    {
        DoCanExtendedAddressingFilterType& cut = (idx == 0U) ? cut1 : cut2;
        DataLinkAddressPairType dlPair;

        EXPECT_EQ(
            &codec, cut.getTransmissionParameters(DoCanTransportAddressPair(0x11, 0x22), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(
                AddressingType::pack(0x638U, 0x11U), AddressingType::pack(0x513U, 0x22U)),
            dlPair);

        EXPECT_EQ(
            &codec, cut.getTransmissionParameters(DoCanTransportAddressPair(0x22, 0x33), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(
                AddressingType::pack(0x745U, 0x22U), AddressingType::pack(0x638U, 0x33U)),
            dlPair);

        // unknown source id
        EXPECT_EQ(
            nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x99, 0x22), dlPair));
        // unknown target id
        EXPECT_EQ(
            nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x11, 0x99), dlPair));
    }
}

TEST(DoCanExtendedAddressingFilterTest, testReceptionParams)
{
    DoCanExtendedAddressingFilterType cut(testEntries, {}, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // request from node 0x11 to node 0x22, arriving on node 0x11's CAN identifier 0x513, carrying
    // the target address (0x22) in the address extension byte.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x22U), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x638U, 0x11U), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x11, 0x22), tPair);

    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x638U, 0x33U), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x745U, 0x22U), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x22, 0x33), tPair);

    // address extension byte does not correspond to any known node -> no match
    EXPECT_EQ(
        nullptr, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x99U), tPair, dlAddress));

    // unknown CAN identifier -> no match
    EXPECT_EQ(
        nullptr, cut.getReceptionParameters(AddressingType::pack(0x2U, 0x11U), tPair, dlAddress));
}

TEST(DoCanExtendedAddressingFilterTest, testReceptionAddressIsMatchedByRawCanIdOnly)
{
    DoCanExtendedAddressingFilterType cut(testEntries, {}, codec);
    // match() is invoked with the raw (unpacked) CAN identifier, before the address extension
    // byte has even been read from the payload.
    EXPECT_FALSE(cut.match(0x512U));
    EXPECT_TRUE(cut.match(0x513U));
    EXPECT_FALSE(cut.match(0x514U));
    EXPECT_FALSE(cut.match(0x744U));
    EXPECT_TRUE(cut.match(0x745U));
    EXPECT_FALSE(cut.match(0x746U));
}

TEST(DoCanExtendedAddressingFilterTest, testAscendingCanIdsExpected)
{
    static DoCanExtendedAddressingFilterType::AddressEntryType const unsortedEntries[] = {
        {0x638U, 0x22U},
        {0x513U, 0x11U},
        {0x745U, 0x33U},
    };
    ASSERT_THROW(
        { DoCanExtendedAddressingFilterType cut(unsortedEntries, {}, codec); }, ::etl::exception);
}

TEST(DoCanExtendedAddressingFilterTest, testDuplicateCanIdsAllowed)
{
    // A second entry may share its predecessor's _canId, e.g. to let a functional/broadcast
    // target address (0x44) resolve to the same, real CAN identifier as another node's physical
    // target address (0x33), both transmitted by the node at 0x745.
    static DoCanExtendedAddressingFilterType::AddressEntryType const entriesWithDuplicate[] = {
        {0x513U, 0x11U},
        {0x638U, 0x22U},
        {0x745U, 0x33U},
        {0x745U, 0x44U},
    };
    DoCanExtendedAddressingFilterType cut(entriesWithDuplicate, {}, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // request from node 0x11 to the functional/broadcast target 0x44 resolves to the same CAN
    // identifier (0x745) as a request targeting the real node 0x33 would.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x44U), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x745U, 0x11U), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x11, 0x44), tPair);
}

TEST(DoCanExtendedAddressingFilterTest, testFunctionalTargetReportsInvalidTransmissionAddress)
{
    // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target. Configuring
    // 0x44 as a functional transport address causes requests targeting it to report an invalid
    // transmission address, so DoCanReceiver rejects them if they are multi-frame requests.
    static DoCanExtendedAddressingFilterType::AddressEntryType const entriesWithFunctional[] = {
        {0x513U, 0x11U},
        {0x638U, 0x22U},
        {0x745U, 0x33U},
        {0x745U, 0x44U},
    };
    static uint16_t const functionalAddresses[] = {0x44U};
    DoCanExtendedAddressingFilterType cut(entriesWithFunctional, functionalAddresses, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // request to the functional target (0x44) reports an invalid transmission address...
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x44U), tPair, dlAddress));
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x11, 0x44), tPair);

    // ...while a request to the real physical target (0x33) is unaffected.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x33U), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x745U, 0x11U), dlAddress);
}

TEST(DoCanExtendedAddressingFilterTest, testMultipleFunctionalAddressesSupported)
{
    // ISO 15765-2 does not guarantee that a node only ever has a single functional (broadcast)
    // address, so more than one may be configured at once.
    static DoCanExtendedAddressingFilterType::AddressEntryType const entriesWithFunctionals[] = {
        {0x513U, 0x11U},
        {0x638U, 0x22U},
        {0x745U, 0x33U},
        {0x745U, 0x44U},
        {0x745U, 0x55U},
    };
    static uint16_t const functionalAddresses[] = {0x44U, 0x55U};
    DoCanExtendedAddressingFilterType cut(entriesWithFunctionals, functionalAddresses, codec);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    // both configured functional targets report an invalid transmission address...
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x44U), tPair, dlAddress));
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);

    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x55U), tPair, dlAddress));
    EXPECT_EQ(static_cast<uint32_t>(DataLinkLayerType::INVALID_ADDRESS), dlAddress);

    // ...while a request to the real physical target (0x33) is unaffected.
    EXPECT_EQ(
        &codec, cut.getReceptionParameters(AddressingType::pack(0x513U, 0x33U), tPair, dlAddress));
    EXPECT_EQ(AddressingType::pack(0x745U, 0x11U), dlAddress);
}

TEST(DoCanExtendedAddressingFilterTest, testFormatDataLinkAddress)
{
    DoCanExtendedAddressingFilterType cut(testEntries, {}, codec);
    char output[20];
    EXPECT_EQ(output, cut.formatDataLinkAddress(AddressingType::pack(0x7F1U, 0x2AU), output));
    EXPECT_STREQ("0x7f1/0x2a", output);
}

} // anonymous namespace
