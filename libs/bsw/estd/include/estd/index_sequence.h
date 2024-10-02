// Copyright 2024 Accenture.

#ifndef GUARD_9224694B_14C8_497C_B1AA_2D751EC84CA5
#define GUARD_9224694B_14C8_497C_B1AA_2D751EC84CA5

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

#endif // GUARD_9224694B_14C8_497C_B1AA_2D751EC84CA5
