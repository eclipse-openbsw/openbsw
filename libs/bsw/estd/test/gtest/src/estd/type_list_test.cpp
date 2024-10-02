// Copyright 2024 Accenture.

#include "estd/type_list.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
struct double_uint64_t
{
    uint64_t a[2];
};
} // namespace

using tl_1               = ::estd::make_type_list<uint8_t>::type;
using tl_2               = ::estd::make_type_list<uint16_t>::type;
using tl_4               = ::estd::make_type_list<uint32_t>::type;
using tl_1_2             = ::estd::make_type_list<uint8_t, uint16_t>::type;
using tl_4_2             = ::estd::make_type_list<uint32_t, uint16_t>::type;
using tl_1_2_4           = ::estd::make_type_list<uint8_t, uint16_t, uint32_t>::type;
using tl_1_1_2_2_4_4_8_8 = ::estd::
    make_type_list<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>::type;

using tl_1_1_2_2_4_4_8_8_16 = ::estd::make_type_list<
    uint8_t,
    int8_t,
    uint16_t,
    int16_t,
    uint32_t,
    int32_t,
    uint64_t,
    int64_t,
    double_uint64_t>::type;

using tl_1_v = ::estd::make_type_list<uint8_t, void>::type;

TEST(type_list, type_indexing)
{
    EXPECT_EQ(0, tl_1::index_of<uint8_t>());

    EXPECT_EQ(0, tl_1_v::index_of<uint8_t>());

    EXPECT_EQ(0, tl_1_2::index_of<uint8_t>());
    EXPECT_EQ(1, tl_1_2::index_of<uint16_t>());

    EXPECT_EQ(0, tl_1_2_4::index_of<uint8_t>());
    EXPECT_EQ(1, tl_1_2_4::index_of<uint16_t>());
    EXPECT_EQ(2, tl_1_2_4::index_of<uint32_t>());

    EXPECT_EQ(0, tl_1_1_2_2_4_4_8_8::index_of<uint8_t>());
    EXPECT_EQ(1, tl_1_1_2_2_4_4_8_8::index_of<int8_t>());
    EXPECT_EQ(2, tl_1_1_2_2_4_4_8_8::index_of<uint16_t>());
    EXPECT_EQ(3, tl_1_1_2_2_4_4_8_8::index_of<int16_t>());
    EXPECT_EQ(4, tl_1_1_2_2_4_4_8_8::index_of<uint32_t>());
    EXPECT_EQ(5, tl_1_1_2_2_4_4_8_8::index_of<int32_t>());
    EXPECT_EQ(6, tl_1_1_2_2_4_4_8_8::index_of<uint64_t>());
    EXPECT_EQ(7, tl_1_1_2_2_4_4_8_8::index_of<int64_t>());
    EXPECT_EQ(8, tl_1_1_2_2_4_4_8_8_16::index_of<double_uint64_t>());
}

TEST(type_list, type_indexing_standalone)
{
    EXPECT_EQ(0, (::estd::index_of<tl_1, uint8_t>::value));
    EXPECT_EQ(0, (::estd::index_of<tl_1_v, uint8_t>::value));
    EXPECT_EQ(0, (::estd::index_of<tl_1_2, uint8_t>::value));
    EXPECT_EQ(1, (::estd::index_of<tl_1_2, uint16_t>::value));
    EXPECT_EQ(0, (::estd::index_of<tl_1_2_4, uint8_t>::value));
    EXPECT_EQ(1, (::estd::index_of<tl_1_2_4, uint16_t>::value));
    EXPECT_EQ(2, (::estd::index_of<tl_1_2_4, uint32_t>::value));
    EXPECT_EQ(0, (::estd::index_of<tl_1_1_2_2_4_4_8_8, uint8_t>::value));
    EXPECT_EQ(1, (::estd::index_of<tl_1_1_2_2_4_4_8_8, int8_t>::value));
    EXPECT_EQ(2, (::estd::index_of<tl_1_1_2_2_4_4_8_8, uint16_t>::value));
    EXPECT_EQ(3, (::estd::index_of<tl_1_1_2_2_4_4_8_8, int16_t>::value));
    EXPECT_EQ(4, (::estd::index_of<tl_1_1_2_2_4_4_8_8, uint32_t>::value));
    EXPECT_EQ(5, (::estd::index_of<tl_1_1_2_2_4_4_8_8, int32_t>::value));
    EXPECT_EQ(6, (::estd::index_of<tl_1_1_2_2_4_4_8_8, uint64_t>::value));
    EXPECT_EQ(7, (::estd::index_of<tl_1_1_2_2_4_4_8_8, int64_t>::value));
    EXPECT_EQ(8, (::estd::index_of<tl_1_1_2_2_4_4_8_8_16, double_uint64_t>::value));
}

