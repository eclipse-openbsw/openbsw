/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/filter/MaskFilter.h"

#include <gmock/gmock.h>

using namespace ::can;
using namespace ::testing;

struct MaskFilterTest : Test
{
    MaskFilter fFilter;
};

/**
 * \desc
 * After default construction nothing must match.
 */
TEST_F(MaskFilterTest, DefaultConstructor)
{
    ASSERT_FALSE(fFilter.match(0x0U));
    ASSERT_FALSE(fFilter.match(0x7FFU));
    ASSERT_FALSE(fFilter.match(0xFFFFFFFFU));
}

/**
 * \desc
 * Adding a single id must result in an exact match filter.
 */
TEST_F(MaskFilterTest, AddId)
{
    fFilter.add(0x123U);
    ASSERT_TRUE(fFilter.match(0x123U));
    ASSERT_FALSE(fFilter.match(0x122U));
    ASSERT_FALSE(fFilter.match(0x124U));
    ASSERT_EQ(0xFFFFFFFFU, fFilter.getMask());
    ASSERT_EQ(0x123U, fFilter.getPattern());

    // merging a second id widens the mask/pattern to cover both
    fFilter.add(0x120U);
    ASSERT_TRUE(fFilter.match(0x123U));
    ASSERT_TRUE(fFilter.match(0x120U));
    ASSERT_TRUE(fFilter.match(0x121U));
    ASSERT_TRUE(fFilter.match(0x122U));
    ASSERT_FALSE(fFilter.match(0x124U));
    ASSERT_FALSE(fFilter.match(0x11FU));
}

/**
 * \desc
 * Adding a range must result in a mask/pattern block that covers (at least) the whole range.
 */
TEST_F(MaskFilterTest, AddRange)
{
    fFilter.add(0x700U, 0x7FFU);
    for (uint32_t id = 0x700U; id <= 0x7FFU; ++id)
    {
        ASSERT_TRUE(fFilter.match(id));
    }
    ASSERT_FALSE(fFilter.match(0x6FFU));
    ASSERT_FALSE(fFilter.match(0x800U));
    ASSERT_EQ(0xFFFFFF00U, fFilter.getMask());
    ASSERT_EQ(0x700U, fFilter.getPattern());
}

/**
 * \desc
 * Adding a range spanning two blocks that only differ in a single, non-contiguous bit position
 * (as used e.g. to combine the physical/functional addressing formats of ISO 15765-2 normal
 * fixed addressing) must leave all lower bits unconstrained.
 */
TEST_F(MaskFilterTest, AddRangeDifferingInSingleBit)
{
    fFilter.add(0x98DA0000U, 0x98DB0000U);
    ASSERT_EQ(0xFFFE0000U, fFilter.getMask());
    ASSERT_EQ(0x98DA0000U, fFilter.getPattern());

    ASSERT_TRUE(fFilter.match(0x98DA0000U));
    ASSERT_TRUE(fFilter.match(0x98DA1234U));
    ASSERT_TRUE(fFilter.match(0x98DBFFFFU));
    ASSERT_FALSE(fFilter.match(0x98DC0000U));
    ASSERT_FALSE(fFilter.match(0x18DA0000U));
}

/**
 * \desc
 * The order of from/to must not matter.
 */
TEST_F(MaskFilterTest, AddRangeSwapped)
{
    fFilter.add(0x7FFU, 0x700U);
    for (uint32_t id = 0x700U; id <= 0x7FFU; ++id)
    {
        ASSERT_TRUE(fFilter.match(id));
    }
    ASSERT_FALSE(fFilter.match(0x6FFU));
}

/**
 * \desc
 * The range based constructor must behave identically to default construction plus add().
 */
TEST_F(MaskFilterTest, RangeConstructor)
{
    MaskFilter filter(0x700U, 0x7FFU);
    for (uint32_t id = 0x700U; id <= 0x7FFU; ++id)
    {
        ASSERT_TRUE(filter.match(id));
    }
    ASSERT_FALSE(filter.match(0x6FFU));
    ASSERT_FALSE(filter.match(0x800U));
}

/**
 * \desc
 * Verification of clear() method
 */
TEST_F(MaskFilterTest, Clear)
{
    fFilter.add(0x700U, 0x7FFU);
    ASSERT_TRUE(fFilter.match(0x700U));
    fFilter.clear();
    ASSERT_FALSE(fFilter.match(0x700U));
    ASSERT_FALSE(fFilter.match(0x0U));
}

/**
 * \desc
 * open() accepts all ids
 */
TEST_F(MaskFilterTest, Open)
{
    fFilter.open();
    ASSERT_TRUE(fFilter.match(0x0U));
    ASSERT_TRUE(fFilter.match(0x7FFU));
    ASSERT_TRUE(fFilter.match(0xFFFFFFFFU));
}

/**
 * \desc
 * acceptMerger() must call mergeWithMask() on the passed merger.
 */
TEST_F(MaskFilterTest, AcceptMerger)
{
    struct MergerStub : IMerger
    {
        void mergeWithBitField(BitFieldFilter const&) override {}

        void mergeWithStaticBitField(AbstractStaticBitFieldFilter const&) override {}

        void mergeWithInterval(IntervalFilter const&) override {}

        void mergeWithMask(MaskFilter const&) override { called = true; }

        bool called = false;
    };

    MergerStub merger;
    fFilter.acceptMerger(merger);
    ASSERT_TRUE(merger.called);
}
