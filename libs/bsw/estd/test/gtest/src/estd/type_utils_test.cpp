// Copyright 2024 Accenture.

#include "estd/type_utils.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

#include <typeinfo>

namespace
{
template<class P>
union CastUnion
{
    P _p;
    uintptr_t _address;
};

TEST(PointerCastTest, IntegerToPointer)
{
    {
        // type with all qualifiers
        CastUnion<uint16_t const volatile*> cast;
        cast._address = 0x213489U;
        EXPECT_EQ(cast._p, ::estd::pointer_from_integer<uint16_t const volatile*>(cast._address));
    }
    {
        // type without any qualifier
        CastUnion<uint32_t*> cast;
        cast._address = 0x213489U;
        EXPECT_EQ(cast._p, ::estd::pointer_from_integer<uint32_t*>(cast._address));
    }
}

TEST(PointerCastTest, PointerToInteger)
{
    {
        // type with all qualifiers
        CastUnion<uint16_t const volatile*> cast;
        cast._address = 0x213489U;
        EXPECT_EQ(cast._address, ::estd::integer_from_pointer(cast._p));
    }
    {
        // type without any qualifier
        CastUnion<uint32_t*> cast;
        cast._address = 0x213489U;
        EXPECT_EQ(cast._address, ::estd::integer_from_pointer(cast._p));
    }
}

template<typename T>
struct EnumTest : ::testing::Test
{};

using EnumUnderlyingTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t, int16_t>;

TYPED_TEST_SUITE(EnumTest, EnumUnderlyingTypes);

/**
 * \desc
 * Tests that to_underlying returns the correct underlying type
 */
TYPED_TEST(EnumTest, to_underlying)
{
    enum class Color : TypeParam
    {
        red,
        green = 20,
        blue,
    };
    auto value = ::estd::to_underlying(Color::green);
    EXPECT_EQ(value, 20);
    EXPECT_EQ(typeid(value), typeid(TypeParam));
}

} // namespace
