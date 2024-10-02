// Copyright 2024 Accenture.

#include "estd/bitset.h"

#include "estd/big_endian.h"
#include "estd/little_endian.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>

using namespace ::testing;

namespace
{
template<typename T>
struct BitsetTest : ::testing::Test
{};

template<typename T>
struct IntegralTypeBitsetTest : ::testing::Test
{};

using IntegralBitsetTypes = ::testing::Types<
    ::estd::bitset<5>,
    ::estd::bitset<8>,
    ::estd::bitset<13>,
    ::estd::bitset<16>,
    ::estd::bitset<17>,
    ::estd::bitset<32>,
    ::estd::bitset<47>,
    ::estd::bitset<63>,
    ::estd::bitset<64>>;

using BitsetTypes = ::testing::Types<
    ::estd::bitset<5>,
    ::estd::bitset<8>,
    ::estd::bitset<13>,
    ::estd::bitset<16>,
    ::estd::bitset<17>,
    ::estd::bitset<32>,
    ::estd::bitset<47>,
    ::estd::bitset<63>,
    ::estd::bitset<64>,
    ::estd::bitset<65>,
    ::estd::bitset<66>,
    ::estd::bitset<67>,
    ::estd::bitset<97>>;

TYPED_TEST_SUITE(BitsetTest, BitsetTypes);

TYPED_TEST_SUITE(IntegralTypeBitsetTest, IntegralBitsetTypes);

/**
 * \desc
 * Tests that default constructed bitsets have value zero.
 * \see ::estd::bitset::bitset()
 */
TYPED_TEST(BitsetTest, DefaultConstructor)
{
    TypeParam b;
    EXPECT_EQ(0U, b.value());
    EXPECT_TRUE(b.none());
    EXPECT_FALSE(b.any());
}

/**
 * \desc
 * Tests that value constructed bitsets take that value.
 */
TYPED_TEST(IntegralTypeBitsetTest, ValueConstructor)
{
    TypeParam b(1);
    EXPECT_EQ(1U, b.value());
    EXPECT_FALSE(b.none());
    EXPECT_TRUE(b.any());
}

/**
 * \desc
 * Tests if function test return correct value.
 * \see ::estd::bitset::test()
 */
TYPED_TEST(IntegralTypeBitsetTest, Test)
{
    TypeParam b(9);
    EXPECT_TRUE(b.test(0));
    EXPECT_FALSE(b.test(1));
    EXPECT_FALSE(b.test(2));
    EXPECT_TRUE(b.test(3));
    EXPECT_FALSE(b.test(4));
    for (size_t i = 5; i < b.size(); ++i)
    {
        EXPECT_FALSE(b.test(i));
    }
}

/**
 * \desc
 * Tests if one exclusive bit can be set.
 * \see ::estd::bitset::set()
 */
TYPED_TEST(BitsetTest, SetIndividualBitExclusive)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_FALSE(b.test(i));
        b.set(i);
        EXPECT_TRUE(b.test(i));
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
        }
        b.set(i, false);
        EXPECT_FALSE(b.test(i));
    }
}

TYPED_TEST(BitsetTest, SetIndividualBitAndReadRandomAccessOperator)
{
    TypeParam b;
    TypeParam const& b_ref(b);
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_FALSE(b_ref[i]);
        b.set(i);
        EXPECT_TRUE(b_ref[i]);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b_ref[j]);
            }
        }
        b.set(i, false);
        EXPECT_FALSE(b_ref[i]);
    }
}

TYPED_TEST(BitsetTest, SetOutOfBoundsSet)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    TypeParam b;
    ASSERT_TRUE(b.none());
    ASSERT_THROW({ b.set(b.size()); }, ::estd::assert_exception);
}

TYPED_TEST(BitsetTest, SetOutOfBoundsUnset)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    TypeParam b;
    ASSERT_TRUE(b.none());
    EXPECT_THROW({ b.set(b.size(), false); }, ::estd::assert_exception);
}

/**
 * \desc
 * -    Loops through all valid indices of a bitset
 *  -#  Sets the bit
 *  -#  Tests that this bit is set and all other bits are not
 *  -#  Flips the bitset
 *  -#  Tests that this bit is not set but all others are
 *  -#  Resets the bitset and checks that value() is zero
 */
TYPED_TEST(BitsetTest, SetIndividualBitAndFlipAllAndResetAll)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        b.set(i);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        b.flip();
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_TRUE(b.test(j)) << "@" << i << ", " << j;
            }
            else
            {
                EXPECT_FALSE(b.test(i));
            }
        }
        b.reset();
        EXPECT_EQ(0U, b.value());
    }
}

/**
 * \desc
 * Tests for all possible bit positions that if one bit is
 * set, all others are not set and after flipping the set bit
 * none is set.
 */
TYPED_TEST(BitsetTest, SetAndFlipIndividualBit)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b.none());
        b.set(i);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        b.flip(i);
        EXPECT_TRUE(b.none());
    }
}