TEST(type_list, contains)
{
    EXPECT_TRUE((tl_1::contains<uint8_t>()));
    EXPECT_FALSE((tl_1::contains<int8_t>()));

    EXPECT_TRUE((tl_1_1_2_2_4_4_8_8_16::contains<double_uint64_t>()));
    EXPECT_FALSE((tl_1_1_2_2_4_4_8_8_16::contains<bool>()));
}

TEST(type_list, contains_standalone)
{
    EXPECT_TRUE((::estd::contains<tl_1, uint8_t>::value));
    EXPECT_FALSE((::estd::contains<tl_1, int8_t>::value));
    EXPECT_TRUE((::estd::contains<tl_1_1_2_2_4_4_8_8_16, double_uint64_t>::value));
    EXPECT_FALSE((::estd::contains<tl_1_1_2_2_4_4_8_8_16, bool>::value));
}

TEST(type_list, max_size)
{
    EXPECT_EQ(1, tl_1::max_size());
    EXPECT_EQ(2, tl_1_2::max_size());
    EXPECT_EQ(4, tl_4_2::max_size());
    EXPECT_EQ(4, tl_1_2_4::max_size());
    EXPECT_EQ(8, tl_1_1_2_2_4_4_8_8::max_size());
    EXPECT_EQ(16, tl_1_1_2_2_4_4_8_8_16::max_size());

    EXPECT_EQ(1, tl_1_v::max_size());
}

TEST(type_list, max_size_standalone)
{
    EXPECT_EQ(1, ::estd::max_size<tl_1>::value);
    EXPECT_EQ(2, ::estd::max_size<tl_1_2>::value);
    EXPECT_EQ(4, ::estd::max_size<tl_4_2>::value);
    EXPECT_EQ(4, ::estd::max_size<tl_1_2_4>::value);
    EXPECT_EQ(8, ::estd::max_size<tl_1_1_2_2_4_4_8_8>::value);
    EXPECT_EQ(16, ::estd::max_size<tl_1_1_2_2_4_4_8_8_16>::value);
    EXPECT_EQ(1, ::estd::max_size<tl_1_v>::value);
}

TEST(type_list, traits)
{
    EXPECT_TRUE(tl_1::all_types<std::is_integral>());
    EXPECT_TRUE(!tl_1_1_2_2_4_4_8_8_16::all_types<std::is_integral>());

    EXPECT_TRUE(tl_1_1_2_2_4_4_8_8_16::all_types<std::is_trivially_copyable>());

    using mixed = ::estd::make_type_list<void*, uint8_t, uint32_t*>::type;

    EXPECT_FALSE(mixed::all_types<std::is_integral>());
    EXPECT_FALSE(mixed::all_types<std::is_pointer>());
}

TEST(type_list, traits_standalone)
{
    EXPECT_TRUE((::estd::all_types<tl_1, std::is_integral>::value));
    EXPECT_TRUE((!::estd::all_types<tl_1_1_2_2_4_4_8_8_16, std::is_integral>::value));
    EXPECT_TRUE((::estd::all_types<tl_1_1_2_2_4_4_8_8_16, std::is_trivially_copyable>::value));
    using mixed = ::estd::make_type_list<void*, uint8_t, uint32_t*>::type;
    EXPECT_FALSE((::estd::all_types<mixed, std::is_integral>::value));
    EXPECT_FALSE((::estd::all_types<mixed, std::is_pointer>::value));
}

