/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/StringEncoding.h"

#include <etl/type_list.h>
#include <util/meta/Bitmask.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace someip
{
/** List of SOME/IP base types */
using BaseTypes = ::etl::type_list<
    char,
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    int8_t,
    int16_t,
    int32_t,
    int64_t,
    bool,
    float_t,
    double_t>;

} // namespace someip

namespace someip
{
namespace internal
{
/// \cond internal
enum
{
    IEC559_NOT_SUPPORTED = false,
    IEC559_SUPPORTED     = true
};

template<typename RT, typename T>
static RT conversion(T const value)
{
    static_assert(
        sizeof(T) == sizeof(RT),
        "floating point to/from integral conversion type not the same size");

    // in C it is possible to use unions to convert from one type to another (type-punning)
    // in C++ this behavior is compiler dependent and can therefore be undefined behavior
    // as a solution memcpy can be used to convert from one type into another type
    RT dest{};
    std::memcpy(&dest, &value, sizeof(value));
    return dest;
}

template<typename T>
struct IEEE756Traits;

template<>
struct IEEE756Traits<float_t>
{
    enum
    {
        MAX_EXP = 256
    };

    static uint32_t const IEEE756_ZERO              = 0x00000000U;
    static uint32_t const IEEE756_NEGATIVE_ZERO     = 0x80000000U;
    static uint32_t const IEEE756_INFINITY          = 0x7F800000U;
    static uint32_t const IEEE756_NEGATIVE_INFINITY = 0xFF800000U;
    static uint32_t const IEEE756_NAN               = 0x7FFFFFFFU;
};

template<>
struct IEEE756Traits<double_t>
{
    enum
    {
        MAX_EXP = 2048
    };