TYPED_TEST(BitsetTest, SetAndResetIndividualBit)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b.none());
        b.set(i);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        b.reset(i);
        EXPECT_TRUE(b.none());
    }
}

TYPED_TEST(BitsetTest, SetTrueAndFalseIndividualBit)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b.none());
        b.set(i, true);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        b.set(i, false);
        EXPECT_TRUE(b.none());
    }
}

/**
 * \desc
 * -    Loops through all valid indices of a bitset
 *  -#  Sets the bit
 *  -#  Tests that this bit is set and all other bits are not
 *  -#  Inverts the bitset
 *  -#  Tests that this bit is not set but all others are
 *  -#  Resets the bitset and checks that value() is zero
 */
TYPED_TEST(BitsetTest, SetIndividualBitAndOperatorInvertAndResetAll)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        b.set(i);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        b = ~b;
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_TRUE(b.test(j));
            }
            else
            {
                EXPECT_FALSE(b.test(i));
            }
        }
        b.reset();
        EXPECT_EQ(0U, b.value());
    }
}

TYPED_TEST(BitsetTest, SetAllAndResetAllBits)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_FALSE(b[i]);
    }
    b.set();
    EXPECT_TRUE(b.all());
    EXPECT_TRUE(b.any());
    EXPECT_FALSE(b.none());
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b[i]);
    }
    b.reset();
    EXPECT_FALSE(b.all());
    EXPECT_FALSE(b.any());
    EXPECT_TRUE(b.none());
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_FALSE(b[i]);
    }
}

TYPED_TEST(BitsetTest, FlipAllBits)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_FALSE(b[i]);
    }
    b.flip();
    EXPECT_TRUE(b.all());
    EXPECT_FALSE(b.none());
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b[i]);
    }
    b.flip();
    EXPECT_FALSE(b.all());
    EXPECT_TRUE(b.none());
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_FALSE(b[i]);
    }
}

/**
 * \desc
 * Tests that bit references can be used as bool.
 */
TYPED_TEST(BitsetTest, ReferenceOperatorBool)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        typename TypeParam::reference r = b[i];
        EXPECT_FALSE(r);
    }
    b.flip();
    for (size_t i = 0; i < b.size(); ++i)
    {
        typename TypeParam::reference r = b[i];
        EXPECT_TRUE(r);
    }
}

/**
 * \desc
 * Tests that every bit reference can be set to true and false.
 * By checking the bitmask, too, it is assured that r is indeed
 * a reference to a bit inside the bitset.
 */
TYPED_TEST(BitsetTest, ReferenceAssignmentOperator)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b.none());
        typename TypeParam::reference r = b[i];
        r                               = true;
        EXPECT_TRUE(r);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        r = false;
        EXPECT_FALSE(r);
        for (size_t j = 0; j < b.size(); ++j)
        {
            EXPECT_FALSE(b.test(i));
        }
    }
}

/**
 * \desc
 * Tests that flipping a bit reference inverts its value.
 */
TYPED_TEST(BitsetTest, ReferenceFlip)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b.none());
        typename TypeParam::reference r = b[i];
        r.flip();
        EXPECT_TRUE(r);
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                EXPECT_FALSE(b.test(j));
            }
            else
            {
                EXPECT_TRUE(b.test(i));
            }
        }
        r.flip();
        EXPECT_FALSE(r);
    }
}

/**
 * \desc
 * Tests assignment of one bit reference to another.
 */
TYPED_TEST(BitsetTest, ReferenceCopyAssignmentOperator)
{
    TypeParam b;
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_TRUE(b.none());
        typename TypeParam::reference r(b[i]);
        r = true;
        for (size_t j = 0; j < b.size(); ++j)
        {
            if (i != j)
            {
                typename TypeParam::reference r2 = b[j];
                EXPECT_FALSE(r2);
                r2 = r;
                EXPECT_TRUE(r2);
            }
        }
        EXPECT_TRUE(b.all());
        b.reset();
    }
}

/*
 * Sets every nth bit of the actual bitset within the range of the size of the bitset. Checks if the
 * number returned by the count() function is correct.
 */
TYPED_TEST(BitsetTest, Count)
{
    TypeParam b;
    ASSERT_EQ(0U, b.count());

    for (size_t i = 1; i < b.size(); ++i)
    {
        size_t ones = 0;
        b.reset();
        ASSERT_EQ(0, b.count());
        for (size_t index = 0; index < b.size(); index += i)
        {
            b.set(index);
            ones += 1;
        }
        ASSERT_EQ(ones, b.count());
    }
}

TYPED_TEST(BitsetTest, OperatorOrAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    ::estd::array<
        typename TypeParam::value_type const,
        ::estd::internal::type_for_size<TypeParam::SIZE>::N> const v1
        = {{0x1}};
    b |= TypeParam(v1);
    ASSERT_EQ(1U, b.value());
    ::estd::array<
        typename TypeParam::value_type const,
        ::estd::internal::type_for_size<TypeParam::SIZE>::N> const v2
        = {{0x2}};
    TypeParam c(v2);
    b |= c;
    ASSERT_EQ(3U, b.value());
}

