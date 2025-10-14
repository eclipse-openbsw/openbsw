// Copyright 2024 Accenture.

#include "platform/estdint.h"

#include <gtest/gtest.h>

extern "C"
{
size_t get_size_uint8_t();
size_t get_size_uint16_t();
size_t get_size_uint32_t();
size_t get_size_uint64_t();

size_t get_size_int8_t();
size_t get_size_int16_t();
size_t get_size_int32_t();
size_t get_size_int64_t();

size_t get_size_size_t();
size_t get_size_ptrdiff_t();
}

/**
 * \refs    SMD_platform_estdint
 * \desc
 * This test checks that the sizes of the defines types are correct.
 */
TEST(estdint, sizes)
{
    EXPECT_EQ(static_cast<size_t>(1), sizeof(uint8_t));
    EXPECT_EQ(static_cast<size_t>(2), sizeof(uint16_t));
    EXPECT_EQ(static_cast<size_t>(4), sizeof(uint32_t));
    EXPECT_EQ(static_cast<size_t>(8), sizeof(uint64_t));

    EXPECT_EQ(static_cast<size_t>(1), sizeof(int8_t));
    EXPECT_EQ(static_cast<size_t>(2), sizeof(int16_t));
    EXPECT_EQ(static_cast<size_t>(4), sizeof(int32_t));
    EXPECT_EQ(static_cast<size_t>(8), sizeof(int64_t));

    EXPECT_EQ(static_cast<size_t>(1), get_size_uint8_t());
    EXPECT_EQ(static_cast<size_t>(2), get_size_uint16_t());
    EXPECT_EQ(static_cast<size_t>(4), get_size_uint32_t());
    EXPECT_EQ(static_cast<size_t>(8), get_size_uint64_t());

    EXPECT_EQ(static_cast<size_t>(1), get_size_int8_t());
    EXPECT_EQ(static_cast<size_t>(2), get_size_int16_t());
    EXPECT_EQ(static_cast<size_t>(4), get_size_int32_t());
    EXPECT_EQ(static_cast<size_t>(8), get_size_int64_t());

    EXPECT_EQ(sizeof(size_t), sizeof(void*));
    EXPECT_GE(sizeof(size_t), sizeof(ptrdiff_t));

    EXPECT_EQ(sizeof(size_t), get_size_size_t());
    EXPECT_EQ(sizeof(ptrdiff_t), get_size_ptrdiff_t());

    EXPECT_EQ(sizeof(uintptr_t), sizeof(size_t));
    EXPECT_EQ(sizeof(intptr_t), sizeof(ptrdiff_t));
}
