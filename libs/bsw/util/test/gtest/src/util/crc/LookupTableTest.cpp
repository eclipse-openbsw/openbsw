// Copyright 2024 Accenture.

#include "util/crc/LookupTable.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::util::crc;
using namespace ::util;
using namespace ::testing;

constexpr size_t TABLE_SIZE = 256UL;

template<class T>
T calcTableEntry(T CRC_POLY, size_t index)
{
    size_t const BITS  = 8UL;
    size_t const WIDTH = BITS * sizeof(T);
    T const TOPBIT     = 1 << (WIDTH - 1UL);

    T entry = index << (WIDTH - 8);

    for (size_t j = 0U; j < BITS; j++)
    {
        if (entry & TOPBIT)
        {
            entry = (entry << 1) ^ CRC_POLY;
        }
        else
        {
            entry <<= 1;
        }
    }

    return entry;
}

TEST(LookupTable, can_be_retrieved)
{
    (void)LookupTable<uint8_t, 0x1D>::getTable();
    (void)LookupTable<uint8_t, 0x07>::getTable();
    (void)LookupTable<uint8_t, 0x2F>::getTable();
    (void)LookupTable<uint8_t, 0x31>::getTable();
    (void)LookupTable<uint8_t, 0xCF>::getTable();
    (void)LookupTable<uint16_t, 0x1021>::getTable();
    (void)LookupTable<uint32_t, 0x4C11DB7>::getTable();
}

TEST(LookupTable, table_0x1D_correct)
{
    LookupTable<uint8_t, 0x1D>::TableRef tableRef = LookupTable<uint8_t, 0x1D>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint8_t>(0x1D, i), tableRef[i]);
    }
}

TEST(LookupTable, table_0x07_correct)
{
    LookupTable<uint8_t, 0x07>::TableRef tableRef = LookupTable<uint8_t, 0x07>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint8_t>(0x07, i), tableRef[i]);
    }
}

TEST(LookupTable, table_0x2F_correct)
{
    LookupTable<uint8_t, 0x2F>::TableRef tableRef = LookupTable<uint8_t, 0x2F>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint8_t>(0x2F, i), tableRef[i]);
    }
}

TEST(LookupTable, table_0x31_correct)
{
    LookupTable<uint8_t, 0x31>::TableRef tableRef = LookupTable<uint8_t, 0x31>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint8_t>(0x31, i), tableRef[i]);
    }
}

TEST(LookupTable, table_0xCF_correct)
{
    LookupTable<uint8_t, 0xCF>::TableRef tableRef = LookupTable<uint8_t, 0xCF>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint8_t>(0xCF, i), tableRef[i]);
    }
}

TEST(LookupTable, table_0x1021_correct)
{
    LookupTable<uint16_t, 0x1021>::TableRef tableRef = LookupTable<uint16_t, 0x1021>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint16_t>(0x1021, i), tableRef[i]);
    }
}

TEST(LookupTable, table_0x4C11DB7_correct)
{
    LookupTable<uint32_t, 0x4C11DB7>::TableRef tableRef
        = LookupTable<uint32_t, 0x4C11DB7>::getTable();

    for (size_t i = 0U; i < TABLE_SIZE; i++)
    {
        EXPECT_EQ(calcTableEntry<uint32_t>(0x4C11DB7, i), tableRef[i]);
    }
}

} // anonymous namespace