TYPED_TEST(IntegralTypeBitsetTest, OperatorOrAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    b |= 1;
    ASSERT_EQ(1U, b.value());
    TypeParam c(2);
    b |= c;
    ASSERT_EQ(3U, b.value());
}

TYPED_TEST(IntegralTypeBitsetTest, OperatorAndAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    b &= 1;
    ASSERT_EQ(0U, b.value());
    b = 3;
    TypeParam c(2);
    b &= c;
    ASSERT_EQ(2U, b.value());
}

TYPED_TEST(BitsetTest, OperatorAndAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    typename TypeParam::value_type const v1[::estd::internal::type_for_size<TypeParam::SIZE>::N]
        = {0x1};
    b &= TypeParam(v1);
    ASSERT_EQ(0U, b.value());
    typename TypeParam::value_type const v3[::estd::internal::type_for_size<TypeParam::SIZE>::N]
        = {0x3};
    b = TypeParam(v3);
    typename TypeParam::value_type const v2[::estd::internal::type_for_size<TypeParam::SIZE>::N]
        = {0x2};
    TypeParam c(v2);
    b &= c;
    ASSERT_EQ(2U, b.value());
}

TYPED_TEST(IntegralTypeBitsetTest, OperatorXorAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    b ^= 1;
    ASSERT_EQ(1U, b.value());
    TypeParam c(3);
    b ^= c;
    ASSERT_EQ(2U, b.value());
}

TYPED_TEST(BitsetTest, OperatorXorAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    ::estd::
        array<typename TypeParam::value_type, ::estd::internal::type_for_size<TypeParam::SIZE>::N>
            v1 = {{0x1}};
    b ^= TypeParam(v1);
    ASSERT_EQ(1U, b.value());
    ::estd::
        array<typename TypeParam::value_type, ::estd::internal::type_for_size<TypeParam::SIZE>::N>
            v3 = {{0x3}};
    TypeParam c(v3);
    b ^= c;
    ASSERT_EQ(2U, b.value());
}

/*
 * Checks if the shift right assignment operator works correctly with integral_bitset types.
 * Shifts a logical 1 from MSB to LSB, by zero, shifts it out and asserts the results.
 */
TYPED_TEST(IntegralTypeBitsetTest, OperatorShiftRightAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    b >>= 1;
    ASSERT_EQ(0U, b.value());
    b.set(b.size() - 1);
    b >>= (b.size() - 1);
    ASSERT_EQ(1, b.value());
    b >>= 0;
    ASSERT_EQ(1, b.value());
    b >>= 1;
    ASSERT_EQ(0, b.value());
}

/*
 * Checks if the shift right operator works correctly with integral_bitset types.
 * Shifts a logical 1 from MSB to LSB, by zero, shifts it out and asserts the results.
 */
TYPED_TEST(IntegralTypeBitsetTest, OperatorShiftRight)
{
    TypeParam b1;
    TypeParam b2;
    ASSERT_EQ(0U, b1.value());
    ASSERT_EQ(0U, b2.value());
    b2 = (b1 >> 1);
    ASSERT_EQ(0U, b2.value());
    b1.set(b1.size() - 1);
    b2 = b1 >> (b1.size() - 1);
    ASSERT_EQ(1, b2.value());
    b2 = (b2 >> 0);
    ASSERT_EQ(1, b2.value());
    b2 = (b2 >> 1);
    ASSERT_EQ(0, b2.value());
}

/*
 * Checks if the shift right assignment operator works correctly with integral_bitset and
 * array_bitset types. Shifts a logical 1 from MSB to LSB through every position and asserts the
 * results, also checks for incorrectly remaining logical 1 bits.
 */
TYPED_TEST(BitsetTest, OperatorShiftRightAssignment)
{
    TypeParam b;

    b.set(b.size() - 1);
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_TRUE(b[b.size() - i - 1]);
        // Reset to check if only one bit remains set to 1 after shifting
        b.reset(b.size() - i - 1);
        ASSERT_TRUE(b.none());
        b.set(b.size() - i - 1);
        b >>= 1;
    }
}

/*
 * Checks if the shift right operator works correctly with integral_bitset and
 * array_bitset types. Shifts a logical 1 from MSB to LSB through every position and asserts the
 * results, also checks for incorrectly remaining logical 1 bits.
 */
TYPED_TEST(BitsetTest, OperatorShiftRight)
{
    TypeParam b;

    b.set(b.size() - 1);
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_TRUE(b[b.size() - i - 1]);
        // Reset to check if only one bit remains set to 1 after shifting
        b.reset(b.size() - i - 1);
        ASSERT_TRUE(b.none());
        b.set(b.size() - i - 1);
        b = (b >> 1);
    }
}

/*
 * Checks if the shift left assignment operator works correctly with integral_bitset types.
 * Shifts a logical 1 from LSB to MSB, by zero, shifts it out and asserts the results.
 */
