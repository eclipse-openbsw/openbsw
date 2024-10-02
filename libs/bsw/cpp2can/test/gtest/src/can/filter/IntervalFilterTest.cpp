// Copyright 2024 Accenture.

#include "can/filter/IntervalFilter.h"

#include "can/canframes/CanId.h"

#include <gmock/gmock.h>

using namespace ::can;
using namespace ::testing;

namespace
{
void verifyRange(IntervalFilter& filter, uint16_t from, uint16_t to)
{
    //[0...from[ must not match
    for (uint16_t id = 0; id < from; ++id)
    {
        ASSERT_TRUE(!filter.match(id));
    }
    //[from...to] must match
    for (uint16_t id = from; id <= to; ++id)
    {
        ASSERT_TRUE(filter.match(id));
    }
    //]to...MAX_ID] must not match
    for (uint16_t id = to + 1; id <= CANFrame::MAX_FRAME_ID; ++id)
    {
        ASSERT_TRUE(!filter.match(id));
    }
}
} // namespace

struct IntervalFilterTest : Test
{
    IntervalFilterTest() { fFilter.clear(); }

    IntervalFilter fFilter;
};

/**
 * \desc
 * After default constructor no id must match.
 * This test is limited to 11 bit CAN ids.
 */
TEST_F(IntervalFilterTest, DefaultConstructor)
{
    IntervalFilter filter;
    for (uint16_t id = 0; id <= CANFrame::MAX_FRAME_ID; ++id)
    {
        ASSERT_TRUE(!filter.match(id));
    }
    uint32_t tmpMaxId = IntervalFilter::MAX_ID;
    ASSERT_EQ(tmpMaxId, filter.getLowerBound());
    ASSERT_EQ(0x0U, filter.getUpperBound());
}

/**
 * \desc
 * The passed ids must match after init constructor.
 * This test only covers 11-bit CAN ids.
 */
TEST_F(IntervalFilterTest, InitConstructor)
{
    // test full range
    IntervalFilter filter1(0x0, CANFrame::MAX_FRAME_ID);
    // check boundaries
    ASSERT_EQ(0x0U, filter1.getLowerBound());
    uint32_t tmpMaxFrameId = CANFrame::MAX_FRAME_ID;
    ASSERT_EQ(tmpMaxFrameId, filter1.getUpperBound());
    verifyRange(filter1, 0x0, CANFrame::MAX_FRAME_ID);

    // test smaller range
    uint16_t from = 0x111;
    uint16_t to   = 0x345;
    IntervalFilter filter2(from, to);
    // check boundaries
    ASSERT_EQ(from, filter2.getLowerBound());
    ASSERT_EQ(to, filter2.getUpperBound());
    verifyRange(filter2, from, to);

    // test over range
    IntervalFilter filter3(0x0, 0x80000000);
    IntervalFilter filter4(0x80000000, 0x90000000);
}

/**
 * \desc
 * Tests the correct behavior when a single id is added.
 * This test only covers 11-bit CAN ids.
 */
TEST_F(IntervalFilterTest, AddId)
{
    uint16_t id1 = 0x00;
    uint16_t id2 = 0xFF;
    uint16_t id3 = 0x6F1;

    // none of the ids must match
    ASSERT_TRUE(!fFilter.match(id1));
    ASSERT_TRUE(!fFilter.match(id2));
    ASSERT_TRUE(!fFilter.match(id3));
    fFilter.add(id1);
    ASSERT_TRUE(fFilter.match(id1));
    ASSERT_TRUE(!fFilter.match(id2));
    ASSERT_TRUE(!fFilter.match(id3));
    // merge second id
    fFilter.add(id2);
    ASSERT_TRUE(fFilter.match(id1));
    ASSERT_TRUE(fFilter.match(id2));
    ASSERT_TRUE(!fFilter.match(id3));

    // merge third id
    fFilter.add(id3);
    verifyRange(fFilter, id1, id3);

    // add maximum possible id
    fFilter.add(CANFrame::MAX_FRAME_ID);
    ASSERT_TRUE(fFilter.match(CANFrame::MAX_FRAME_ID));
    verifyRange(fFilter, id1, CANFrame::MAX_FRAME_ID);

    // test over range
    uint32_t id4 = 0x90000000;
    fFilter.add(id4);
    verifyRange(fFilter, id1, CANFrame::MAX_FRAME_ID);
}

