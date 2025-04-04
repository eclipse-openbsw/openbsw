// Copyright 2024 Accenture.

#pragma once

#include <cstdint>

namespace util
{
namespace meta
{
/**
 * Helper class to create a bit mask of N ones.
 * \tparam  N   Number of ones of the resulting bit mask.
 *
 * \section example Example usage
 * \code
 * uint32_t mask = Bitmask<5>::value; //will set mask to 0x0000001F;
 * \endcode
 */
template<class T, uint8_t N>
struct Bitmask
{
    static_assert(((sizeof(T) * 8) >= N), "N does not for for T");
    static T const value = 1U + static_cast<T>(Bitmask<T, N - 1>::value << 1U);
};

template<class T, uint8_t N>
T const Bitmask<T, N>::value;

/**
 * Specialization for CURRENT == 0.
 */
template<class T>
struct Bitmask<T, 0U>
{
    static T const value = 0;
};

template<class T>
T const Bitmask<T, 0>::value;

} // namespace meta
} // namespace util