TYPED_TEST(IntegralTypeBitsetTest, OperatorShiftLeftAssignment)
{
    TypeParam b;
    ASSERT_EQ(0U, b.value());
    b <<= 1;
    ASSERT_EQ(0U, b.value());
    b = 1;
    b <<= (b.size() - 1);
    ASSERT_TRUE(b[b.size() - 1]);
    b <<= 0;
    ASSERT_TRUE(b[b.size() - 1]);
    b <<= 1;
    ASSERT_EQ(0U, b.value());
}

/*
 * Checks if the shift left operator works correctly with integral_bitset types.
 * Shifts a logical 1 from LSB to MSB, by zero, shifts it out and asserts the results.
 */
TYPED_TEST(IntegralTypeBitsetTest, OperatorShiftLeft)
{
    TypeParam b1;
    TypeParam b2;
    ASSERT_EQ(0U, b1.value());
    ASSERT_EQ(0U, b2.value());
    b2 = (b1 << 1);
    ASSERT_EQ(0U, b2.value());
    b1 = 1;
    b2 = b1 << (b1.size() - 1);
    ASSERT_TRUE(b2[b1.size() - 1]);
    b2 = (b2 << 0);
    ASSERT_TRUE(b2[b1.size() - 1]);
    b2 = (b2 << 1);
    ASSERT_EQ(0U, b2.value());
}

/*
 * Checks if the shift left assignment operator works correctly with integral_bitset and
 * array_bitset types. Shifts a logical 1 from LSB to MSB through every position and asserts the
 * results, also checks for incorrectly remaining logical 1 bits.
 */
TYPED_TEST(BitsetTest, OperatorShiftLeftAssignment)
{
    TypeParam b;

    b.set(0);
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_TRUE(b[i]);
        // Reset to check if only one bit remains set to 1 after shifting
        b.reset(i);
        ASSERT_TRUE(b.none());
        b.set(i);
        b <<= 1;
    }
}

/*
 * Checks if the shift left operator works correctly with integral_bitset and
 * array_bitset types. Shifts a logical 1 from LSB to MSB through every position and asserts the
 * results, also checks for incorrectly remaining logical 1 bits.
 */
TYPED_TEST(BitsetTest, OperatorShiftLeft)
{
    TypeParam b;

    b.set(0);
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_TRUE(b[i]);
        // Reset to check if only one bit remains set to 1 after shifting
        b.reset(i);
        ASSERT_TRUE(b.none());
        b.set(i);
        b = (b << 1);
    }
}

TYPED_TEST(BitsetTest, OperatorEqual)
{
    TypeParam b, c;
    ASSERT_EQ(b, c);
    b.flip();
    ASSERT_NE(b, c);
}

TYPED_TEST(BitsetTest, BitOperators)
{
    TypeParam a, b, c;
    a = b & c;
    ASSERT_EQ(0U, a.value());
    b.flip();
    a = b & c;
    ASSERT_TRUE(a.none());
    a = b | c;
    ASSERT_TRUE(a.all());
    a = b ^ c;
    ASSERT_TRUE(a.all());
    c.flip();
    a = b ^ c;
    ASSERT_TRUE(a.none());
}

TYPED_TEST(IntegralTypeBitsetTest, FlipOutOfBounds)
{
    TypeParam b(9);
    TypeParam c(b);
    ASSERT_EQ(b, c);
    b.flip(b.size());
    ASSERT_EQ(b, c);
}

TEST(Bitset, ZeroSizedBitset)
{
    ::estd::bitset<0> b;
    ASSERT_TRUE(b.none());
    ASSERT_TRUE(b.all());
    ASSERT_FALSE(b.any());
    ASSERT_FALSE(b.test(0));
    ASSERT_EQ(0, b.value());
    b.set(0);
    ASSERT_EQ(0, b.value());
    b.set();
    ASSERT_EQ(0, b.value());
    b.reset();
    ASSERT_EQ(0, b.value());
    b.reset(0);
    ASSERT_EQ(0, b.value());
    b.flip();
    ASSERT_EQ(0, b.value());
    ::estd::bitset<0> c(3);
    ASSERT_EQ(0, c.value());
    ASSERT_EQ(b, c);
    b &= c;
    ASSERT_EQ(0, b.value());
    b |= c;
    ASSERT_EQ(0, b.value());
    b ^= c;
    ASSERT_EQ(0, b.value());
    ::estd::bitset<0>::reference r = b[0];
    ASSERT_FALSE(r);
    ASSERT_TRUE(~r);
    r = true;
    ASSERT_FALSE(r);
    ASSERT_TRUE(~r);
    r = false;
    ASSERT_FALSE(r);
    ASSERT_TRUE(~r);
    r.flip();
    ASSERT_FALSE(r);
    ASSERT_TRUE(~r);
    ::estd::bitset<0>::reference s = b[1];
    r                              = s;
    ASSERT_FALSE(r);
    ASSERT_TRUE(~r);
}

