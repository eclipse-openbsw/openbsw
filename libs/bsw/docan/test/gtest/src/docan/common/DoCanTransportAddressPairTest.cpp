// Copyright 2024 Accenture.

#include "docan/common/DoCanTransportAddressPair.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;

TEST(DoCanTransportAddressPairTest, testConstructionAndInversion)
{
    {
        DoCanTransportAddressPair cut;
        EXPECT_FALSE(cut.isValid());
        EXPECT_EQ(0U, cut.getSourceId());
        EXPECT_EQ(0U, cut.getTargetId());
        DoCanTransportAddressPair inverted = cut.invert();
        EXPECT_FALSE(inverted.isValid());
        EXPECT_EQ(0U, inverted.getSourceId());
        EXPECT_EQ(0U, inverted.getTargetId());
    }
    {
        DoCanTransportAddressPair cut(12367U, 10238U);
        EXPECT_TRUE(cut.isValid());
        EXPECT_EQ(12367U, cut.getSourceId());
        EXPECT_EQ(10238U, cut.getTargetId());
        DoCanTransportAddressPair inverted = cut.invert();
        EXPECT_TRUE(inverted.isValid());
        EXPECT_EQ(10238U, inverted.getSourceId());
        EXPECT_EQ(12367U, inverted.getTargetId());
    }
    {
        DoCanTransportAddressPair cut(12367U, 12367U);
        EXPECT_FALSE(cut.isValid());
        EXPECT_EQ(12367U, cut.getSourceId());
        EXPECT_EQ(12367U, cut.getTargetId());
        DoCanTransportAddressPair inverted = cut.invert();
        EXPECT_FALSE(inverted.isValid());
        EXPECT_EQ(12367U, inverted.getSourceId());
        EXPECT_EQ(12367U, inverted.getTargetId());
    }
}

TEST(DoCanTransportAddressPairTest, testComparison)
{
    // Use operator== explicitly here
    EXPECT_TRUE(DoCanTransportAddressPair() == DoCanTransportAddressPair());
    EXPECT_TRUE(
        DoCanTransportAddressPair(12345U, 12345U) == DoCanTransportAddressPair(12345U, 12345U));
    EXPECT_TRUE(
        DoCanTransportAddressPair(12345U, 8492U) == DoCanTransportAddressPair(12345U, 8492U));
    EXPECT_FALSE(
        DoCanTransportAddressPair(1345U, 8492U) == DoCanTransportAddressPair(12345U, 8492U));
    EXPECT_FALSE(
        DoCanTransportAddressPair(12345U, 892U) == DoCanTransportAddressPair(12345U, 8492U));
}

} // anonymous namespace
