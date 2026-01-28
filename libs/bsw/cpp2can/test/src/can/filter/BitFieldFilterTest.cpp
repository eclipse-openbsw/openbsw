// Copyright 2024 Accenture.

#include "can/filter/BitFieldFilter.h"

#include <gmock/gmock.h>

#include <cstdlib>
#include <ctime>
#include <set>

using namespace can;
using namespace std;
using namespace ::testing;

struct BitFieldFilterTest : Test
{
    BitFieldFilterTest() { fFilter.clear(); }

    BitFieldFilter fFilter;
};

/**
 * \desc
 * verify behaviour with single ids
 */
TEST_F(BitFieldFilterTest, AddId)
{
    uint16_t id1 = 0x00;
    uint16_t id2 = 0xFF;
    uint16_t id3 = 0x6F1;
    uint16_t id4 = 0x7FF;
    ASSERT_TRUE(!fFilter.match(id1));
    fFilter.add(id1);
    ASSERT_TRUE(fFilter.match(id1));

    ASSERT_TRUE(!fFilter.match(id2));
    fFilter.add(id2);
    ASSERT_TRUE(fFilter.match(id1));
    ASSERT_TRUE(fFilter.match(id2));

    ASSERT_TRUE(!fFilter.match(id3));
    fFilter.add(id3);
    ASSERT_TRUE(fFilter.match(id1));
    ASSERT_TRUE(fFilter.match(id2));
    ASSERT_TRUE(fFilter.match(id3));

    ASSERT_TRUE(!fFilter.match(id4));
    fFilter.add(id4);
    ASSERT_TRUE(fFilter.match(id1));
    ASSERT_TRUE(fFilter.match(id2));
    ASSERT_TRUE(fFilter.match(id3));
    ASSERT_TRUE(fFilter.match(id4));

    // test over range
    uint32_t id5 = 0x80000000;
    fFilter.add(id5);
    ASSERT_FALSE(fFilter.match(id5));
    uint32_t id6 = 0x90000000;
    fFilter.add(id6);
    ASSERT_FALSE(fFilter.match(id6));
}

/**
 * \desc
 * verify behaviour with ranges
 */
TEST_F(BitFieldFilterTest, AddRange)
{
    uint16_t from = 0x00;
    uint16_t to   = BitFieldFilter::MAX_ID;

    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(!fFilter.match(i));
    }
    fFilter.add(from, to);
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(fFilter.match(i));
    }
    ASSERT_TRUE(!fFilter.match(to + 1));
}

/**
 * \desc
 * verify merge functionality of BitFieldFilter
 */
TEST_F(BitFieldFilterTest, MergeWithBitFieldFilter)
{
    BitFieldFilter filter1;
    BitFieldFilter filter2;
    BitFieldFilter expectedFilter;

    uint16_t id1 = 0x000;
    uint16_t id2 = 0x001;
    uint16_t id3 = 0x100;
    uint16_t id4 = 0x123;
    uint16_t id5 = 0x234;
    uint16_t id6 = 0x345;

    filter1.add(id1);
    filter1.add(id2);
    filter1.add(id3);

    filter2.add(id3);
    filter2.add(id4);
    filter2.add(id5);
    filter2.add(id6);

    expectedFilter.add(id1);
    expectedFilter.add(id2);
    expectedFilter.add(id3);
    expectedFilter.add(id4);
    expectedFilter.add(id5);
    expectedFilter.add(id6);

    filter1.mergeWithBitField(filter2);

    ASSERT_TRUE(filter1 == expectedFilter);

    ASSERT_TRUE(filter1.match(id1));
    ASSERT_TRUE(filter1.match(id2));
    ASSERT_TRUE(filter1.match(id3));
    ASSERT_TRUE(filter1.match(id4));
    ASSERT_TRUE(filter1.match(id5));
    ASSERT_TRUE(filter1.match(id6));
    ASSERT_TRUE(expectedFilter.match(id1));
    ASSERT_TRUE(expectedFilter.match(id2));
    ASSERT_TRUE(expectedFilter.match(id3));
    ASSERT_TRUE(expectedFilter.match(id4));
    ASSERT_TRUE(expectedFilter.match(id5));
    ASSERT_TRUE(expectedFilter.match(id6));

    uint16_t id7 = 0x456;
    filter1.add(id7);
    ASSERT_FALSE(filter1 == expectedFilter);
}