TEST(Bitset, ArrayConstructor)
{
    {
        ::estd::array<uint8_t, 1> v = {
            {0xA5} // 1010 0101
        };
        ::estd::bitset<1> b1(v);
        EXPECT_EQ(1, b1.value());
        EXPECT_THAT(b1.to_slice(), ElementsAre(1));

        ::estd::bitset<2> b2(v);
        EXPECT_EQ(1, b2.value());
        EXPECT_THAT(b2.to_slice(), ElementsAre(1));

        ::estd::bitset<3> b3(v);
        EXPECT_EQ(5, b3.value());
        EXPECT_THAT(b3.to_slice(), ElementsAre(5));

        ::estd::bitset<4> b4(v);
        EXPECT_EQ(5, b4.value());
        EXPECT_THAT(b4.to_slice(), ElementsAre(5));
    }
    {
        // 1111 1111 1111 1111 0000 0000 0000 0000 0000 0000 1111 1111 0000 0000 1111 1111
        ::estd::array<uint64_t const, 1> v = {{0x00FF00FFFFFF0000ULL}};
        ::estd::bitset<33> const b33(v);
        for (size_t i = 0; i < 15; ++i)
        {
            EXPECT_FALSE(b33[i]) << "at pos " << i;
        }
        for (size_t i = 16; i < 32; ++i)
        {
            EXPECT_TRUE(b33[i]) << "at pos " << i;
        }
        EXPECT_THAT(b33.to_slice(), ElementsAre(0x1FFFF0000U));
    }
    {
        ::estd::bitset<75> b;
        b.set(13);
        b.set(19);
        b.set(74);

        ::estd::bitset<75> b2(b.to_slice());
        for (size_t i = 0; i < 75; ++i)
        {
            if (i == 13 || i == 19 || i == 74)
            {
                EXPECT_TRUE(b2.test(i)) << "at pos " << i;
            }
            else
            {
                EXPECT_FALSE(b2.test(i)) << "at pos " << i;
            }
        }
    }
}

TEST(Bitset, ArrayAssignment)
{
    {
        ::estd::array<uint8_t, 1> v = {
            {0xA5} // 1010 0101
        };
        ::estd::bitset<1> b1;
        b1.assign(v);
        EXPECT_EQ(1, b1.value());

        ::estd::bitset<2> b2;
        b2.assign(v);
        EXPECT_EQ(1, b2.value());

        ::estd::bitset<3> b3;
        b3.assign(v);
        EXPECT_EQ(5, b3.value());

        ::estd::bitset<4> b4;
        b4.assign(v);
        EXPECT_EQ(5, b4.value());

        ::estd::bitset<5> b5;
        b5.assign(v);
        EXPECT_EQ(5, b5.value());

        ::estd::bitset<6> b6;
        b6.assign(v);
        EXPECT_EQ(37, b6.value());

        ::estd::bitset<7> b7;
        b7.assign(v);
        EXPECT_EQ(37, b7.value());

        ::estd::bitset<8> b8;
        b8.assign(v);
        EXPECT_EQ(165, b8.value());
    }
    {
        // 1111 1111 1111 1111 0000 0000 0000 0000 0000 0000 1111 1111 0000 0000 1111 1111
        ::estd::array<uint64_t const, 1> v = {{0x00FF00FFFFFF0000ULL}};
        ::estd::bitset<33> b33;
        b33.assign(v);
        for (size_t i = 0; i < 16; ++i)
        {
            EXPECT_FALSE(b33[i]);
        }
        for (size_t i = 16; i < 32; ++i)
        {
            EXPECT_TRUE(b33[i]);
        }
    }
    {
        ::estd::bitset<75> b;
        b.set(5);
        b.set(55);

        ::estd::bitset<75> b2;
        b2.assign(b.to_slice());
        for (size_t i = 0; i < 75; ++i)
        {
            if (i == 5 || i == 55)
            {
                EXPECT_TRUE(b2.test(i)) << "at pos " << i;
            }
            else
            {
                EXPECT_FALSE(b2.test(i)) << "at pos " << i;
            }
        }
    }
}

/**
 * \desc
 * Verifies that a byte array (not aligned to uint32) can be written to an integral bitset, without
 * UBSan errors.
 */
