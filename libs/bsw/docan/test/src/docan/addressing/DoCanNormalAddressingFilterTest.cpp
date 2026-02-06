// Copyright 2024 Accenture.

#include "docan/addressing/DoCanNormalAddressingFilter.h"

#include "docan/addressing/DoCanNormalAddressing.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <can/canframes/CanId.h>

#include <gmock/gmock.h>

namespace
{
using namespace can;
using namespace docan;

using DataLinkLayerType = DoCanNormalAddressing<>::DataLinkLayerType;
using CodecType         = DoCanFrameCodec<DataLinkLayerType>;
using MapperType        = DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;
using DoCanNormalAddressingFilterType = DoCanNormalAddressingFilter<DataLinkLayerType>;
using DataLinkAddressPairType         = DoCanNormalAddressingFilterType::DataLinkAddressPairType;

static MapperType const mapper;
// NOLINTBEGIN(cert-err58-cpp): Lots of references to these names in this file, as such suppress was
// prefered here since it's just a test file.
static CodecType const codec1(DoCanFrameCodecConfigPresets::PADDED_CLASSIC, mapper);
static CodecType const codec2(DoCanFrameCodecConfigPresets::PADDED_FD, mapper);
static CodecType const codec3(DoCanFrameCodecConfigPresets::PADDED_FD, mapper);
// NOLINTEND(cert-err58-cpp)

static CodecType const* codecEntries[4] = {
    nullptr,
    &codec1,
    &codec2,
    &codec3,
};

static DoCanNormalAddressingFilter<DataLinkLayerType>::AddressEntryType const testEntries[] = {
    {CanId::Base<0x513>::value, CanId::Base<0x638>::value, 0xf54, 0x83, 1, 2},
    {CanId::Base<0x638>::value, CanId::Base<0x7ff>::value, 0x1, 0xfe, 1, 2},
    {CanId::Base<0x745>::value, CanId::Base<0x52>::value, 0x55, 0x999, 1, 2},
    {CanId::Extended<0x513>::value, CanId::Extended<0x638>::value, 0x123, 0x987, 1, 2},
    {CanId::Extended<0x638>::value, CanId::Extended<0x7ff>::value, 0x456, 0x654, 2, 3},
    {CanId::Extended<0x744>::value, CanId::Extended<0x523>::value, 0x789, 0x321, 1, 2},
    {CanId::Invalid::value, CanId::Base<0x123>::value, 0x789, 0x111, 1, 2},
    {CanId::Invalid::value, CanId::Base<0x124>::value, 0x222, 0x333, 1, 2},
};

/*
 * init will assert if the input entries slice is empty.
 */
TEST(DoCanNormalAddressingFilterTest, initAssertsIfEntriesIsEmpty)
{
    ASSERT_THROW(
        { DoCanNormalAddressingFilter<DataLinkLayerType> cut1({}, codecEntries); },
        ::etl::exception);
    DoCanNormalAddressingFilter<DataLinkLayerType> cut2;
    ASSERT_THROW(cut2.init({}, codecEntries), ::etl::exception);
}

/*
 * When all entries are valid, all entries will be added to the addressing filter
 */
TEST(DoCanNormalAddressingFilterTest, initOnlyValidEntries)
{
    static DoCanNormalAddressingFilter<DataLinkLayerType>::AddressEntryType const testValidEntries[]
        = {{CanId::Base<0x513>::value, CanId::Base<0x638>::value, 0xf54, 0x83, 1, 2},
           {CanId::Extended<0x744>::value, CanId::Extended<0x523>::value, 0x789, 0x321, 1, 2}};

    DoCanNormalAddressingFilter<DataLinkLayerType> cut(testValidEntries, codecEntries);
    DataLinkAddressPairType dlPair;

    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x83, 0xf54), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Base<0x513>::value, CanId::Base<0x638>::value), dlPair);

    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x321, 0x789), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Extended<0x744>::value, CanId::Extended<0x523>::value),
        dlPair);
}