TEST_F(BitFieldFilterTest, MergeWithIntervalFilter)
{
    IntervalFilter intervalFilter;
    set<uint16_t> randomIds;
    pair<set<uint16_t>::iterator, bool> result;
    // NOLINTNEXTLINE(cert-msc30-c, cert-msc32-c, cert-msc51-cpp): Constant seed is fine.
    srand(0);
    while (randomIds.size() < 100)
    { // generate 100
        // NOLINTNEXTLINE(cert-msc30-c, cert-msc50-cpp): Probably not needed for this test purpose.
        result = randomIds.insert(rand() % BitFieldFilter::MAX_ID + 1);
        fFilter.add(*result.first);
    }
    // assert that they match the filter
    for (set<uint16_t>::iterator itr = randomIds.begin(); itr != randomIds.end(); ++itr)
    {
        ASSERT_TRUE(fFilter.match(*itr));
    }
    // merge an interval
    uint16_t from = 0x100;
    uint16_t to   = 0x234;
    intervalFilter.add(from, to);
    fFilter.mergeWithInterval(intervalFilter);
    // test old ids
    for (set<uint16_t>::iterator itr = randomIds.begin(); itr != randomIds.end(); ++itr)
    {
        ASSERT_TRUE(fFilter.match(*itr));
    }
    // test added interval
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(fFilter.match(i));
    }
    // open filter fully
    intervalFilter.add(0x0, CANFrame::MAX_FRAME_ID);
    fFilter.mergeWithInterval(intervalFilter);
    for (uint16_t i = 0x0; i <= CANFrame::MAX_FRAME_ID; ++i)
    {
        ASSERT_TRUE(fFilter.match(i));
    }
    // open filter fully with extended ID
    intervalFilter.add(0x0, CANFrame::MAX_FRAME_ID_EXTENDED);
    fFilter.mergeWithInterval(intervalFilter);
}

/**
 * \desc
 * Verification of method clear()
 */
TEST_F(BitFieldFilterTest, Clear)
{
    uint16_t from = 0x00;
    uint16_t to   = BitFieldFilter::MAX_ID;
    fFilter.open();
    // assert that all possible ids match
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(fFilter.match(i));
    }
    fFilter.clear();
    // assert that no id matches
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(!fFilter.match(i));
    }
}

/**
 * \desc
 * Verification of method open()
 */
TEST_F(BitFieldFilterTest, Open)
{
    uint16_t from = 0x00;
    uint16_t to   = BitFieldFilter::MAX_ID;

    fFilter.clear();
    // assert that no id matches
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(!fFilter.match(i));
    }
    fFilter.open();
    // assert that all possible ids match
    for (uint16_t i = from; i <= to; ++i)
    {
        ASSERT_TRUE(fFilter.match(i));
    }
}

/**
 * \desc
 * Check if out of range behaves correctly. All other match functions
 * are checked within the other tests.
 */
TEST_F(BitFieldFilterTest, Match) { ASSERT_TRUE(!fFilter.match(BitFieldFilter::MAX_ID + 1)); }

/**
 * \desc
 * Check if out of range behaves correctly. All other match functions
 * are checked within the other tests.
 */
TEST_F(BitFieldFilterTest, getRawBitField)
{
    uint8_t const* rawBitField = fFilter.getRawBitField();
    ASSERT_NE(reinterpret_cast<uint8_t*>(0L), rawBitField);
}