TEST(Bitset, FromBytesTestUnalignedIntegralBitset)
{
    alignas(uint32_t)::estd::array<uint8_t const, 5> array = {{0xF0, 0xB4, 0xC3, 0xD2, 0xE1}};

    // Make unaligned from uint32_t, throw away 0xF0
    // 1111 0000 1011 0100, 1100 0011, 1101 0010, 1110 0001
    //   Throw    index 0    index 1    index 2    index 3
    auto unalignedSlice = ::estd::slice<uint8_t const>(array).advance(1);
    // One offset from uint32 alignment
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(unalignedSlice.data()) % alignof(uint32_t), 1);

    auto slice = unalignedSlice.subslice(1);
    ::estd::bitset<4> bitset4;
    bitset4.from_bytes(slice);
    EXPECT_EQ(unalignedSlice[0] & 0x0F, bitset4.value());

    ::estd::bitset<8> bitset8;
    bitset8.from_bytes(slice);
    EXPECT_EQ(unalignedSlice[0], bitset8.value());

    auto slice2 = unalignedSlice.subslice(2);
    ::estd::bitset<12> bitset12;
    bitset12.from_bytes(slice2);
    ::estd::memory::unaligned<uint16_t> expected_uint16;
    expected_uint16.set(0);
    expected_uint16.bytes[0] = unalignedSlice[0];
    expected_uint16.bytes[1] = unalignedSlice[1] & 0x0F;
    EXPECT_EQ(expected_uint16.get(), bitset12.value());

    ::estd::bitset<20> bitset20;
    bitset20.from_bytes(unalignedSlice);
    ::estd::memory::unaligned<uint32_t> expected_uint32;
    expected_uint32.set(0);
    expected_uint32.bytes[0] = unalignedSlice[0];
    expected_uint32.bytes[1] = unalignedSlice[1];
    expected_uint32.bytes[2] = unalignedSlice[2] & 0x0F;
    EXPECT_EQ(expected_uint32.get(), bitset20.value());

    ::estd::bitset<28> bitset28;
    bitset28.from_bytes(unalignedSlice);
    expected_uint32.bytes[2] = unalignedSlice[2];
    expected_uint32.bytes[3] = unalignedSlice[3] & 0x0F;
    EXPECT_EQ(expected_uint32.get(), bitset28.value());

    ::estd::bitset<28> bitset28_1_byte;
    bitset28_1_byte.from_bytes(slice);
    EXPECT_EQ(unalignedSlice[0], bitset28_1_byte.value());
}

/**
 * \desc
 * Verifies that a byte array (not aligned to uint32) can be written to an array bitset, without
 * UBSan errors.
 */
TEST(Bitset, FromBytesTestUnalignedArrayBitset)
{
    alignas(uint32_t)::estd::array<uint8_t const, 10> array
        = {{0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87, 0x78, 0x69}};

    // Make unaligned from uint32_t, throw away 0xF0
    // 1111 0000 1011 0100, 1100 0011, 1101 0010, 1110 0001
    //   Throw    index 0    index 1    index 2    index 3
    auto unalignedSlice = ::estd::slice<uint8_t const>(array).advance(1);
    // One offset from uint32 alignment
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(unalignedSlice.data()) % alignof(uint32_t), 1);

    ::estd::bitset<68> bitset68;
    bitset68.from_bytes(unalignedSlice);
    auto data = bitset68.to_slice().reinterpret_as<uint8_t const>();
    for (size_t i = 0; i < unalignedSlice.size() - 1; i++)
    {
        EXPECT_EQ(data[i], unalignedSlice[i]);
    }
    EXPECT_EQ(data[unalignedSlice.size() - 1], unalignedSlice[unalignedSlice.size() - 1] & 0x0F);
}

/**
 * \desc
 * Verifies that when a bitset is set to a byte array which has fewer bits than the bitset itself,
 * any remaining bits in the bitset will be reset to 0
 */
TEST(Bitset, FromBytesTestRemainingIsReset)
{
    ::estd::array<uint8_t const, 9> array
        = {{0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87, 0x78}};
    auto slice = ::estd::slice<uint8_t const>(array).advance(1);

    ::estd::bitset<127> bitset127;
    bitset127.set();
    bitset127.from_bytes(slice);
    auto data = bitset127.to_slice().reinterpret_as<uint8_t const>();
    for (size_t i = 0; i < slice.size(); i++)
    {
        EXPECT_EQ(data[i], slice[i]);
    }
    for (size_t i = slice.size(); i < data.size(); i++)
    {
        EXPECT_EQ(data[i], 0);
    }
}

/**
 * \desc
 * Verifies that a bitset, with its bits set by from_bytes, will have each bit set correctly in
 * order.
 */
TEST(Bitset, FromBytesTestBits)
{
    // 1111 1111, x4...
    // 1010 1010, x4...
    // 0000 0000, x4...
    ::estd::array<uint64_t, 1> array = {0x00000000AAAAAAAAFFFFFFFF};

    auto slice = ::estd::slice<uint64_t const>(array).reinterpret_as<uint8_t const>();

    ::estd::bitset<75> b75;
    b75.from_bytes(slice);
    for (size_t i = 0; i < 32; ++i)
    {
        EXPECT_TRUE(b75[i]);
    }
    for (size_t i = 32; i < 64; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(b75[i]);
        }
        else
        {
            EXPECT_TRUE(b75[i]);
        }
    }
    for (size_t i = 64; i < 75; ++i)
    {
        EXPECT_FALSE(b75[i]);
    }
}

TEST(Bitset, ValueTooLarge)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::bitset<65> b;
    b.flip();

    ASSERT_THROW({ b.value(); }, ::estd::assert_exception);
}

