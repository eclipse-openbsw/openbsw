// Copyright 2024 Accenture.

/**
 * Contains internal type traits
 * \file
 * \ingroup estl_utils
 */
#ifndef GUARD_CF8086C7_8C2B_4181_AB7E_21B1F43B9FDA
#define GUARD_CF8086C7_8C2B_4181_AB7E_21B1F43B9FDA

#include <climits>

namespace estd
{
namespace internal
{
template<class T, T Min, T Max, T Zero>
struct _numeric_limits_base
{
    static constexpr T min  = Min;
    static constexpr T max  = Max;
    static constexpr T zero = Zero;
};
} // namespace internal

template<class T>
struct numeric_limits
{};

template<>
struct numeric_limits<bool> : internal::_numeric_limits_base<bool, false, true, false>
{};

template<>
struct numeric_limits<signed char>
: internal::_numeric_limits_base<signed char, SCHAR_MIN, SCHAR_MAX, 0>
{};

template<>
struct numeric_limits<unsigned char>
: internal::_numeric_limits_base<unsigned char, 0U, UCHAR_MAX, 0U>
{};

template<>
struct numeric_limits<signed short>
: internal::_numeric_limits_base<signed short, SHRT_MIN, SHRT_MAX, 0>
{};

template<>
struct numeric_limits<unsigned short>
: internal::_numeric_limits_base<unsigned short, 0U, USHRT_MAX, 0U>
{};

template<>
struct numeric_limits<signed int> : internal::_numeric_limits_base<signed int, INT_MIN, INT_MAX, 0>
{};

template<>
struct numeric_limits<unsigned int> : internal::_numeric_limits_base<unsigned int, 0U, UINT_MAX, 0U>
{};

template<>
struct numeric_limits<signed long>
: internal::_numeric_limits_base<signed long, LONG_MIN, LONG_MAX, 0>
{};

template<>
struct numeric_limits<unsigned long>
: internal::_numeric_limits_base<unsigned long, 0U, ULONG_MAX, 0U>
{};

template<>
struct numeric_limits<signed long long>
: internal::_numeric_limits_base<signed long long, LLONG_MIN, LLONG_MAX, 0>
{};

template<>
struct numeric_limits<unsigned long long>
: internal::_numeric_limits_base<unsigned long long, 0U, ULLONG_MAX, 0U>
{};

} // namespace estd

#endif // GUARD_CF8086C7_8C2B_4181_AB7E_21B1F43B9FDA
