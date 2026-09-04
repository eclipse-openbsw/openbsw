/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains
 */

#include "someip/EtlString.h"

namespace someip
{
// protected
// static
bool StringParser::parseBom(SomeIpParser& parser, char const* const bom, uint32_t const bomLength)
{
    char ch = '\0';

    for (uint32_t i = 0U; i < bomLength; ++i)
    {
        parser >> ch;
        if ((bom != nullptr) && (ch != bom[i]))
        {
            parser.setFailure();
            return false;
        }
    }

    return true;
}

// protected
// static
void StringParser::readAndSkipNulls(SomeIpParser& parser, ::etl::istring& str, uint32_t const nulls)
{
    char ch = '\0';

    // read in nulls
    for (uint32_t i = 0U; i < nulls; ++i)
    {
        parser >> ch;
        if (ch != '\0')
        {
            parser.setFailure();
            str.clear();
            break;
        }
    }
}

// protected
// static
void StringSerializer::writeStringBytes(
    SomeIpSerializer& serializer,
    ::etl::istring const& str,
    Encoding const encoding,
    uint32_t const nulls)
{
    // write out the BOM
    char const* const bom = ::someip::internal::EncodingHelper::bom(encoding);
    if (bom != nullptr)
    {
        uint32_t const bomLength = ::someip::internal::EncodingHelper::encodingLength(encoding);

        for (uint32_t i = 0U; i < bomLength; ++i)
        {
            serializer << bom[i];
        }

        uint32_t const length = static_cast<uint32_t>(str.size());

        // write out each element
        for (uint32_t i = 0U; i < length; ++i)
        {
            serializer << str[i];
        }

        // pad with null terminators
        for (uint32_t i = 0U; i < nulls; ++i)
        {
            serializer << '\0';
        }
    }
}

/*
 * StaticStringParser
 */
// private
// static
void StaticStringParser::readUtf8Bytes(
    SomeIpParser& parser, ::etl::istring& str, uint32_t const terminationLength)
{
    char ch = '\0';

    size_t const strMaxSize = str.max_size();
    for (size_t i = 0U; i < (strMaxSize - 1U); ++i)
    {
        parser >> ch;
        if (ch == '\0')
        {
            break;
        }
        str += ch;
    }

    if ((strMaxSize - 1U) == str.size())
    {
        readAndSkipNulls(parser, str, terminationLength);
    }
    else
    {
        readAndSkipNulls(
            parser, str, static_cast<uint32_t>(strMaxSize - str.size()) + (terminationLength - 2U));
    }
}

// private
// static
void StaticStringParser::readUtf16Bytes(
    SomeIpParser& parser, ::etl::istring& str, uint32_t const terminationLength)
{
    // for UTF16 we look for 2 null terminators
    char ch  = '\0';
    char ch2 = '\0';

    size_t const strMaxSize = str.max_size();
    for (size_t i = 0U; i < (strMaxSize - 1U); ++i)
    {
        parser >> ch;
        if (ch == '\0')
        {
            parser >> ch2;
            if (ch2 == '\0')
            {
                break;
            }

            ++i;

            str += ch;
            str += ch2;
        }
        else
        {
            str += ch;
        }
    }

    if ((strMaxSize - 1U) == str.size())
    {
        readAndSkipNulls(parser, str, terminationLength);
    }
    else
    {
        readAndSkipNulls(
            parser, str, static_cast<uint32_t>(strMaxSize - str.size()) + (terminationLength - 3U));
    }
}

// static
void StaticStringParser::readString(
    SomeIpParser& parser, ::etl::istring& str, Encoding const encoding)
{
    if (!parser.isGood())
    {
        return;
    }

    str.clear();

    char const* const bom    = ::someip::internal::EncodingHelper::bom(encoding);
    uint32_t const bomLength = ::someip::internal::EncodingHelper::encodingLength(encoding);
    uint32_t const terminationLength
        = ::someip::internal::EncodingHelper::terminationLength(encoding);

    // make sure we have enough bytes!
    if (parser.bytesAvailable() < (((bomLength + str.max_size()) - 1U) + terminationLength))
    {
        parser.setFailure();
        return;
    }

    // read in the BOM
    if (!parseBom(parser, bom, bomLength))
    {
        return;
    }

    if (encoding == Encoding::SOMEIP_ENCODING_UTF8)
    {
        readUtf8Bytes(parser, str, terminationLength);
    }
    else
    {
        readUtf16Bytes(parser, str, terminationLength);
    }
}

/*
 * StaticStringSerializer
 */
// static
void StaticStringSerializer::writeString(
    SomeIpSerializer& serializer, ::etl::istring const& str, Encoding const encoding)
{
    if (!serializer.isGood())
    {
        return;
    }

    uint32_t const terminationLength
        = ::someip::internal::EncodingHelper::terminationLength(encoding);
    uint32_t const nulls
        = static_cast<uint32_t>((str.max_size() - str.size()) - 1U) + terminationLength;

    writeStringBytes(serializer, str, encoding, nulls);
}

/*
 * DynamicStringParserBase
 */
void DynamicStringParserBase::parseStringBytes(
    SomeIpParser& parser, ::etl::istring& str, Encoding const encoding, uint32_t const numBytes)
{
    char const* const bom    = ::someip::internal::EncodingHelper::bom(encoding);
    uint32_t const bomLength = ::someip::internal::EncodingHelper::encodingLength(encoding);
    uint32_t const terminationLength
        = ::someip::internal::EncodingHelper::terminationLength(encoding);

    // verify we have the correct BOM
    if (!parseBom(parser, bom, bomLength))
    {
        return;
    }

    uint32_t const usefulBytes = numBytes - bomLength - terminationLength;

    if (usefulBytes >= str.max_size())
    {
        parser.setFailure();
        return;
    }

    char ch = '\0';

    for (size_t i = 0U; i < usefulBytes; ++i)
    {
        parser >> ch;
        str += ch;
    }

    readAndSkipNulls(parser, str, terminationLength);
}

} // namespace someip