/*
 * A normal addressing filter can be constructed/initialized with some invalid entries as well, but
 * only at the end of the set of entries. The handling is to stop processing new entries as soon as
 * an invalid entry is found, and then to assert that all valid entries precede all invalid entries
 * in the set of entries
 */
TEST(DoCanNormalAddressingFilterTest, testTransmissionParams)
{
    DoCanNormalAddressingFilter<DataLinkLayerType> cut1(testEntries, codecEntries);
    DoCanNormalAddressingFilter<DataLinkLayerType> cut2;
    cut2.init(testEntries, codecEntries);
    for (uint8_t idx = 0U; idx < 2U; ++idx)
    {
        DoCanNormalAddressingFilter<DataLinkLayerType>& cut = (idx == 0U) ? cut1 : cut2;
        DataLinkAddressPairType dlPair;
        EXPECT_EQ(
            &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x83, 0xf54), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Base<0x513>::value, CanId::Base<0x638>::value), dlPair);

        EXPECT_EQ(
            &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0xfe, 0x01), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Base<0x638>::value, CanId::Base<0x7ff>::value), dlPair);

        EXPECT_EQ(
            &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x999, 0x55), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Base<0x745>::value, CanId::Base<0x52>::value), dlPair);

        EXPECT_EQ(
            &codec2,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x987, 0x123), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Extended<0x513>::value, CanId::Extended<0x638>::value),
            dlPair);

        EXPECT_EQ(
            &codec3,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x654, 0x456), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Extended<0x638>::value, CanId::Extended<0x7ff>::value),
            dlPair);

        EXPECT_EQ(
            &codec2,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x321, 0x789), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Extended<0x744>::value, CanId::Extended<0x523>::value),
            dlPair);

        EXPECT_EQ(
            &codec2,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x111, 0x789), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Invalid::value, CanId::Base<0x123>::value), dlPair);

        EXPECT_EQ(
            &codec2,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x333, 0x222), dlPair));
        EXPECT_EQ(
            DataLinkAddressPairType(CanId::Invalid::value, CanId::Base<0x124>::value), dlPair);

        EXPECT_EQ(
            nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x84, 0xf54), dlPair));
        EXPECT_EQ(
            nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x83, 0x5e), dlPair));
        EXPECT_EQ(
            nullptr,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x457, 0x123), dlPair));
        EXPECT_EQ(
            nullptr,
            cut.getTransmissionParameters(DoCanTransportAddressPair(0x456, 0x122), dlPair));
    }
}

TEST(DoCanNormalAddressingFilterTypeTest, testReceptionParams)
{
    DoCanNormalAddressingFilter<DataLinkLayerType> cut(testEntries, codecEntries);
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Base<0x513>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Base<0x638>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0xf54, 0x83), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Base<0x638>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Base<0x7ff>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x01, 0xfe), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Base<0x745>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Base<0x52>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x55, 0x999), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Extended<0x513>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Extended<0x638>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x123, 0x987), tPair);

    EXPECT_EQ(&codec2, cut.getReceptionParameters(CanId::Extended<0x638>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Extended<0x7ff>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x456, 0x654), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Extended<0x744>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Extended<0x523>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x789, 0x321), tPair);

    EXPECT_EQ(nullptr, cut.getReceptionParameters(0x2, tPair, dlAddress));
}