/**
 * \desc
 * Adding a range must result in merging the new range to the existing
 */
TEST_F(IntervalFilterTest, AddRange)
{
    uint16_t from = 0x100;
    uint16_t to   = 0x1FF;
    // nothing of range must be accepted
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(!fFilter.match(i));
    }
    fFilter.add(from, to);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test newFrom < from, newTo == to
    from = 0x90;
    fFilter.add(from, to);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test newFrom == from, newTo > to
    to = 0x2FF;
    fFilter.add(from, to);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test newFrom < from, newTo > to
    from = 0x50;
    to   = 0x3FF;
    fFilter.add(from, to);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test newFrom > from, newTo == to
    fFilter.add(from + 1, to);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test newFrom == from, newTo < to
    fFilter.add(from, to - 1);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test newFrom > from, newTo < to
    fFilter.add(from + 1, to - 1);
    ASSERT_EQ(from, fFilter.getLowerBound());
    ASSERT_EQ(to, fFilter.getUpperBound());
    verifyRange(fFilter, from, to);

    // test if swapping from and to works
    from = 0x600;
    to   = 0x10;
    fFilter.add(from, to);
    ASSERT_EQ(to, fFilter.getLowerBound());
    ASSERT_EQ(from, fFilter.getUpperBound());
    verifyRange(fFilter, to, from);

    // test over range
    uint32_t from2 = 0x10;
    uint32_t to2   = 0xB0000000;
    fFilter.add(from2, to2);
    ASSERT_EQ(from2, fFilter.getLowerBound());
    uint32_t tmpMaxId = IntervalFilter::MAX_ID;
    ASSERT_EQ(tmpMaxId, fFilter.getUpperBound());
    from2 = 0xA0000000;
    fFilter.clear();
    fFilter.add(from2, to2);
    ASSERT_EQ(tmpMaxId, fFilter.getLowerBound());
    fFilter.clear();
}

/**
 * \desc
 * Verification of clear() method
 */
TEST_F(IntervalFilterTest, Clear)
{
    uint16_t id1 = 0x00;
    uint16_t id2 = 0xFF;
    uint16_t id3 = 0x6F1;

    fFilter.add(id1, id2);
    fFilter.add(id3);

    verifyRange(fFilter, id1, id3);

    fFilter.clear();

    for (uint16_t id = 0; id <= CANFrame::MAX_FRAME_ID; ++id)
    {
        ASSERT_TRUE(!fFilter.match(id));
    }
}

/**
 * \desc
 * open() accepts all ids
 */
TEST_F(IntervalFilterTest, Open)
{
    fFilter.clear();

    // nothing must be accepted
    for (uint16_t id = 0; id <= CANFrame::MAX_FRAME_ID; ++id)
    {
        ASSERT_TRUE(!fFilter.match(id));
    }
    fFilter.open();
    // all ids must be accepted
    for (uint16_t id = 0; id <= CANFrame::MAX_FRAME_ID; ++id)
    {
        ASSERT_TRUE(fFilter.match(id));
    }
    ASSERT_TRUE(fFilter.match(IntervalFilter::MAX_ID));
}

/**
 * \desc
 * Check if out of range behaves correctly. All other match functions
 * are checked within the other tests.
 */
TEST_F(IntervalFilterTest, Match) { ASSERT_TRUE(!fFilter.match(IntervalFilter::MAX_ID + 1)); }
