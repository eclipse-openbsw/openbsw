// Copyright 2024 Accenture.

#include "util/memory/Bit.h"

#include <gmock/gmock.h>

#include <limits>

namespace memory = ::util::memory;

template<typename T>
class BitReverseTest : public ::testing::Test
{};

using BitReverseTypes
    = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t>;

TYPED_TEST_SUITE(BitReverseTest, BitReverseTypes);

TYPED_TEST(BitReverseTest, reverse_bits)
{
    TypeParam value    = static_cast<TypeParam>(0xF0F0F0F0F0F0F0F0ULL);
    TypeParam expected = static_cast<TypeParam>(0x0F0F0F0F0F0F0F0FULL);
    EXPECT_EQ(memory::reverseBits(value), expected);
    EXPECT_EQ(memory::reverseBits(expected), value);

    value    = static_cast<TypeParam>(0x3A3A3A3A3A3A3A3AULL);
    expected = static_cast<TypeParam>(0x5C5C5C5C5C5C5C5CULL);
    EXPECT_EQ(memory::reverseBits(value), expected);
    EXPECT_EQ(memory::reverseBits(expected), value);

    value    = static_cast<TypeParam>((1ULL) << (sizeof(TypeParam) * 8 - 1));
    expected = static_cast<TypeParam>(1);
    EXPECT_EQ(memory::reverseBits(value), expected);
    EXPECT_EQ(memory::reverseBits(expected), value);

    value    = static_cast<TypeParam>((1ULL) << (sizeof(TypeParam) * 8 - 4));
    expected = static_cast<TypeParam>(8);
    EXPECT_EQ(memory::reverseBits(value), expected);
    EXPECT_EQ(memory::reverseBits(expected), value);

    value    = static_cast<TypeParam>((5ULL) << (sizeof(TypeParam) * 8 - 4));
    expected = static_cast<TypeParam>(10);
    EXPECT_EQ(memory::reverseBits(value), expected);
    EXPECT_EQ(memory::reverseBits(expected), value);

    value    = static_cast<TypeParam>((7ULL) << (sizeof(TypeParam) * 8 - 4));
    expected = static_cast<TypeParam>(14);
    EXPECT_EQ(memory::reverseBits(value), expected);
    EXPECT_EQ(memory::reverseBits(expected), value);
}