TEST(DoCanNormalAddressingFilterTypeTest, testReceptionAddressIsMatched)
{
    DoCanNormalAddressingFilterType cut(testEntries, codecEntries);
    EXPECT_FALSE(cut.match(CanId::Base<0x512>::value));
    EXPECT_TRUE(cut.match(CanId::Base<0x513>::value));
    EXPECT_FALSE(cut.match(CanId::Base<0x514>::value));
    EXPECT_FALSE(cut.match(CanId::Base<0x637>::value));
    EXPECT_TRUE(cut.match(CanId::Base<0x638>::value));
    EXPECT_FALSE(cut.match(CanId::Base<0x639>::value));
    EXPECT_FALSE(cut.match(CanId::Base<0x744>::value));
    EXPECT_TRUE(cut.match(CanId::Base<0x745>::value));
    EXPECT_FALSE(cut.match(CanId::Base<0x746>::value));
    EXPECT_FALSE(cut.match(CanId::Extended<0x512>::value));
    EXPECT_TRUE(cut.match(CanId::Extended<0x513>::value));
    EXPECT_FALSE(cut.match(CanId::Extended<0x514>::value));
    EXPECT_FALSE(cut.match(CanId::Extended<0x637>::value));
    EXPECT_TRUE(cut.match(CanId::Extended<0x638>::value));
    EXPECT_FALSE(cut.match(CanId::Extended<0x639>::value));
    EXPECT_FALSE(cut.match(CanId::Extended<0x743>::value));
    EXPECT_TRUE(cut.match(CanId::Extended<0x744>::value));
    EXPECT_FALSE(cut.match(CanId::Extended<0x745>::value));
}

TEST(DoCanNormalAddressingFilterTypeTest, testTableWithBaseEntriesOnlyIsMatched)
{
    DoCanNormalAddressingFilter<DataLinkLayerType> cut(testEntries, codecEntries);
    DataLinkAddressPairType dlPair;
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x83, 0xf54), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Base<0x513>::value, CanId::Base<0x638>::value), dlPair);

    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0xfe, 0x01), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Base<0x638>::value, CanId::Base<0x7ff>::value), dlPair);

    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x999, 0x55), dlPair));
    EXPECT_EQ(DataLinkAddressPairType(CanId::Base<0x745>::value, CanId::Base<0x52>::value), dlPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Base<0x513>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Base<0x638>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0xf54, 0x83), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Base<0x638>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Base<0x7ff>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x01, 0xfe), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Base<0x745>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Base<0x52>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x55, 0x999), tPair);
}

TEST(DoCanNormalAddressingFilterTypeTest, testTableWithExtendedEntriesOnlyIsMatched)
{
    DoCanNormalAddressingFilter<DataLinkLayerType> cut(testEntries, codecEntries);
    DataLinkAddressPairType dlPair;
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x987, 0x123), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Extended<0x513>::value, CanId::Extended<0x638>::value),
        dlPair);
    EXPECT_EQ(
        &codec3, cut.getTransmissionParameters(DoCanTransportAddressPair(0x654, 0x456), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Extended<0x638>::value, CanId::Extended<0x7ff>::value),
        dlPair);
    EXPECT_EQ(
        &codec2, cut.getTransmissionParameters(DoCanTransportAddressPair(0x321, 0x789), dlPair));
    EXPECT_EQ(
        DataLinkAddressPairType(CanId::Extended<0x744>::value, CanId::Extended<0x523>::value),
        dlPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Extended<0x513>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Extended<0x638>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x123, 0x987), tPair);

    EXPECT_EQ(&codec2, cut.getReceptionParameters(CanId::Extended<0x638>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Extended<0x7ff>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x456, 0x654), tPair);

    EXPECT_EQ(&codec1, cut.getReceptionParameters(CanId::Extended<0x744>::value, tPair, dlAddress));
    EXPECT_EQ(CanId::Extended<0x523>::value, dlAddress);
    EXPECT_EQ(DoCanTransportAddressPair(0x789, 0x321), tPair);
}

static DoCanNormalAddressingFilterType::AddressEntryType const invalidEntries[] = {
    {CanId::Invalid::value, CanId::Base<0x123>::value, 0x789, 0x111, 1, 2},
    {CanId::Invalid::value, CanId::Base<0x124>::value, 0x222, 0x333, 1, 2},
};

TEST(DoCanNormalAddressingFilterTypeTest, testTableWithInvalidEntriesOnlyIsMatched)
{
    DoCanNormalAddressingFilterType cut(invalidEntries, codecEntries);
    DataLinkAddressPairType dlPair;
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x457, 0x123), dlPair));
    EXPECT_EQ(
        nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x456, 0x122), dlPair));
    EXPECT_EQ(nullptr, cut.getReceptionParameters(CanId::Invalid::value, tPair, dlAddress));
}

