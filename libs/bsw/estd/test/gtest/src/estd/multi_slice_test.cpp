// Copyright 2024 Accenture.

#include "estd/multi_slice.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
using namespace ::testing;

using MS816 = ::estd::multi_slice<uint8_t, uint16_t const>;

uint8_t u8_array[10U]         = {0};
uint16_t const u16_array[10U] = {0}; // const to make sure multi_slice is usable with const as well

// Check that conditional return type is correct in all cases, meaning is non-const if and
// only if both the requested type as well as the listed type are non-const
static_assert(
    std::is_same<
        ::estd::internal::return_T<::estd::internal::TL_id<MS816::types, uint8_t>>::type,
        uint8_t>::value,
    "");
static_assert(
    std::is_same<
        ::estd::internal::return_T<::estd::internal::TL_id<MS816::types, uint8_t const>>::type,
        uint8_t const>::value,
    "");
static_assert(
    std::is_same<
        ::estd::internal::return_T<::estd::internal::TL_id<MS816::types, uint16_t const>>::type,
        uint16_t const>::value,
    "");
static_assert(
    std::is_same<
        ::estd::internal::return_T<::estd::internal::TL_id<MS816::types, uint16_t>>::type,
        uint16_t const>::value,
    "");

TEST(multi_slice, smoke_constructor)
{
    MS816 u8u16;
    EXPECT_EQ(0U, u8u16.size());
}

TEST(multi_slice, r_value_constructor)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    MS816 u8u16_copy(u8u16);
    EXPECT_EQ(u8u16.size(), u8u16_copy.size());
    EXPECT_EQ(u8u16.get<uint8_t>().size(), u8u16_copy.get<uint8_t>().size());
    EXPECT_EQ(u8u16.get<uint16_t const>().size(), u8u16_copy.get<uint16_t const>().size());
}

TEST(multi_slice, constructed_from_slices_of_different_sizes_has_size_zero)
{
    ::estd::array<uint16_t, 14> u16_array_14;

    MS816 u8u16 = MS816(::estd::slice<uint8_t>(u8_array), ::estd::slice<uint16_t>(u16_array_14));
    EXPECT_EQ(0U, u8u16.size());
}

TEST(multi_slice, constructed_from_slices_of_matching_sizes_has_same_size)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    EXPECT_EQ(10U, u8u16.size());
    EXPECT_EQ(10U, u8u16.get<uint8_t>().size());
    EXPECT_EQ(10U, u8u16.get<uint16_t const>().size());
}

TEST(multi_slice, constructed_from_slices_of_matching_sizes_has_same_different_size)
{
    uint8_t u8_array_5[5U];
    uint16_t u16_array_5[5U];
    MS816 u8u16 = MS816(u8_array_5, u16_array_5);
    EXPECT_EQ(5U, u8u16.size());
    EXPECT_EQ(5U, u8u16.get<uint8_t>().size());
    EXPECT_EQ(5U, u8u16.get<uint16_t const>().size());
}

TEST(multi_slice, constructed_from_slices_of_matching_sizes_has_data_pointers_initialized)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    EXPECT_EQ(u8_array, u8u16.data<uint8_t>());
    EXPECT_EQ(u8_array, u8u16.data<uint8_t const>());
    EXPECT_EQ(u16_array, u8u16.data<uint16_t>());
    EXPECT_EQ(u16_array, u8u16.data<uint16_t const>());
}

TEST(multi_slice, trim_reduces_the_size)
{
    MS816 u8u16 = MS816(u8_array, u16_array);

    u8u16.trim(6U);

    EXPECT_EQ(6U, u8u16.size());
    EXPECT_EQ(6U, u8u16.get<uint8_t>().size());
    EXPECT_EQ(6U, u8u16.get<uint16_t const>().size());
}

TEST(multi_slice, trim_sets_size_to_zero_if_passed_value_is_bigger_than_size)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.trim(12);
    EXPECT_EQ(0U, u8u16.size());
}

TEST(multi_slice, advance_reduces_size)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.advance(6);
    EXPECT_EQ(4U, u8u16.size());
}

TEST(multi_slice, advance_adjusts_data_pointers)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.advance(6);
    EXPECT_EQ(u8_array + 6U, u8u16.get<uint8_t>().data());
    EXPECT_EQ(u16_array + 6U, u8u16.get<uint16_t const>().data());
}

TEST(multi_slice, advance_sets_size_to_zero_if_size_is_to_big)
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.advance(11);
    EXPECT_EQ(0U, u8u16.size());
}

