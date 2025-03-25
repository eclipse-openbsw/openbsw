// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace estd
{
/**
 * Represents a sequence of integers generated at compile time.
 * Can be used as alternative for integer_sequence from C++14.
 */
template<std::size_t...>
struct index_sequence
{};

namespace internal
{
template<std::size_t N, std::size_t... Next>
struct IndexSequenceHelper : public IndexSequenceHelper<N - 1, N - 1, Next...>
{};

template<std::size_t... Next>
struct IndexSequenceHelper<0, Next...>
{
    using type = index_sequence<Next...>;
};
} // namespace internal

template<std::size_t N>
using make_index_sequence = typename internal::IndexSequenceHelper<N>::type;

} // namespace estd

