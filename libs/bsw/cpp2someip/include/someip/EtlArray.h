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

#include "someip/PrimitiveTypes.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include <etl/array.h>

namespace someip
{
namespace internal
{
/// \cond INTERNAL
template<class T, size_t S>
struct GetSizeHelper<::etl::array<T, S>>
{
    static uint32_t getSize(::etl::array<T, S> const& t, uint32_t const lengthSize)
    {
        uint32_t size = 0U;

        for (auto const entry : t)
        {
            size += GetSizeHelper<T>::getSize(entry, lengthSize);
        }

        return size;
    }

    static uint32_t
    getSize(::etl::array<T, S> const& t, uint32_t const lengthSize, Encoding const encoding)
    {
        uint32_t size = lengthSize;

        for (auto const entry : t)
        {
            size += GetSizeHelper<T>::getSize(entry, lengthSize, encoding);
        }

        return size;
    }
};

/// \endcond

} // namespace internal
} // namespace someip

namespace someip
{
/**
 * ArrayParser is a helper class used for reading a SOME/IP fixed-length array from
 * an array of bytes. This class is used by the SOME/IP code generator.
 *
 * \tparam T The type of the underlying fixed-length array.
 * \tparam SIZE The length of the array.
 */
template<class T, size_t SIZE>
class ArrayParser
{
public:
    ArrayParser() = delete;

    /**
     * Create a ArrayParser that will fill the specified 'obj' with objects of type 'T'.
     *
     * \param obj The estd array to fill.
     */
    static void parse(::etl::array<T, SIZE>& obj, SomeIpParser& parser, bool isBigEndian);
};

/**
 * ArraySerializer is a helper class used for writing a SOME/IP fixed-length array to
 * an array of bytes.
 *
 * \tparam T The type of the underlying fixed-length array.
 * \tparam SIZE The length of the fixed-length array.
 */
template<class T, size_t SIZE>
class ArraySerializer
{
public:
    ArraySerializer() = delete;

    /**
     * Create a ArraySerializer to serialize the specified array into an array of bytes.
     *
     * \param obj The array to serialize.
     */
    static void
    serialize(::etl::array<T, SIZE> const& obj, SomeIpSerializer& serializer, bool isBigEndian);
};

/*
 * inline implementation
 */

/*
 * ArraySerializer
 */
// static
template<class T, size_t SIZE>
void ArraySerializer<T, SIZE>::serialize(
    ::etl::array<T, SIZE> const& obj, SomeIpSerializer& serializer, bool const isBigEndian)
{
    if (!serializer.isGood())
    {
        return;
    }

    if (isBigEndian == true)
    {
        serializer.bigEndian();
    }
    else
    {
        serializer.littleEndian();
    }

    // write out each element
    for (size_t i = 0U; i < SIZE; ++i)
    {
        serializer << obj[i];
    }
}

/*
 * ArrayParser
 */
// static
template<class T, size_t SIZE>
void ArrayParser<T, SIZE>::parse(
    ::etl::array<T, SIZE>& obj, SomeIpParser& parser, bool const isBigEndian)
{
    if (!parser.isGood())
    {
        return;
    }

    if (isBigEndian == true)
    {
        parser.bigEndian();
    }
    else
    {
        parser.littleEndian();
    }

    for (size_t i = 0U; i < SIZE; ++i)
    {
        if (!parser.isGood())
        {
            // fatal. failed to read in all values!
            parser.setFailure();
            return;
        }

        parser >> obj[i];
    }
}

} // namespace someip