TEST(type_list, max_alignment)
{
    EXPECT_EQ(1, tl_1::max_align());
    EXPECT_EQ(2, tl_1_2::max_align());
    EXPECT_EQ(4, tl_4_2::max_align());
    EXPECT_EQ(4, tl_1_2_4::max_align());
    EXPECT_EQ(8, tl_1_1_2_2_4_4_8_8::max_align());
    EXPECT_EQ(8, tl_1_1_2_2_4_4_8_8_16::max_align());

    EXPECT_EQ(1, tl_1_v::max_align());
}

TEST(type_list, max_alignment_standalone)
{
    EXPECT_EQ(1, ::estd::max_align<tl_1>::value);
    EXPECT_EQ(2, ::estd::max_align<tl_1_2>::value);
    EXPECT_EQ(4, ::estd::max_align<tl_4_2>::value);
    EXPECT_EQ(4, ::estd::max_align<tl_1_2_4>::value);
    EXPECT_EQ(8, ::estd::max_align<tl_1_1_2_2_4_4_8_8>::value);
    EXPECT_EQ(8, ::estd::max_align<tl_1_1_2_2_4_4_8_8_16>::value);
    EXPECT_EQ(1, ::estd::max_align<tl_1_v>::value);
}

TEST(type_list, size)
{
    EXPECT_EQ(1, tl_1::size());
    EXPECT_EQ(2, tl_1_2::size());
    EXPECT_EQ(2, tl_4_2::size());
    EXPECT_EQ(3, tl_1_2_4::size());
    EXPECT_EQ(8, tl_1_1_2_2_4_4_8_8::size());
    EXPECT_EQ(9, tl_1_1_2_2_4_4_8_8_16::size());
}

TEST(type_list, flat_type_list)
{
    // TL on its own is preserved
    using tl_1_2_merge = ::estd::flat_type_list<tl_1_2>::type;
    EXPECT_EQ(2, tl_1_2_merge::size());
    EXPECT_EQ(0, tl_1_2_merge::index_of<uint8_t>());
    EXPECT_EQ(1, tl_1_2_merge::index_of<uint16_t>());

    // integral then TL
    using tl_4_1_2 = ::estd::flat_type_list<uint32_t, tl_1_2>::type;
    EXPECT_EQ(3, tl_4_1_2::size());
    EXPECT_EQ(0, tl_4_1_2::index_of<uint32_t>());
    EXPECT_EQ(1, tl_4_1_2::index_of<uint8_t>());
    EXPECT_EQ(2, tl_4_1_2::index_of<uint16_t>());

    // TL then integral
    using tl_1_2_4 = ::estd::flat_type_list<tl_1_2, uint32_t>::type;
    EXPECT_EQ(3, tl_1_2_4::size());
    EXPECT_EQ(0, tl_1_2_4::index_of<uint8_t>());
    EXPECT_EQ(1, tl_1_2_4::index_of<uint16_t>());
    EXPECT_EQ(2, tl_1_2_4::index_of<uint32_t>());

    using tl_1s_2s_4s = ::estd::flat_type_list<int8_t, int16_t, int32_t>::type;

    // TL then TL
    using tl_4_1_2_1_2_4 = ::estd::flat_type_list<tl_4_1_2, tl_1s_2s_4s>::type;
    EXPECT_EQ(6, tl_4_1_2_1_2_4::size());
    EXPECT_EQ(0, tl_4_1_2_1_2_4::index_of<uint32_t>());
    EXPECT_EQ(1, tl_4_1_2_1_2_4::index_of<uint8_t>());
    EXPECT_EQ(2, tl_4_1_2_1_2_4::index_of<uint16_t>());
    EXPECT_EQ(3, tl_4_1_2_1_2_4::index_of<int8_t>());
    EXPECT_EQ(4, tl_4_1_2_1_2_4::index_of<int16_t>());
    EXPECT_EQ(5, tl_4_1_2_1_2_4::index_of<int32_t>());
}