    static uint64_t const IEEE756_ZERO              = 0x0000000000000000U;
    static uint64_t const IEEE756_NEGATIVE_ZERO     = 0x8000000000000000U;
    static uint64_t const IEEE756_INFINITY          = 0x7FF0000000000000U;
    static uint64_t const IEEE756_NEGATIVE_INFINITY = 0xFFF0000000000000U;
    static uint64_t const IEEE756_NAN               = 0x7FFFFFFFFFFFFFFFU;
};

template<bool IS_IEC599>
struct PackIEEE754Dispatcher;

template<>
struct PackIEEE754Dispatcher<IEC559_SUPPORTED>
{
    template<typename RT, typename T, uint32_t EXP_BITS>
    static RT packIEEE754(T const value)
    {
        return conversion<RT, T>(value);
    }
};

template<>
struct PackIEEE754Dispatcher<IEC559_NOT_SUPPORTED>
{
    template<typename RT, typename T, uint32_t EXP_BITS>
    static RT packIEEE754(T const value)
    {
        uint32_t const BITS          = (static_cast<uint32_t>(sizeof(T)) * 8U);
        uint32_t const FRACTION_BITS = BITS - EXP_BITS - 1U;
        if (value == static_cast<T>(0.0))
        {
            if (conversion<RT, T>(value) == static_cast<RT>(0.0))
            {
                return IEEE756Traits<T>::IEEE756_ZERO;
            }

            return IEEE756Traits<T>::IEEE756_NEGATIVE_ZERO;
        }
        if (value == std::numeric_limits<T>::infinity())
        {
            return IEEE756Traits<T>::IEEE756_INFINITY;
        }
        if (value == -std::numeric_limits<T>::infinity())
        {
            return IEEE756Traits<T>::IEEE756_NEGATIVE_INFINITY;
        }
        if (value != value)
        {
            return IEEE756Traits<T>::IEEE756_NAN;
        }

        T valueNormalized = value;
        RT sign           = 0U;
        if (value < static_cast<T>(0))
        {
            sign            = 1U;
            valueNormalized = -valueNormalized;
        }
        int32_t shift = 0;
        while ((valueNormalized >= static_cast<T>(2.0)) && (shift < IEEE756Traits<T>::MAX_EXP))
        {
            valueNormalized /= static_cast<T>(2.0);
            ++shift;
        }
        int32_t rounds = 0;
        while ((valueNormalized < 1.0) && (rounds < IEEE756Traits<T>::MAX_EXP))
        {
            valueNormalized *= static_cast<T>(2.0);
            --shift;
            ++rounds;
        }
        valueNormalized = valueNormalized - static_cast<T>(1.0);
        RT const fraction
            = valueNormalized * ((static_cast<RT>(1) << FRACTION_BITS) + static_cast<T>(0.5));
        RT const exponent
            = shift + static_cast<int32_t>(((static_cast<uint32_t>(1U) << (EXP_BITS - 1U)) - 1U));

        return (sign << (BITS - 1U)) | (exponent << FRACTION_BITS) | fraction;
    }
};

template<bool IS_IEC599>
struct UnpackIEEE754Dispatcher;

template<>
struct UnpackIEEE754Dispatcher<IEC559_SUPPORTED>
{
    template<typename RT, typename T, uint32_t EXP_BITS>
    static RT unpackIEEE754(T const value)
    {
        return conversion<RT, T>(value);
    }
};

template<>
struct UnpackIEEE754Dispatcher<IEC559_NOT_SUPPORTED>
{
    template<typename RT, typename T, uint32_t EXP_BITS>
    static RT unpackIEEE754(T const value)
    {
        uint32_t const BITS          = static_cast<uint32_t>(sizeof(T) * 8U);
        uint32_t const FRACTION_BITS = BITS - EXP_BITS - 1U;
        if (value == IEEE756Traits<RT>::IEEE756_ZERO)
        {
            return static_cast<RT>(0.0);
        }
        if (value == IEEE756Traits<RT>::IEEE756_NEGATIVE_ZERO)
        {
            return -static_cast<RT>(0.0);
        }
        if (value == IEEE756Traits<RT>::IEEE756_INFINITY)
        {
            return std::numeric_limits<RT>::infinity();
        }
        if (value == IEEE756Traits<RT>::IEEE756_NEGATIVE_INFINITY)
        {
            return -std::numeric_limits<RT>::infinity();
        }
        T const sign = (value >> (BITS - 1U));
        T const exponent
            = (value & static_cast<T>(::util::meta::Bitmask<uint64_t, BITS - 1U>::value))
              >> FRACTION_BITS;
        T const fraction
            = (value & static_cast<T>(::util::meta::Bitmask<uint64_t, FRACTION_BITS>::value));

        if ((::util::meta::Bitmask<uint64_t, EXP_BITS>::value == exponent) && (fraction > 0U))
        {
            return std::numeric_limits<RT>::quiet_NaN();
        }
        RT valueNormalized = static_cast<RT>(fraction);
        int32_t shift      = static_cast<int32_t>(
            exponent - static_cast<T>(::util::meta::Bitmask<uint64_t, EXP_BITS - 1U>::value));

        valueNormalized
            /= static_cast<T>(static_cast<T>(1U) << FRACTION_BITS) + static_cast<RT>(0.5);
        valueNormalized += static_cast<RT>(1.0);

        while (shift < 0)
        {
            valueNormalized /= static_cast<RT>(2.0);
            ++shift;
        }
        while (shift > 0)
        {
            valueNormalized *= static_cast<RT>(2.0);
            --shift;
        }
        if (sign != 0U)
        {
            valueNormalized = -valueNormalized;
        }
        return valueNormalized;
    }
};

template<typename RT, typename T, uint32_t EXP_BITS>
RT packIEEE754(T const value)
{
    return PackIEEE754Dispatcher<
        std::numeric_limits<T>::is_iec559>::template packIEEE754<RT, T, EXP_BITS>(value);
}

template<typename RT, typename T, uint32_t EXP_BITS>
RT unpackIEEE754(T const value)
{
    return UnpackIEEE754Dispatcher<
        std::numeric_limits<RT>::is_iec559>::template unpackIEEE754<RT, T, EXP_BITS>(value);
}

template<typename T, bool IS_BASIC_TYPE>
struct GetSizeDispatcher;

template<typename T>
struct GetSizeDispatcher<T, true>
{
    static uint32_t getSize(T const& /* t */) { return static_cast<uint32_t>(sizeof(T)); }
};

template<typename T>
struct GetSizeDispatcher<T, false>
{
    static uint32_t getSize(T const& t) { return t.getSize(); }
};

template<typename T>
struct GetSizeHelper
{
    static uint32_t getSize(T const& t, uint32_t const /* lengthSize */)
    {
        return GetSizeDispatcher<T, ::etl::type_list_contains<BaseTypes, T>::value>::getSize(t);
    }

    static uint32_t
    getSize(T const& t, uint32_t const /* lengthSize */, Encoding const /* encoding */)
    {
        return GetSizeDispatcher<T, ::etl::type_list_contains<BaseTypes, T>::value>::getSize(t);
    }
};

/// \endcond
} // namespace internal
} // namespace someip
