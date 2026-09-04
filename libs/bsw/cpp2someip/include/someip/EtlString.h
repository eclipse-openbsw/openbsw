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

#include <etl/string.h>

namespace someip
{
namespace internal
{
/// \cond INTERNAL
template<size_t S>
struct GetSizeHelper<::etl::string<S>>
{
    static uint32_t
    getSize(::etl::string<S> const& t, size_t const lengthSize, Encoding const encoding)
    {
        uint32_t const bomLength  = EncodingHelper::encodingLength(encoding);
        uint32_t const termLength = EncodingHelper::terminationLength(encoding);

        // static string
        if (lengthSize == 0U)
        {
            return static_cast<uint32_t>(
                (t.max_size() * sizeof(char)) - 1U + bomLength + termLength);
        }

        // dynamic string
        return static_cast<uint32_t>(
            (lengthSize + (t.size() * sizeof(char))) + bomLength + termLength);
    }
};

/// \endcond

} // namespace internal
} // namespace someip

namespace someip
{
/*
 * Base class string parser for both static and dynamic strings
 */
class StringParser
{
protected:
    static bool parseBom(SomeIpParser& parser, char const* bom, uint32_t bomLength);

    static void readAndSkipNulls(SomeIpParser& parser, ::etl::istring& str, uint32_t nulls);
};

/*
 * Base class string serializer for both static and dynamic strings
 */
class StringSerializer
{
protected:
    static void writeStringBytes(
        SomeIpSerializer& serializer, ::etl::istring const& str, Encoding encoding, uint32_t nulls);
};

/**
 * StaticStringParser is a helper class used for reading a SOME/IP fixed-length string from
 * an array of bytes. This class is used by the SOME/IP code generator.
 */
class StaticStringParser : public StringParser
{
public:
    /**
     * A helper friend function for reading in a fixed-length string.
     *
     * \param parser The SOME/IP parser.
     * \param str The container object that holds the string to be parsed.
     */
    static void readString(SomeIpParser& parser, ::etl::istring& str, Encoding encoding);

private:
    static void
    readUtf8Bytes(SomeIpParser& parser, ::etl::istring& str, uint32_t terminationLength);

    static void
    readUtf16Bytes(SomeIpParser& parser, ::etl::istring& str, uint32_t terminationLength);
};

class StaticStringSerializer : public StringSerializer
{
public:
    static void
    writeString(SomeIpSerializer& serializer, ::etl::istring const& str, Encoding encoding);
};

template<class LENGTH_TYPE>
class DynamicStringSerializer : public StringSerializer
{
public:
    static void
    writeString(SomeIpSerializer& serializer, ::etl::istring const& str, Encoding encoding);
};

class DynamicStringParserBase : public StringParser
{
protected:
    static void parseStringBytes(
        SomeIpParser& parser, ::etl::istring& str, Encoding encoding, uint32_t numBytes);
};

template<class LENGTH_TYPE>
class DynamicStringParser : public DynamicStringParserBase
{
public:
    static void readString(SomeIpParser& parser, ::etl::istring& str, Encoding encoding);
};

/*
 * inline implementation
 */

/*
 * DynamicStringSerializer
 */
// static
template<class LENGTH_TYPE>
void DynamicStringSerializer<LENGTH_TYPE>::writeString(
    SomeIpSerializer& serializer, ::etl::istring const& str, Encoding const encoding)
{
    if (!serializer.isGood())
    {
        return;
    }

    LengthSerializerHelper<LENGTH_TYPE> const helper(serializer);

    uint32_t const nulls = internal::EncodingHelper::terminationLength(encoding);

    writeStringBytes(serializer, str, encoding, nulls);
}

/*
 * DynamicStringParser
 */
template<class LENGTH_TYPE>
void DynamicStringParser<LENGTH_TYPE>::readString(
    SomeIpParser& parser, ::etl::istring& str, Encoding const encoding)
{
    if (!parser.isGood())
    {
        return;
    }

    str.clear();

    uint32_t const numBytes = LengthParserHelper<LENGTH_TYPE>::parseLength(parser);
    if (numBytes == 0U)
    {
        return;
    }

    parseStringBytes(parser, str, encoding, numBytes);
}

} // namespace someip