TEST(multi_slice, can_be_converted_to_a_subset_multi_slice)
{
    ::estd::array<uint8_t, 10> a8;
    ::estd::array<uint16_t, 10> a16;
    ::estd::array<uint32_t, 10> a32;

    // Note that it also converts "uint16_t" -> "const uint16_t"
    ::estd::multi_slice<uint16_t, uint8_t, uint32_t> c(a16, a8, a32);
    MS816 u8u16 = MS816(c);

    EXPECT_EQ(10U, u8u16.size());
    EXPECT_EQ(a16.data(), u8u16.data<uint16_t const>());
    EXPECT_EQ(a8.data(), u8u16.data<uint8_t>());
}

TEST(multi_slice, idx_gives_the_position_of_an_item)
{
    MS816 u8u16 = MS816(u8_array, u16_array);

    EXPECT_EQ(3U, u8u16.idx(&u8_array[3]));
    EXPECT_EQ(4U, u8u16.idx(&u16_array[4]));
}

TEST(multi_slice, at_returns_reference_to_an_item)
{
    MS816 u8u16 = MS816(u8_array, u16_array);

    EXPECT_EQ(&u8_array[2], &u8u16.at<uint8_t>(2));
    EXPECT_EQ(&u16_array[5], &u8u16.at<uint16_t>(5));
}

template<size_t>
struct V
{
    V() : value(0) {}

    V(int v) : value(v) {}

    int value;
};

TEST(multi_slice, all_the_overloads_want_to_be_covered)
{
    ::estd::array<V<0>, 5> v0;
    ::estd::array<V<1>, 5> v1;
    ::estd::array<V<2>, 5> v2;
    ::estd::array<V<3>, 5> v3;
    ::estd::array<V<4>, 5> v4;
    ::estd::array<V<5>, 5> v5;
    ::estd::array<V<6>, 5> v6;
    ::estd::array<V<7>, 5> v7;

    ::estd::array<V<8>, 1> d0; // different length to test constructor return case

    ::estd::multi_slice<V<0>, V<8>> z2(v0, d0);
    ::estd::multi_slice<V<0>, V<1>, V<8>> z3(v0, v1, d0);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<8>> z4(v0, v1, v2, d0);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<8>> z5(v0, v1, v2, v3, d0);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<8>> z6(v0, v1, v2, v3, v4, d0);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<8>> z7(v0, v1, v2, v3, v4, v5, d0);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6>, V<8>> z8(
        v0, v1, v2, v3, v4, v5, v6, d0);

    EXPECT_EQ(0U, z2.size());
    EXPECT_EQ(0U, z3.size());
    EXPECT_EQ(0U, z4.size());
    EXPECT_EQ(0U, z5.size());
    EXPECT_EQ(0U, z6.size());
    EXPECT_EQ(0U, z7.size());
    EXPECT_EQ(0U, z8.size());

    ::estd::multi_slice<V<0>, V<1>> s2(v0, v1);
    ::estd::multi_slice<V<0>, V<1>, V<2>> s3(v0, v1, v2);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>> s4(v0, v1, v2, v3);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>> s5(v0, v1, v2, v3, v4);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>> s6(v0, v1, v2, v3, v4, v5);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6>> s7(v0, v1, v2, v3, v4, v5, v6);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6>, V<7>> s8(
        v0, v1, v2, v3, v4, v5, v6, v7);

    EXPECT_EQ(0U, s2.advance(11).size());
    EXPECT_EQ(0U, s3.advance(11).size());
    EXPECT_EQ(0U, s4.advance(11).size());
    EXPECT_EQ(0U, s5.advance(11).size());
    EXPECT_EQ(0U, s6.advance(11).size());
    EXPECT_EQ(0U, s7.advance(11).size());
    EXPECT_EQ(0U, s8.advance(11).size());
}

TEST(multi_slice, make_multi_slice_overloads)
{
    ::estd::array<V<0>, 5> v0;
    ::estd::array<V<1>, 5> v1;
    ::estd::array<V<2>, 5> v2;
    ::estd::array<V<3>, 5> v3;
    ::estd::array<V<4>, 5> v4;
    ::estd::array<V<5>, 5> v5;
    ::estd::array<V<6>, 5> v6;
    ::estd::array<V<7>, 5> v7;

    ::estd::multi_slice<V<0>, V<1>> s2 = ::estd::make_multi_slice<V<0>, V<1>>(v0, v1);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>> s4
        = ::estd::make_multi_slice<V<0>, V<1>, V<2>, V<3>>(v0, v1, v2, v3);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>> s5
        = ::estd::make_multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>>(v0, v1, v2, v3, v4);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>> s6
        = ::estd::make_multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>>(v0, v1, v2, v3, v4, v5);
    ::estd::multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6>, V<7>> s8
        = ::estd::make_multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6>, V<7>>(
            v0, v1, v2, v3, v4, v5, v6, v7);

    EXPECT_EQ(5U, s2.size());
    EXPECT_EQ(5U, s4.size());
    EXPECT_EQ(5U, s5.size());
    EXPECT_EQ(5U, s6.size());
    EXPECT_EQ(5U, s8.size());
}

} // namespace