TEST(type_list, make_type_list_nested)
{
    // Un-nest nested types in nested TL - i.e. flatten TL
    // This test is just to prove that it's possible to flatten any nested TL
    using tl_nested_1 = ::estd::make_type_list<tl_1>::type;
    EXPECT_EQ(1, tl_nested_1::size());
    EXPECT_EQ(0, (tl_nested_1::index_of<::estd::type_list<uint8_t, ::estd::type_list_end>>()));
    using tl_merged_1 = ::estd::flat_type_list<tl_nested_1>::type;
    EXPECT_EQ(1, tl_merged_1::size());
    EXPECT_EQ(0, tl_merged_1::index_of<uint8_t>());

    // A bit of a ridiculous example, but it proves it will un-nest arbitrary nestings
    using tl_front_nested_1_2_4 = ::estd::make_type_list<::estd::make_type_list<
        ::estd::make_type_list<::estd::make_type_list<tl_1>::type, tl_2>::type,
        tl_4>::type>::type;
    EXPECT_EQ(1, tl_front_nested_1_2_4::size());
    EXPECT_EQ(
        0,
        (tl_front_nested_1_2_4::index_of<::estd::type_list<
             ::estd::type_list<::estd::type_list<tl_1>, ::estd::type_list<tl_2>>,
             ::estd::type_list<tl_4>>>()));

    using tl_merged_1_2_4 = ::estd::flat_type_list<tl_front_nested_1_2_4>::type;
    EXPECT_EQ(3, tl_merged_1_2_4::size());
    EXPECT_EQ(0, tl_merged_1_2_4::index_of<uint8_t>());
    EXPECT_EQ(1, tl_merged_1_2_4::index_of<uint16_t>());
    EXPECT_EQ(2, tl_merged_1_2_4::index_of<uint32_t>());

    using tl_middle_nested_1_2_4 = ::estd::make_type_list<tl_1, tl_2, tl_4>::type;
    EXPECT_EQ(3, tl_middle_nested_1_2_4::size());
    EXPECT_EQ(0, tl_middle_nested_1_2_4::index_of<tl_1>());
    EXPECT_EQ(1, tl_middle_nested_1_2_4::index_of<tl_2>());
    EXPECT_EQ(2, tl_middle_nested_1_2_4::index_of<tl_4>());

    // Can declare this multiple times in a single translation unit because it's the exact same type
    using tl_merged_1_2_4 = ::estd::flat_type_list<tl_middle_nested_1_2_4>::type;

    using tl_end_nested_1_2_4 = ::estd::make_type_list<
        tl_1,
        ::estd::make_type_list<tl_2, ::estd::make_type_list<tl_4>::type>::type>::type;
    EXPECT_EQ(2, tl_end_nested_1_2_4::size());
    EXPECT_EQ(0, tl_end_nested_1_2_4::index_of<tl_1>());
    EXPECT_EQ(
        1,
        (tl_end_nested_1_2_4::index_of<
            ::estd::type_list<tl_2, ::estd::type_list<::estd::type_list<tl_4>>>>()));

    // Can declare this multiple times in a single translation unit because it's the exact same type
    using tl_merged_1_2_4 = ::estd::flat_type_list<tl_end_nested_1_2_4>::type;
}

TEST(type_list, make_type_list_other_templates)
{
    // Need to be able to preserve all other multi-templated types aside from type_list that are
    // passed into flat_type_list
    using tl_vec
        = ::estd::flat_type_list<std::vector<uint8_t, std::allocator<uint8_t>>, uint16_t>::type;
    EXPECT_EQ(2, tl_vec::size());
    EXPECT_EQ(0, tl_vec::index_of<std::vector<uint8_t>>());
    EXPECT_EQ(1, tl_vec::index_of<uint16_t>());
}
