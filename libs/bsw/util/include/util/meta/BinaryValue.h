// Copyright 2024 Accenture.

#pragma once

#include <cstdint>

namespace util
{
namespace meta
{
namespace internal
{
/**
 * Represents a binary digit.
 * Only specializations for 0 and 1 must be provided to enforce the binary alphabet.
 *
 * \tparam N state of the binary digit, only <code>0</code> or <code>1<code> are allowed.
 */
template<uint32_t N>
struct BinaryDigit;

/** Binary 0 */
template<>
struct BinaryDigit<0U>
{
    static uint32_t const value = 0U;
};

/** Binary 1 */
template<>
struct BinaryDigit<1U>
{
    static uint32_t const value = 1U;
};

} /* namespace internal */

/**
 * Interprets a given decimal number that only consists of 1 and 0 as a BinaryValue number and
 * converts it to a decimal number.
 *
 * \tparam    N Number to convert.
 *
 * \section    example    Example usage
 * \code
 *      uint32_t x = BinaryValue<1001>::value; //set x to 9
 * \endcode
 */
template<uint32_t N>
struct BinaryValue
{
    static uint32_t const value
        = internal::BinaryDigit<N % 10U>::value + (BinaryValue<N / 10U>::value << 1U);
};

/**
 * Specialization for N == 0.
 */
template<>
struct BinaryValue<0U>
{
    static uint32_t const value = 0U;
};

} /* namespace meta */
} /* namespace util */
