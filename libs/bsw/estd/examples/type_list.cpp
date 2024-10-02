// Copyright 2024 Accenture.

#include "estd/type_list.h"

#include <gmock/gmock.h>

using namespace ::testing;

// Returns the position of the specific type within the typelist
void example_type_indexing()
{
    // [EXAMPLE_TYPE_INDEXING_START]

    using tl_1 = ::estd::make_type_list<uint8_t>::type;
    EXPECT_EQ(0, tl_1::index_of<uint8_t>());
    // Calling standalone function to get the index of the type
    EXPECT_EQ(0, (::estd::index_of<tl_1, uint8_t>::value));

    using tl_1_1_2_2_4_4_8_8 = ::estd::
        make_type_list<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>::
            type;
    EXPECT_EQ(5, tl_1_1_2_2_4_4_8_8::index_of<int32_t>());
    EXPECT_EQ(5, (::estd::index_of<tl_1_1_2_2_4_4_8_8, int32_t>::value));

    // [EXAMPLE_TYPE_INDEXING_END]
}

// Checks if a type is present in the typelist
void example_type_list_contains()
{
    // [EXAMPLE_TYPE_LIST_CONTAINS_START]
    using tl_1_2 = ::estd::make_type_list<uint8_t, int16_t>::type;
    EXPECT_TRUE((tl_1_2::contains<uint8_t>()));
    EXPECT_FALSE((tl_1_2::contains<uint16_t>()));
    EXPECT_TRUE((::estd::contains<tl_1_2, int16_t>::value));
    // [EXAMPLE_TYPE_LIST_CONTAINS_END]
}

// Returns the size of greatest type in the typelist
void example_max_size()
{
    // [EXAMPLE_MAXSIZE_START]
    using tl_1 = ::estd::make_type_list<uint8_t>::type;
    EXPECT_THAT(1, tl_1::max_size());

    using tl_1_2_4 = ::estd::make_type_list<uint8_t, uint16_t, uint32_t>::type;
    EXPECT_THAT(4, ::estd::max_size<tl_1_2_4>::value);
    // [EXAMPLE_MAXSIZE_END]
}

// Checks whether all the types of the type_list are of the type provided as parameter
void example_traits()
{
    // [EXAMPLE_TRAITS_START]
    using tl_1_1_2_2_4_4_8_8 = ::estd::
        make_type_list<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>::
            type;
    // Calling the all_types standalone function and checking if all the types are of integer data
    // type
    EXPECT_TRUE((::estd::all_types<tl_1_1_2_2_4_4_8_8, std::is_integral>::value));

    using types = ::estd::make_type_list<void*, uint8_t, uint16_t*>::type;
    // Returns false as all the data types in the type_list are not of integral type
    EXPECT_FALSE(types::all_types<std::is_integral>());
    // [EXAMPLE_TRAITS_END]
}

// Returns the number of types present in type_list
void example_size()
{
    // [EXAMPLE_SIZE_START]
    using tl_1_2_4 = ::estd::make_type_list<int8_t, uint16_t, uint32_t>::type;
    EXPECT_EQ(3, tl_1_2_4::size());
    // [EXAMPLE_SIZE_END]
}

void example_max_alignment()
{
    // [EXAMPLE_MAX_ALIGN_START]
    using tl_1_v = ::estd::make_type_list<uint8_t, void>::type;
    EXPECT_EQ(1, tl_1_v::max_align());
    EXPECT_EQ(1, ::estd::max_align<tl_1_v>::value);
    // [EXAMPLE_MAX_ALIGN_END]
}

// Combines type and multiple type lists into single flattened type list
void example_flat_type_list()
{
    // [EXAMPLE_FLAT_TYPE_LIST_START]
    using tl_1_2   = ::estd::make_type_list<uint8_t, int16_t>::type;
    // Combine the type list with uint32_t into a single type list
    using tl_4_1_2 = ::estd::flat_type_list<uint32_t, tl_1_2>::type;
    EXPECT_EQ(3, tl_4_1_2::size());

    using tl_1s_2s_4s    = ::estd::flat_type_list<int8_t, int16_t, int32_t>::type;
    // Combine two type lists into a single type list
    using tl_4_1_2_1_2_4 = ::estd::flat_type_list<tl_4_1_2, tl_1s_2s_4s>::type;
    EXPECT_EQ(6, tl_4_1_2_1_2_4::size());
    // [EXAMPLE_FLAT_TYPE_LIST_END]
}

TEST(type_list_example, run_examples)
{
    example_type_indexing();
    example_type_list_contains();
    example_max_size();
    example_traits();
    example_size();
    example_max_alignment();
    example_flat_type_list();
}
