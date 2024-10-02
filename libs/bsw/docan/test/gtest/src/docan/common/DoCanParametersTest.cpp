// Copyright 2024 Accenture.

#include "docan/common/DoCanParameters.h"

#include <estd/functional.h>

#include <gmock/gmock.h>

namespace
{
using namespace docan;
using namespace transport;

uint32_t systemUs() { return 0; }

TEST(DoCanParametersTest, testConstructedParametersAndSetter)
{
    DoCanParameters cut(
        ::estd::function<decltype(systemUs)>::create<&systemUs>(),
        23425,
        3543,
        1232,
        3442,
        98,
        153,
        75000,
        122);
    EXPECT_EQ(23425, cut.getWaitAllocateTimeout());
    EXPECT_EQ(3543, cut.getWaitRxTimeout());
    EXPECT_EQ(1232, cut.getWaitTxCallbackTimeout());
    EXPECT_EQ(3442, cut.getWaitFlowControlTimeout());
    EXPECT_EQ(98, cut.getMaxAllocateRetryCount());
    EXPECT_EQ(153, cut.getMaxFlowControlWaitCount());
    EXPECT_EQ(DoCanParameters::encodeMinSeparationTime(75000), cut.getEncodedMinSeparationTime());
    EXPECT_EQ(122, cut.getMaxBlockSize());
    cut.setMaxBlockSize(167);
    EXPECT_EQ(167, cut.getMaxBlockSize());
    cut.setEncodedMinSeparationTime(0x88);
    EXPECT_EQ(0x88, cut.getEncodedMinSeparationTime());
}

TEST(DoCanParameters, testDecodeMinSeparationTime)
{
    EXPECT_EQ(0U, DoCanParameters::decodeMinSeparationTime(0x00));
    EXPECT_EQ(1000U, DoCanParameters::decodeMinSeparationTime(0x01));
    EXPECT_EQ(0x7F * 1000U, DoCanParameters::decodeMinSeparationTime(0x7F));
    EXPECT_EQ(0x7F * 1000U, DoCanParameters::decodeMinSeparationTime(0x80));
    EXPECT_EQ(0x7F * 1000U, DoCanParameters::decodeMinSeparationTime(0xEF));
    EXPECT_EQ(0x7F * 1000U, DoCanParameters::decodeMinSeparationTime(0xF0));
    // Test that all encoded values 0xF1-0xF9 correspond to 100-900us
    for (uint32_t value = 1; value <= 9; ++value)
    {
        EXPECT_EQ(value * 100U, DoCanParameters::decodeMinSeparationTime(value + 0xF0));
    }
    EXPECT_EQ(0x7F * 1000U, DoCanParameters::decodeMinSeparationTime(0xFA));
}

TEST(DoCanParameters, testEncodeMinSeparationTime)
{
    EXPECT_EQ(0x00, DoCanParameters::encodeMinSeparationTime(0U));
    EXPECT_EQ(0x01, DoCanParameters::encodeMinSeparationTime(1000U));
    EXPECT_EQ(0x01, DoCanParameters::encodeMinSeparationTime(1500U));
    EXPECT_EQ(0x7F, DoCanParameters::encodeMinSeparationTime(0x7F * 1000U));
    EXPECT_EQ(0x7F, DoCanParameters::encodeMinSeparationTime(0x7F * 1000U + 1500U));
    EXPECT_EQ(0x7F, DoCanParameters::encodeMinSeparationTime(0x80 * 1000U));
    // Test that all decoded values 100-900us correspond to encoded value 0xF1-0xF9
    for (uint32_t value = 1; value <= 9; ++value)
    {
        EXPECT_EQ(value + 0xF0, DoCanParameters::encodeMinSeparationTime(value * 100U));
    }
}

} // anonymous namespace
