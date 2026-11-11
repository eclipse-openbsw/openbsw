/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/array.h>
#include <etl/bit_stream.h>
#include <etl/limits.h>
#include <etl/type_traits.h>
#include <cstddef>

namespace middleware::core::codec
{
namespace detail
{
template<typename Value>
struct IsArray : etl::false_type
{};

template<typename Element, size_t Count>
struct IsArray<etl::array<Element, Count>> : etl::true_type
{};

template<typename Value, typename = void>
struct HasRawValue : etl::false_type
{};

template<typename Value>
struct HasRawValue<Value, etl::void_t<decltype(etl::declval<Value const&>().value())>>
: etl::true_type
{};
} // namespace detail

/**
 * \brief Appends a signal using the caller's stream bit and byte order.
 *
 * Arrays use full-width elements in index order. Enums use their underlying type;
 * bitset wrappers expose value(). No padding or buffer initialization is performed.
 * Stateless; concurrent calls require separate streams and buffers.
 *
 * \tparam BitLength Signal wire width in bits.
 * \param stream Writer with enough remaining buffer capacity for the signal.
 * \param value Unsigned scalar, bool, enum, bitset wrapper or fixed ETL array.
 */
template<size_t BitLength, typename Value>
void writeSignal(etl::bit_stream_writer& stream, Value const& value)
{
    static_assert(BitLength > 0U, "Signal width must be positive.");
    if constexpr (detail::IsArray<Value>::value)
    {
        using Element                = typename Value::value_type;
        constexpr size_t elementBits = etl::numeric_limits<Element>::digits;
        static_assert(
            BitLength == Value::SIZE * elementBits, "Array width must cover all elements.");
        for (auto const& element : value)
        {
            writeSignal<elementBits>(stream, element);
        }
    }
    else if constexpr (etl::is_enum<Value>::value)
    {
        using Underlying = typename etl::underlying_type<Value>::type;
        writeSignal<BitLength>(stream, static_cast<Underlying>(value));
    }
    else if constexpr (detail::HasRawValue<Value>::value)
    {
        writeSignal<BitLength>(stream, value.value());
    }
    else if constexpr (etl::is_same<Value, bool>::value)
    {
        static_assert(BitLength == 1U, "Boolean signals occupy one bit.");
        stream.write_unchecked(value);
    }
    else
    {
        static_assert(
            etl::is_integral<Value>::value && etl::is_unsigned<Value>::value,
            "Scalar signals must use unsigned integral types.");
        static_assert(
            BitLength <= etl::numeric_limits<Value>::digits,
            "Signal width exceeds its storage type.");
        stream.write_unchecked(value, BitLength);
    }
}

/**
 * \brief Reads a signal using the caller's stream bit and byte order.
 *
 * Preserves raw enum values; bitset wrappers must be constructible from value()'s type.
 * Arrays consume full-width elements in index order. Stateless; concurrent calls
 * require separate streams.
 *
 * \tparam Value Unsigned scalar, bool, enum, bitset wrapper or fixed ETL array.
 * \tparam BitLength Signal wire width in bits.
 * \param stream Reader with enough remaining buffer capacity for the signal.
 * \return Decoded signal value.
 */
template<typename Value, size_t BitLength>
Value readSignal(etl::bit_stream_reader& stream)
{
    static_assert(BitLength > 0U, "Signal width must be positive.");
    if constexpr (detail::IsArray<Value>::value)
    {
        using Element                = typename Value::value_type;
        constexpr size_t elementBits = etl::numeric_limits<Element>::digits;
        static_assert(
            BitLength == Value::SIZE * elementBits, "Array width must cover all elements.");
        Value result{};
        for (auto& element : result)
        {
            element = readSignal<Element, elementBits>(stream);
        }
        return result;
    }
    else if constexpr (etl::is_enum<Value>::value)
    {
        using Underlying = typename etl::underlying_type<Value>::type;
        return static_cast<Value>(readSignal<Underlying, BitLength>(stream));
    }
    else if constexpr (detail::HasRawValue<Value>::value)
    {
        using Underlying = decltype(etl::declval<Value const&>().value());
        return Value(readSignal<Underlying, BitLength>(stream));
    }
    else if constexpr (etl::is_same<Value, bool>::value)
    {
        static_assert(BitLength == 1U, "Boolean signals occupy one bit.");
        return stream.read_unchecked<bool>();
    }
    else
    {
        static_assert(
            etl::is_integral<Value>::value && etl::is_unsigned<Value>::value,
            "Scalar signals must use unsigned integral types.");
        static_assert(
            BitLength <= etl::numeric_limits<Value>::digits,
            "Signal width exceeds its storage type.");
        return stream.read_unchecked<Value>(BitLength);
    }
}

} // namespace middleware::core::codec
