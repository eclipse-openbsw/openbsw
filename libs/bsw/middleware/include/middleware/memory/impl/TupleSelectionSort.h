// Copyright 2025 BMW AG

#pragma once

#include <etl/tuple.h>
#include <etl/type_traits.h>

#include <cstddef>

namespace middleware
{
namespace impl
{

template<size_t i, size_t j, class Tuple>
class TupleElementSwap
{
    template<class IndexSequence>
    struct TupleElementSwapImpl;

    template<size_t... indices>
    struct TupleElementSwapImpl<etl::index_sequence<indices...>>
    {
        using type = etl::tuple<typename etl::tuple_element<
            (indices != i) && (indices != j) ? indices : ((indices == i) ? j : i),
            Tuple>::type...>;
    };

public:
    using type = typename TupleElementSwapImpl<
        etl::make_index_sequence<etl::tuple_size<Tuple>::value>>::type;
};

template<template<typename, typename> class Comparator, typename Tuple>
class TupleSelectionSort
{
    template<size_t I, size_t J, size_t TupleSize, class LoopTuple>
    struct TupleSelectionSortImpl
    {
        using tuple_type = typename etl::conditional<
            Comparator<
                typename etl::tuple_element<I, LoopTuple>::type,
                typename etl::tuple_element<J, LoopTuple>::type>::value,
            typename TupleElementSwap<I, J, LoopTuple>::type,
            LoopTuple>::type;

        using type = typename TupleSelectionSortImpl<I, J + 1U, TupleSize, tuple_type>::type;
    };

    template<size_t I, size_t TupleSize, class LoopTuple>
    struct TupleSelectionSortImpl<I, TupleSize, TupleSize, LoopTuple>
    {
        using type = typename TupleSelectionSortImpl<I + 1U, I + 2U, TupleSize, LoopTuple>::type;
    };

    template<size_t J, size_t TupleSize, class LoopTuple>
    struct TupleSelectionSortImpl<TupleSize, J, TupleSize, LoopTuple>
    {
        using type = LoopTuple;
    };

public:
    using type = typename TupleSelectionSortImpl<0, 1, etl::tuple_size<Tuple>::value, Tuple>::type;
};

template<typename T, typename U>
struct Ascending
: etl::conditional<(U::chunkSize() < T::chunkSize()), etl::true_type, etl::false_type>::type
{
    static_assert(U::chunkSize() != T::chunkSize(), "Pools of same sizes are not allowed");
};

} // namespace impl
} // namespace middleware