TEST(Bitset, Set)
{
    ::estd::bitset<8> m;
    m.set(0);
    EXPECT_EQ(1U, m.value());
    m.set(1).set(2);
    EXPECT_EQ(7U, m.value());
    m.set(1, false);
    EXPECT_EQ(5U, m.value());
}

TEST(Bitset, SetUsingIndexing)
{
    ::estd::bitset<8> m;
    m[0] = true;
    EXPECT_EQ(1U, m.value());
    m[1] = m[2] = true;
    EXPECT_EQ(7U, m.value());
    m[1] = false;
    EXPECT_EQ(5U, m.value());
}

TEST(Bitset, SizeNotPowerOfTwo)
{
    ::estd::bitset<9> m;
    m.set(0);
    EXPECT_EQ(1U, m.value());

    m.reset();
    m.set(8);
    EXPECT_EQ(256U, m.value());

    m = ::estd::bitset<9>(uint16_t(~0U));

    EXPECT_TRUE(m[8]);
    EXPECT_FALSE(m[9]);
    EXPECT_EQ(511, m.value());
}

TEST(Bitset, Test)
{
    ::estd::bitset<8> m;
    EXPECT_FALSE(m.test(0));
    m.set(0);
    EXPECT_TRUE(m.test(0));
}

TEST(Bitset, ResetAll)
{
    ::estd::bitset<8> m(10);
    EXPECT_EQ(10U, m.value());
    m.reset();
    EXPECT_EQ(0U, m.value());
}

TEST(Bitset, ResetBit)
{
    ::estd::bitset<8> m(12); // 1100
    m.reset(2);
    EXPECT_EQ(8U, m.value());
}

TEST(Bitset, Any)
{
    ::estd::bitset<8> m(12);
    EXPECT_TRUE(m.any());
    m.reset();
    EXPECT_FALSE(m.any());
}

TEST(Bitset, All)
{
    ::estd::bitset<8> m(12);
    EXPECT_FALSE(m.all());
    m.set();
    EXPECT_TRUE(m.all());
}

TEST(Bitset, None)
{
    ::estd::bitset<8> m(12);
    EXPECT_FALSE(m.none());
    m.reset();
    EXPECT_TRUE(m.none());
}

TEST(Bitset, OperatorEQ)
{
    ::estd::bitset<8> m(12);
    EXPECT_TRUE(m == 12);
    EXPECT_FALSE(m == 13);
}

TEST(Bitset, OperatorNE)
{
    ::estd::bitset<8> m(12);
    EXPECT_FALSE(m != 12);
    EXPECT_TRUE(m != 13);
}

TEST(Bitset, OperatorAnd)
{
    ::estd::bitset<8> m(12);
    ::estd::bitset<8> other(8);
    EXPECT_TRUE((m & other) == other);
}

TEST(Bitset, OperatorXOR)
{
    ::estd::bitset<16> a(0x0fff);
    ::estd::bitset<16> b(0xf0ff);
    EXPECT_EQ(0xff00, (a ^ b).value());
}

TEST(Bitset, Bitmask_works_for_32_bits)
{
    ::estd::bitset<32> b32(0xffff);
    EXPECT_EQ(0xffffU, b32.value());
}

TEST(Bitset, SizeAndSizeOf)
{
    ::estd::bitset<0> b0;
    EXPECT_EQ(0U, b0.size());
    EXPECT_EQ(1U, sizeof(b0));

    ::estd::bitset<32> b32;
    EXPECT_EQ(32U, b32.size());
    EXPECT_EQ(4U, sizeof(b32));

    ::estd::bitset<8> b8;
    EXPECT_EQ(8U, b8.size());
    EXPECT_EQ(1U, sizeof(b8));

    ::estd::bitset<30> b30;
    EXPECT_EQ(30U, b30.size());
    EXPECT_EQ(4U, sizeof(b30));

    ::estd::bitset<17> b17;
    EXPECT_EQ(17U, b17.size());
    EXPECT_EQ(4U, sizeof(b17));

    ::estd::bitset<47> b47;
    EXPECT_EQ(47U, b47.size());
    EXPECT_EQ(8U, sizeof(b47));

    ::estd::bitset<67> b67;
    EXPECT_EQ(67U, b67.size());
    EXPECT_EQ(12U, sizeof(b67));
}

/**
 * \desc
 * This test was added for the observed bug that when resetting a bit
 * in the last byte of the mask, all bits were reset.
 */
TEST(Bitset, ResetBitInLastByte)
{
    ::estd::bitset<128> bitset;
    bitset.set();
    for (size_t i = 0U; i < 128U; ++i)
    {
        EXPECT_TRUE(bitset.test(i)) << "@ " << i;
    }
    bitset.reset(126U);
    for (size_t i = 0U; i < 126U; ++i)
    {
        EXPECT_TRUE(bitset.test(i)) << "@ " << i;
    }
    for (size_t i = 127U; i < 128U; ++i)
    {
        EXPECT_TRUE(bitset.test(i)) << "@ " << i;
    }
}