static DoCanNormalAddressingFilterType::AddressEntryType const unsortedEntries[]
    = {{CanId::Base<0x513>::value, CanId::Base<0x638>::value, 0xf54, 0x83, 1, 2},
       {CanId::Base<0x745>::value, CanId::Base<0x52>::value, 0x55, 0x999, 1, 2},
       {CanId::Base<0x638>::value, CanId::Base<0x7ff>::value, 0x1, 0xfe, 1, 2}};

TEST(DoCanNormalAddressingFilterTypeTest, testAscendingReceptionAddressesExpected)
{
    ASSERT_THROW(
        { DoCanNormalAddressingFilterType cut(unsortedEntries, codecEntries); }, ::etl::exception);
}

static DoCanNormalAddressingFilterType::AddressEntryType const invalidInvalidEntries[] = {
    {CanId::Invalid::value, CanId::Base<0x123>::value, 0x789, 0x111, 1, 2},
    {CanId::Base<0x777>::value, CanId::Base<0x124>::value, 0x222, 0x333, 1, 2},
};

/*
 * The init function will assert if there are any invalid address entries preceding valid address
 * entries
 */
TEST(DoCanNormalAddressingFilterTypeTest, testOnlyInvalidAddressesExpectedAtEnd)
{
    ASSERT_THROW(
        { DoCanNormalAddressingFilterType cut(invalidInvalidEntries, codecEntries); },
        ::etl::exception);
}

static DoCanNormalAddressingFilterType::AddressEntryType const anyEntries[] = {
    {CanId::Invalid::value, CanId::Base<0x123>::value, 0x789, 0x111, 1, 2},
    {CanId::Invalid::value, CanId::Base<0x124>::value, 0x222, 0x333, 1, 2},
};

TEST(DoCanNormalAddressingFilterTypeTest, testFormatTableWithInvalidEntriesOnlyIsMatched)
{
    {
        DoCanNormalAddressingFilterType cut(anyEntries, codecEntries);
        char output[20];
        EXPECT_EQ(output, cut.formatDataLinkAddress(0x13f485U, output));
        EXPECT_EQ(0, ::strcmp(output, "0x0013f485"));
    }
    {
        DoCanNormalAddressingFilterType cut(anyEntries, codecEntries);
        char output[7];
        output[6] = 0x7f;
        EXPECT_EQ(
            output, cut.formatDataLinkAddress(0x13f485U, ::etl::span<char>(output).first(6U)));
        EXPECT_EQ(0, ::strcmp(output, "0x001"));
        EXPECT_EQ(0x7f, output[6]);
    }
}

// sizeof(codecEntries) will always get us to indices past what's valid
static DoCanNormalAddressingFilterType::AddressEntryType const outOfBoundsEntries[]
    = {{CanId::Base<0x123>::value,
        CanId::Base<0x456>::value,
        0x111,
        0x222,
        sizeof(codecEntries),
        sizeof(codecEntries)}};

/*
 * Out-of-bounds indices for the codecs will return nullptrs from getTransmissionParameters &
 * getReceptionParameters
 */
TEST(DoCanNormalAddressingFilterTypeTest, testEntryWithOutOfBoundsCodecIndices)
{
    DoCanNormalAddressingFilterType cut(outOfBoundsEntries, codecEntries);
    DataLinkAddressPairType dlPair;
    uint32_t dlAddress;
    DoCanTransportAddressPair tPair;

    EXPECT_EQ(
        nullptr, cut.getTransmissionParameters(DoCanTransportAddressPair(0x222, 0x111), dlPair));
    EXPECT_EQ(nullptr, cut.getReceptionParameters(CanId::Base<0x123>::value, tPair, dlAddress));
}

} // anonymous namespace
