// Copyright 2024 Accenture.

#include "docan/datalink/DoCanDataLinkAddressPair.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;

TEST(DoCanDataLinkAddressPairTest, testConstruction)
{
    {
        DoCanDataLinkAddressPair<uint32_t> cut;
        EXPECT_FALSE(cut.isValid());
        EXPECT_EQ(0U, cut.getReceptionAddress());
        EXPECT_EQ(0U, cut.getTransmissionAddress());
    }
    {
        DoCanDataLinkAddressPair<uint32_t> cut(1234567U, 102938U);
        EXPECT_TRUE(cut.isValid());
        EXPECT_EQ(1234567U, cut.getReceptionAddress());
        EXPECT_EQ(102938U, cut.getTransmissionAddress());
    }
    {
        DoCanDataLinkAddressPair<uint32_t> cut(1234567U, 1234567U);
        EXPECT_FALSE(cut.isValid());
        EXPECT_EQ(1234567U, cut.getReceptionAddress());
        EXPECT_EQ(1234567U, cut.getTransmissionAddress());
    }
}

TEST(DoCanDataLinkAddressPairTest, testEqOperator)
{
    DoCanDataLinkAddressPair<uint32_t> cut(123U, 456U);
    auto const another = DoCanDataLinkAddressPair<uint32_t>();
    EXPECT_FALSE(another == cut);
    // Test value copy
    cut = another;
    EXPECT_EQ(another, cut);
    // Test assigning itself:
    // This assignment is included in this test so a UB sanitizer can catch the UB if
    // self-assignment does occur
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
    cut = cut;
#pragma GCC diagnostic pop
}

TEST(DoCanDataLinkAddressPairTest, testComparison)
{
    // Use operator== explicitly here
    EXPECT_TRUE(DoCanDataLinkAddressPair<uint32_t>() == DoCanDataLinkAddressPair<uint32_t>());
    EXPECT_TRUE(
        DoCanDataLinkAddressPair<uint32_t>(12345U, 12345U)
        == DoCanDataLinkAddressPair<uint32_t>(12345U, 12345U));
    EXPECT_TRUE(
        DoCanDataLinkAddressPair<uint32_t>(12345U, 83492U)
        == DoCanDataLinkAddressPair<uint32_t>(12345U, 83492U));
    EXPECT_FALSE(
        DoCanDataLinkAddressPair<uint32_t>(1345U, 83492U)
        == DoCanDataLinkAddressPair<uint32_t>(12345U, 83492U));
    EXPECT_FALSE(
        DoCanDataLinkAddressPair<uint32_t>(12345U, 8492U)
        == DoCanDataLinkAddressPair<uint32_t>(12345U, 83492U));
}

} // anonymous namespace