/*
 * Checks if the shift right assignment operator works correctly with a 128 bit bitset when it is
 * shifted by 0.
 */
TEST(Bitset, RightShiftByZeroAssign)
{
    ::estd::bitset<128> b;
    ASSERT_EQ(0U, b.value());
    b >>= 0;
    ASSERT_EQ(0U, b.value());
    b.set(31);
    b.set(63);
    b.set(95);
    b.set(127);
    b >>= 0;
    auto slice = b.to_slice();
    ASSERT_EQ(0x80000000U, slice[0]);
    ASSERT_EQ(0x80000000U, slice[1]);
    ASSERT_EQ(0x80000000U, slice[2]);
    ASSERT_EQ(0x80000000U, slice[3]);
}

/*
 * Checks if the shift left assignment operator works correctly with a 128 bit bitset when it is
 * shifted by 0.
 */
TEST(Bitset, LeftShiftByZeroAssign)
{
    ::estd::bitset<128> b;
    ASSERT_EQ(0U, b.value());
    b <<= 0;
    ASSERT_EQ(0U, b.value());
    b.set(31);
    b.set(63);
    b.set(95);
    b.set(127);
    b <<= 0;
    auto slice = b.to_slice();
    ASSERT_EQ(0x80000000U, slice[0]);
    ASSERT_EQ(0x80000000U, slice[1]);
    ASSERT_EQ(0x80000000U, slice[2]);
    ASSERT_EQ(0x80000000U, slice[3]);
}

/*
 * Checks if the shift right assignment operator works correctly with a 128 bit bitset. Shifts
 * 0x55555555 through the bitset, which will be 0x55555555 when shifted by an even number and
 * 0xAAAAAAAA when shifted by an odd number except at the actual highest index block, which is
 *  continuously shifted out.
 */
TEST(Bitset, RightShiftAssignArray)
{
    uint32_t data[]                = {0x55555555, 0x55555555, 0x55555555, 0x55555555};
    uint32_t const oddShiftResult  = 0xAAAAAAAA;
    uint32_t const evenShiftResult = 0x55555555;

    for (size_t i = 0; i < sizeof(data) * 8; ++i)
    {
        ::estd::bitset<sizeof(data) * 8> test;
        test.assign(data);
        test >>= i;
        auto const values       = test.to_slice();
        uint32_t shiftMaskCount = i;
        // Reverse iterate values because a right shift will clear the highest index bits first
        for (auto iter = values.rbegin(); iter != values.rend(); ++iter)
        {
            uint32_t const value        = *iter;
            // Shift up to 32 bits off the result, preserving the rest of the count for the next
            // value in the bitset array
            uint32_t const currentShift = std::min<uint32_t>(shiftMaskCount, 32);
            // Calculate the mask for the current value
            uint32_t const shiftMask
                = (currentShift == 32) ? 0 : std::numeric_limits<uint32_t>::max() >> currentShift;
            // Subtract however many "shifts" we "used" on this value
            shiftMaskCount -= currentShift;
            auto const shiftResult = (i % 2 == 0) ? evenShiftResult : oddShiftResult;
            EXPECT_EQ(value, shiftResult & shiftMask);
        }
    }
}

/*
 * Checks if the shift left assignment operator works correctly with a 128 bit bitset. Shifts
 * 0x55555555 through the bitset, which will be 0x55555555 when shifted by an even number and
 * 0xAAAAAAAA when shifted by an odd number except at the actual lowest index block, which is
 *  continuously shifted out.
 */
TEST(Bitset, LeftShiftAssignArray)
{
    uint32_t data[]                = {0x55555555, 0x55555555, 0x55555555, 0x55555555};
    uint32_t const oddShiftResult  = 0xAAAAAAAA;
    uint32_t const evenShiftResult = 0x55555555;

    for (size_t i = 0; i < sizeof(data) * 8; ++i)
    {
        ::estd::bitset<sizeof(data) * 8> test;
        test.assign(data);
        test <<= i;
        auto const values       = test.to_slice();
        uint32_t shiftMaskCount = i;
        // Iterate values because a left shift will clear the lowest index bits first
        for (auto iter = values.begin(); iter != values.end(); ++iter)
        {
            uint32_t const value        = *iter;
            // Shift up to 32 bits off the result, preserving the rest of the count for the next
            // value in the bitset array
            uint32_t const currentShift = std::min<uint32_t>(shiftMaskCount, 32);
            // Calculate the mask for the current value
            uint32_t const shiftMask
                = (currentShift == 32) ? 0 : std::numeric_limits<uint32_t>::max() << currentShift;
            // Subtract however many "shifts" we "used" on this value
            shiftMaskCount -= currentShift;
            auto const shiftResult = (i % 2 == 0) ? evenShiftResult : oddShiftResult;
            EXPECT_EQ(value, shiftResult & shiftMask);
        }
    }
}

} // anonymous namespace
