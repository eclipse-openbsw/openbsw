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

#include "someip/LengthHelper.h"
#include "someip/PrimitiveTypes.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"
#include "someip/StringEncoding.h"

#include <etl/vector.h>

namespace someip
{
namespace internal
{
/// \cond INTERNAL
template<class T, size_t S>
struct GetSizeHelper<::etl::vector<T, S>>
{
    static uint32_t getSize(::etl::vector<T, S> const& t, uint32_t const lengthSize)
    {
        uint32_t size = lengthSize;

        for (uint32_t i = 0U; i < t.size(); ++i)
        {
            size += GetSizeHelper<T>::getSize(t[i], lengthSize);
        }

        return size;
    }

    static uint32_t
    getSize(::etl::vector<T, S> const& t, uint32_t const lengthSize, Encoding const encoding)
    {
        uint32_t size = lengthSize;

        for (uint32_t i = 0U; i < t.size(); ++i)
        {
            size += GetSizeHelper<T>::getSize(t[i], lengthSize, encoding);
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
 * DynamicArrayParser is a helper class used for reading a SOME/IP dynamic array from
 * an array of bytes.
 *
 * \tparam T The type of the underlying dynamic array.
 * \tparam LENGTH_TYPE The type used for the length field.
 */
template<class T, class LENGTH_TYPE>
class DynamicArrayParser
{
public:
    DynamicArrayParser() = delete;

    /**
     * Create a DynamicArrayParser that will fill the specified 'obj' with objects of type 'T'.
     *
     * \param obj The estd vector to fill
     * \param parser The SomeIpParser used to parse the dynamic array items
     * \param minLength The minimum number of elements required for this dynamic array
     * \param isBigEndian True if this dynamic array is stored in big endian, false for little
     * endian
     */
    static void
    parse(::etl::ivector<T>& obj, SomeIpParser& parser, size_t minLength, bool isBigEndian);
};

/**
 * DynamicArraySerializer is a helper class used for writing a SOME/IP dynamic array to
 * an array of bytes.
 *
 * \tparam T The type of the underlying dynamic array.
 * \tparam LENGTH_TYPE The type used for the length field.
 */
template<class T, class LENGTH_TYPE>
class DynamicArraySerializer
{
public:
    DynamicArraySerializer() = delete;

    /**
     * Create a DynamicArraySerializer to serialize the specified vector into an array of bytes.
     *
     * \param obj The vector to serialize.
     * \param serialize The SomeIpSerializer used to serialize the dynamic array items
     * \param minLength The minimum number of elements required for this dynamic array
     * \param isBigEndian True if this dynamic array is stored in big endian, false for little
     * endian
     */
    static void serialize(
        ::etl::ivector<T> const& obj,
        SomeIpSerializer& serializer,
        size_t minLength,
        bool isBigEndian);
};

/*
 * inline implementation
 */

/*
 * DynamicArrayParser
 */
// static
template<class T, class LENGTH_TYPE>
void DynamicArrayParser<T, LENGTH_TYPE>::parse(
    ::etl::ivector<T>& obj, SomeIpParser& parser, size_t const minLength, bool const isBigEndian)
{
    obj.clear();

    parser.bigEndian();

    LENGTH_TYPE const numBytes
        = static_cast<LENGTH_TYPE>(LengthParserHelper<LENGTH_TYPE>::parseLength(parser));

    if (!parser.isGood())
    {
        return;
    }

    if (isBigEndian == false)
    {
        parser.littleEndian();
    }

    size_t pos               = 0U;
    size_t const endPosition = parser.getCurrentPosition() + static_cast<size_t>(numBytes);
    while (parser.getCurrentPosition() < endPosition)
    {
        if (pos >= obj.max_size())
        {
            // fatal. Trying to read in too many values!
            obj.clear();
            parser.setFailure();
            return;
        }

        T element{};
        parser >> element;
        if (!parser.isGood())
        {
            obj.clear();
            return;
        }
        obj.push_back(element);

        pos++;
    }

    if (pos < minLength)
    {
        parser.setFailure();
        obj.clear();
    }
}

/*
 * DynamicArraySerializer
 */
// static
template<class T, class LENGTH_TYPE>
void DynamicArraySerializer<T, LENGTH_TYPE>::serialize(
    ::etl::ivector<T> const& obj,
    SomeIpSerializer& serializer,
    size_t const minLength,
    bool const isBigEndian)
{
    if (!serializer.isGood())
    {
        return;
    }

    if (obj.size() < minLength)
    {
        serializer.setFailure();
        return;
    }

    serializer.bigEndian();

    LengthSerializerHelper<LENGTH_TYPE> const helper(serializer);

    size_t const length = obj.size();

    if (isBigEndian == false)
    {
        serializer.littleEndian();
    }
    // write out each element
    for (size_t i = 0U; i < length; ++i)
    {
        serializer << obj[i];
    }
}

} // namespace someip
