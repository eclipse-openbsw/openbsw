// Copyright 2022 Accenture.

#include <estd/index_sequence.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

// [EXAMPLE_INDEX_SEQUENCE_STRUCT_START]
template<typename T>
struct UnpackSequence;

// Defining a structure 'UnpackSequence'.
template<size_t... Index>
struct UnpackSequence<estd::index_sequence<Index...>>
{
    size_t values[sizeof...(Index)]{Index...};
};

// [EXAMPLE_INDEX_SEQUENCE_STRUCT_END]

TEST(IndexSequenceExample, index_sequence)
{
    // [EXAMPLE_INDEX_SEQUENCE_START]
    // make_index_sequence<1> = 0.
    EXPECT_THAT(UnpackSequence<estd::make_index_sequence<1>>().values, ElementsAre(0));
    // make_index_sequence<3> = 0,1,2.
    EXPECT_THAT(UnpackSequence<estd::make_index_sequence<3>>().values, ElementsAre(0, 1, 2));
    // make_index_sequence<5> = 0,1,2,3,4.
    EXPECT_THAT(UnpackSequence<estd::make_index_sequence<5>>().values, ElementsAre(0, 1, 2, 3, 4));
    // [EXAMPLE_INDEX_SEQUENCE_END]
}

template<::size_t i, ::size_t j, class Tuple>
class tuple_element_swap
{
    // [EXAMPLE_INDEX_SEQUENCE_IN_TUPLE_START]
    template<class index_sequence>
    struct tuple_element_swap_impl;

    // Defining a structure 'tuple_element_swap_impl', which used to swap the elements in the tuple
    // using selection sort.
    template<::size_t... indices>
    // Passing '::estd::index_sequence<indices...>' as parameter indicates that this
    // implementation is tailored to work with a specific sequence of indices.
    struct tuple_element_swap_impl<::estd::index_sequence<indices...>>
    {
        // Swaps the elements of tuple.
        using type = std::tuple<typename std::tuple_element<
            (indices != i) && (indices != j) ? indices
            : (indices == i)                 ? j
                                             : i,
            Tuple>::type...>;
    };

    // [EXAMPLE_INDEX_SEQUENCE_IN_TUPLE_END]
public:
    using type = typename tuple_element_swap_impl<
        ::estd::make_index_sequence<std::tuple_size<Tuple>::value>>::type;
};
