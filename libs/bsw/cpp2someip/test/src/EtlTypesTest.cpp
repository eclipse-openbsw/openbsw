/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/EtlArray.h"
#include "someip/EtlString.h"
#include "someip/EtlVector.h"

#include <etl/unaligned_type.h>
#include <gmock/gmock.h>

namespace someip
{
using Vector5    = ::etl::vector<uint8_t, 5U>;
using Vector5x10 = ::etl::vector<Vector5, 10U>;

template<>
class DynamicArrayParser<Vector5, uint32_t>
{
public:
    static void parse(
        Vector5x10& obj,
        SomeIpParser& parser,
        size_t /* minSize */,
        size_t /* maxSize */,
        bool /* isBigEndian */)
    {
        obj.clear();

        uint32_t numBytes = LengthParserHelper<uint32_t>::parseLength(parser);
        if (numBytes == 0U)
        {
            return;
        }

        size_t pos         = 0U;
        size_t endPosition = parser.getCurrentPosition() + size_t(numBytes);
        while (parser.getCurrentPosition() < endPosition)
        {
            if (pos >= 10U)
            {
                // fatal. Trying to read in too many values!
                obj.clear();
                parser.setFailure();
                return;
            }

            Vector5 element;
            obj.push_back(element);
            DynamicArrayParser<uint8_t, uint32_t>::parse(obj.back(), parser, 0U, true);
            pos++;
        }

        if (!pos)
        {
            parser.setFailure();
            obj.clear();
        }
    }
};

template<>
class DynamicArraySerializer<Vector5, uint32_t>
{
public:
    static void serialize(
        Vector5x10 const& obj,
        SomeIpSerializer& serializer,
        size_t /* minSize */,
        size_t /* maxSize */,
        bool /* isBigEndian */)
    {
        LengthSerializerHelper<uint32_t> helper(serializer);

        uint32_t length = obj.size();

        // write out each element
        for (uint32_t i = 0U; i < length; ++i)
        {
            DynamicArraySerializer<uint8_t, uint32_t>::serialize(obj[i], serializer, 0U, true);
        }
    }
};

} // namespace someip

namespace
{
using namespace ::someip;
using namespace ::testing;

TEST(EtlTypes, SerializeVectorInvalidSerializer)
{
    ::etl::vector<uint8_t, 10U> v;
    v.push_back(1U);
    v.push_back(2U);

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    set.setFailure();

    DynamicArraySerializer<uint8_t, uint32_t>::serialize(v, set, 0U, true);

    EXPECT_THAT(buf, Each(Eq(0U)));
}

TEST(EtlTypes, SerializeVectorUInt8)
{
    ::etl::vector<uint8_t, 10U> v;
    v.push_back(1U);
    v.push_back(2U);

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    DynamicArraySerializer<uint8_t, uint32_t>::serialize(v, set, 0U, true);

    EXPECT_EQ(2U, etl::be_uint32_t{&buf[0]});
    EXPECT_EQ(1U, buf[4]);
    EXPECT_EQ(2U, buf[5]);
}

TEST(EtlTypes, SerializeVectorUInt8_NotEnoughElements)
{
    ::etl::vector<uint8_t, 10U> v;
    v.push_back(1U);
    v.push_back(2U);

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    // expecting 3 items but only 2 are present.
    DynamicArraySerializer<uint8_t, uint32_t>::serialize(v, set, 3U, true);

    EXPECT_FALSE(set.isGood());
}

TEST(EtlTypes, ParseVectorUInt8)
{
    ::etl::vector<uint8_t, 10U> v;
    v.push_back(1U);
    v.push_back(2U);

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    DynamicArraySerializer<uint8_t, uint32_t>::serialize(v, set, 0U, true);

    SomeIpParser parser(buf);
    ::etl::vector<uint8_t, 5U> v1;
    DynamicArrayParser<uint8_t, uint32_t>::parse(v1, parser, 0U, true);
    EXPECT_EQ(2U, v1.size());
    EXPECT_EQ(1U, v1[0]);
    EXPECT_EQ(2U, v1[1]);
}

TEST(EtlTypes, ParseVectorUInt16)
{
    ::etl::vector<uint16_t, 5U> a;
    a.push_back(0x1020U);
    a.push_back(0x2345U);

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    // write out big endian
    DynamicArraySerializer<uint16_t, uint16_t>::serialize(a, set, 2U, true);

    // now write out little endian
    DynamicArraySerializer<uint16_t, uint16_t>::serialize(a, set, 2U, false);

    SomeIpParser parser(buf);
    ::etl::vector<uint16_t, 5U> a1, a2;
    DynamicArrayParser<uint16_t, uint16_t>::parse(a1, parser, 2U, true);
    DynamicArrayParser<uint16_t, uint16_t>::parse(a2, parser, 2U, false);
    EXPECT_EQ(0x1020U, a1[0]);
    EXPECT_EQ(0x2345U, a1[1]);
    EXPECT_EQ(0x1020U, a2[0]);
    EXPECT_EQ(0x2345U, a2[1]);
}

TEST(EtlTypes, ParseVectorUInt8BadParser)
{
    uint8_t buf[20] = {0U};
    SomeIpParser parser(buf);

    parser.setFailure();

    ::etl::vector<uint8_t, 5U> v1;
    v1.push_back(1U);

    // bad parser, clears out the vector
    DynamicArrayParser<uint8_t, uint32_t>::parse(v1, parser, 0U, true);
    EXPECT_EQ(0U, v1.size());
}

TEST(EtlTypes, ParseVectorUInt8NotEnoughBytes)
{
    uint8_t buf[20] = {128U};
    SomeIpParser parser(buf);

    ::etl::vector<uint8_t, 5U> v1;
    v1.push_back(1U);

    // not enough bytes, it'll clear out the vector
    DynamicArrayParser<uint8_t, uint8_t>::parse(v1, parser, 0U, true);
    EXPECT_EQ(0U, v1.size());
}

TEST(EtlTypes, ParseVectorUInt8TooManyElements)
{
    ::etl::vector<uint8_t, 10U> v;
    for (uint32_t i = 0U; i < 10U; ++i)
    {
        v.push_back(i);
    }

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    DynamicArraySerializer<uint8_t, uint32_t>::serialize(v, set, 0U, true);

    SomeIpParser parser(buf);

    ::etl::vector<uint8_t, 5U> v1;
    v1.push_back(1U);

    // too many elements it'll clear out the vector
    DynamicArrayParser<uint8_t, uint32_t>::parse(v1, parser, 0U, true);
    EXPECT_EQ(0U, v1.size());
}

TEST(EtlTypes, ParseVectorUInt8MinLength)
{
    ::etl::vector<uint8_t, 10U> v;
    v.push_back(1U);

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    DynamicArraySerializer<uint8_t, uint32_t>::serialize(v, set, 0U, true);

    SomeIpParser parser(buf);

    ::etl::vector<uint8_t, 5U> v1;
    v1.push_back(1U);

    // not enough elements it'll clear out the vector
    DynamicArrayParser<uint8_t, uint32_t>::parse(v1, parser, 4U, true);
    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ(0U, v1.size());
}

class CustomObject
{
public:
    CustomObject() = default;
};

[[maybe_unused]] static void operator>>(SomeIpParser& parser, CustomObject& /* obj */)
{
    parser.setFailure();
}

TEST(EtlTypes, ParseVectorNotEnoughBytes)
{
    uint8_t const buf[7] = {0U, 0U, 0U, 2U, 1U, 2U};
    SomeIpParser parser(buf);

    ::etl::vector<CustomObject, 5U> v1;
    v1.push_back(CustomObject());

    // not enough bytes it'll clear out the vector
    DynamicArrayParser<CustomObject, uint32_t>::parse(v1, parser, 0U, true);
    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ(0U, v1.size());
}

TEST(EtlTypes, SerializeArrayBadSerializer)
{
    ::etl::array<uint8_t, 10U> a{};
    for (size_t i = 0U; i < a.size(); ++i)
    {
        a[i] = i;
    }

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    set.setFailure();
    ArraySerializer<uint8_t, 10U>::serialize(a, set, true);

    EXPECT_THAT(buf, Each(Eq(0U)));
}

TEST(EtlTypes, SerializeArrayUInt8)
{
    ::etl::array<uint8_t, 10U> a{};
    a[0] = 1U;
    a[1] = 2U;

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    ArraySerializer<uint8_t, 10U>::serialize(a, set, true);

    EXPECT_EQ(1U, buf[0]);
    EXPECT_EQ(2U, buf[1]);
}

TEST(EtlTypes, ParseArrayUInt8)
{
    ::etl::array<uint8_t, 10U> a{};
    a[0] = 1U;
    a[1] = 2U;

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    ArraySerializer<uint8_t, 10U>::serialize(a, set, true);

    SomeIpParser parser(buf);
    ::etl::array<uint8_t, 10U> a1{};
    ArrayParser<uint8_t, 10U>::parse(a1, parser, true);
    EXPECT_EQ(1U, a1[0]);
    EXPECT_EQ(2U, a1[1]);
}

TEST(EtlTypes, ParseArrayUInt16)
{
    ::etl::array<uint16_t, 2U> a{};
    a[0] = 0x1020U;
    a[1] = 0x2345U;

    uint8_t buf[20] = {0U};
    SomeIpSerializer set(buf);

    // write out big endian
    ArraySerializer<uint16_t, 2U>::serialize(a, set, true);

    // now write out little endian
    ArraySerializer<uint16_t, 2U>::serialize(a, set, false);

    SomeIpParser parser(buf);
    ::etl::array<uint16_t, 2U> a1{}, a2{};
    ArrayParser<uint16_t, 2U>::parse(a1, parser, true);
    ArrayParser<uint16_t, 2U>::parse(a2, parser, false);
    EXPECT_EQ(0x1020U, a1[0]);
    EXPECT_EQ(0x2345U, a1[1]);
    EXPECT_EQ(0x1020U, a2[0]);
    EXPECT_EQ(0x2345U, a2[1]);
}

TEST(EtlTypes, ParseArrayUInt8BadParser)
{
    uint8_t buf[20] = {0U};
    SomeIpParser parser(buf);

    parser.setFailure();

    ::etl::array<uint8_t, 5U> a1{};
    a1[0] = 1U;

    // bad parser, doesn't clear out the vector
    ArrayParser<uint8_t, 5>::parse(a1, parser, true);
    EXPECT_EQ(1U, a1[0]);
}

TEST(EtlTypes, ParseArrayUInt8NotEnoughBytes)
{
    uint8_t buf[4] = {0U};
    SomeIpParser parser(buf);

    ::etl::array<uint8_t, 6U> a1{};
    a1[4] = 1U;

    // not enough bytes, it'll clear out the vector
    ArrayParser<uint8_t, 6U>::parse(a1, parser, true);
    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ(0U, a1[0]);
    EXPECT_EQ(1U, a1[4]);
}

TEST(EtlTypes, GetSizeHelperArray)
{
    ::etl::array<uint8_t, 5U> a{};
    for (size_t i = 0U; i < 5U; ++i)
    {
        a[i] = i;
    }

    EXPECT_EQ(5U, (::someip::internal::GetSizeHelper<::etl::array<uint8_t, 5U>>::getSize(a, 10U)));
    EXPECT_EQ(
        15U,
        (::someip::internal::GetSizeHelper<::etl::array<uint8_t, 5U>>::getSize(
            a, 10U, ::someip::Encoding::SOMEIP_ENCODING_UTF8)));
}

TEST(EtlTypes, GetSizeHelperVector)
{
    ::etl::vector<uint8_t, 5U> a;
    for (size_t i = 0U; i < 4U; ++i)
    {
        a.push_back(i);
    }

    EXPECT_EQ(
        14U, (::someip::internal::GetSizeHelper<::etl::vector<uint8_t, 5U>>::getSize(a, 10U)));
    EXPECT_EQ(
        14U,
        (::someip::internal::GetSizeHelper<::etl::vector<uint8_t, 5U>>::getSize(
            a, 10U, ::someip::Encoding::SOMEIP_ENCODING_UTF8)));
}

TEST(EtlTypes, ParseNestedArray)
{
    uint8_t buf[64] = {0U};

    Vector5x10 orig;
    Vector5 nested;
    nested.push_back(2U);
    orig.push_back(nested);

    SomeIpSerializer serializer(buf);

    DynamicArraySerializer<Vector5, uint32_t>::serialize(orig, serializer, 0U, 10U, true);

    SomeIpParser parser(buf);

    Vector5x10 v;

    DynamicArrayParser<Vector5, uint32_t>::parse(v, parser, 0U, 10U, true);
    EXPECT_EQ(1U, v.size());
    Vector5 const& first = v[0];
    EXPECT_EQ(1U, first.size());
    EXPECT_EQ(2U, first[0]);
}

TEST(EtlTypes, SerializeDynamicString_InvalidSerializer)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    set.setFailure();
    ::etl::string<7U> str("Hello");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    EXPECT_THAT(buf, Each(Eq(0U)));
}

TEST(EtlTypes, SerializeUTF8FixedLengthString)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hello");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    //\xEF\xBB\xBF
    // validate BOM
    EXPECT_EQ(0xEF, buf[0]);
    EXPECT_EQ(0xBB, buf[1]);
    EXPECT_EQ(0xBF, buf[2]);
    EXPECT_EQ('H', buf[3]);
    EXPECT_EQ('e', buf[4]);
    EXPECT_EQ('l', buf[5]);
    EXPECT_EQ('l', buf[6]);
    EXPECT_EQ('o', buf[7]);
    EXPECT_EQ('\0', buf[8]);
    EXPECT_EQ('\0', buf[9]);
}

TEST(EtlTypes, UTF8StaticStringNoZeroTerminator)
{
    uint8_t buf[15] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<8U> str("Hello12");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8_NO_ZERO);
    EXPECT_EQ(10U, set.getCurrentPosition());
    //\xEF\xBB\xBF
    // validate BOM
    EXPECT_EQ(0xEF, buf[0]);
    EXPECT_EQ(0xBB, buf[1]);
    EXPECT_EQ(0xBF, buf[2]);
    EXPECT_EQ('H', buf[3]);
    EXPECT_EQ('e', buf[4]);
    EXPECT_EQ('l', buf[5]);
    EXPECT_EQ('l', buf[6]);
    EXPECT_EQ('o', buf[7]);
    EXPECT_EQ('1', buf[8]);
    EXPECT_EQ('2', buf[9]);

    SomeIpParser parser(buf);

    ::etl::string<8> str1;

    StaticStringParser::readString(parser, str1, ::someip::Encoding::SOMEIP_ENCODING_UTF8_NO_ZERO);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(10U, parser.getCurrentPosition());

    EXPECT_EQ(7U, str1.size());
    EXPECT_EQ("Hello12", str1);
}

TEST(EtlTypes, StaticStringNoBOMNoZeroTerminator)
{
    uint8_t buf[15] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<8U> str("Hello12");

    StaticStringSerializer::writeString(
        set, str, ::someip::Encoding::SOMEIP_ENCODING_NO_BOM_NO_ZERO);
    EXPECT_EQ(7U, set.getCurrentPosition());

    EXPECT_EQ('H', buf[0]);
    EXPECT_EQ('e', buf[1]);
    EXPECT_EQ('l', buf[2]);
    EXPECT_EQ('l', buf[3]);
    EXPECT_EQ('o', buf[4]);
    EXPECT_EQ('1', buf[5]);
    EXPECT_EQ('2', buf[6]);

    SomeIpParser parser(buf);

    ::etl::string<8U> str1;

    StaticStringParser::readString(
        parser, str1, ::someip::Encoding::SOMEIP_ENCODING_NO_BOM_NO_ZERO);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(7U, parser.getCurrentPosition());
    EXPECT_EQ(7U, str1.size());
    EXPECT_EQ("Hello12", str1);
}

TEST(EtlTypes, WriteStringInvalidEncoding)
{
    uint8_t buf[15] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<8U> str("Hello42");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UNKNOWN);
}

TEST(EtlTypes, StaticStringNoBOMTerminator)
{
    uint8_t buf[15] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<8U> str("Hello12");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_NO_BOM);
    EXPECT_EQ(8U, set.getCurrentPosition());

    EXPECT_EQ('H', buf[0]);
    EXPECT_EQ('e', buf[1]);
    EXPECT_EQ('l', buf[2]);
    EXPECT_EQ('l', buf[3]);
    EXPECT_EQ('o', buf[4]);
    EXPECT_EQ('1', buf[5]);
    EXPECT_EQ('2', buf[6]);
    EXPECT_EQ('\0', buf[7]);

    SomeIpParser parser(buf);

    ::etl::string<8U> str1;

    StaticStringParser::readString(parser, str1, ::someip::Encoding::SOMEIP_ENCODING_NO_BOM);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(8U, parser.getCurrentPosition());
    EXPECT_EQ(7U, str1.size());
    EXPECT_EQ("Hello12", str1);
}

TEST(EtlTypes, SerializeUTF8DynamicLengthString)
{
    uint8_t buf[13] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hello");

    DynamicStringSerializer<uint32_t>::writeString(
        set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    //\xEF\xBB\xBF
    // validate BOM
    EXPECT_EQ(0U, buf[0]);
    EXPECT_EQ(0U, buf[1]);
    EXPECT_EQ(0U, buf[2]);
    EXPECT_EQ(9U, buf[3]);
    EXPECT_EQ(0xEF, buf[4]);
    EXPECT_EQ(0xBB, buf[5]);
    EXPECT_EQ(0xBF, buf[6]);
    EXPECT_EQ('H', buf[7]);
    EXPECT_EQ('e', buf[8]);
    EXPECT_EQ('l', buf[9]);
    EXPECT_EQ('l', buf[10]);
    EXPECT_EQ('o', buf[11]);
    EXPECT_EQ('\0', buf[12]);
}

TEST(EtlTypes, ParseUTF8StaticString)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hello");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);

    SomeIpParser parser(buf);

    ::etl::string<7U> str1;

    StaticStringParser::readString(parser, str1, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(5U, str1.size());
    EXPECT_EQ("Hello", str1);
}

TEST(EtlTypes, ParseUTF8DynamicString)
{
    uint8_t buf[13] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7> str("Hello");

    DynamicStringSerializer<uint32_t>::writeString(
        set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);

    SomeIpParser parser(buf);

    ::etl::string<7U> str1;

    DynamicStringParser<uint32_t>::readString(
        parser, str1, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(5U, str1.size());
    EXPECT_EQ("Hello", str1);
}

TEST(EtlTypes, ParseUTF8DynamicString_StringExactSize)
{
    uint8_t buf[15] = {0U};

    uint32_t pos = 0U;
    buf[pos++]   = 0U;
    buf[pos++]   = 0U;
    buf[pos++]   = 0U;
    buf[pos++]   = 10U;
    buf[pos++]   = 0xEF;
    buf[pos++]   = 0xBB;
    buf[pos++]   = 0xBF;
    buf[pos++]   = 'H';
    buf[pos++]   = 'e';
    buf[pos++]   = 'l';
    buf[pos++]   = 'l';
    buf[pos++]   = 'o';
    buf[pos++]   = 'o';
    buf[pos++]   = '\0';

    SomeIpParser parser(buf);

    ::etl::string<7U> str;
    DynamicStringParser<uint32_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(6U, str.size());
    EXPECT_EQ("Helloo", str);
}

TEST(EtlTypes, ParseUTF8DynamicString_StringTooSmall)
{
    uint8_t buf[15] = {0U};

    uint32_t pos = 0U;
    buf[pos++]   = 0U;
    buf[pos++]   = 0U;
    buf[pos++]   = 0U;
    buf[pos++]   = 11U;
    buf[pos++]   = 0xEF;
    buf[pos++]   = 0xBB;
    buf[pos++]   = 0xBF;
    buf[pos++]   = 'H';
    buf[pos++]   = 'e';
    buf[pos++]   = 'l';
    buf[pos++]   = 'l';
    buf[pos++]   = 'o';
    buf[pos++]   = 'o';
    buf[pos++]   = 'o';
    buf[pos++]   = '\0';

    SomeIpParser parser(buf);

    ::etl::string<7U> str;
    DynamicStringParser<uint32_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    EXPECT_FALSE(parser.isGood());
}

TEST(EtlTypes, SerializeUTF16LEString)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hell");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);
    // \xFF\xFE
    // validate BOM
    EXPECT_EQ(0xFF, buf[0]);
    EXPECT_EQ(0xFE, buf[1]);
    EXPECT_EQ('H', buf[2]);
    EXPECT_EQ('e', buf[3]);
    EXPECT_EQ('l', buf[4]);
    EXPECT_EQ('l', buf[5]);
    EXPECT_EQ('\0', buf[6]);
    EXPECT_EQ('\0', buf[7]);
    EXPECT_EQ('\0', buf[8]);
    EXPECT_EQ('\0', buf[9]);
}

TEST(EtlTypes, ParseUTF16LEString)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hell");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    SomeIpParser parser(buf);

    ::etl::string<7U> str1;

    StaticStringParser::readString(parser, str1, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(4U, str1.size());
    EXPECT_EQ("Hell", str1);
}

TEST(EtlTypes, SerializeUTF16BEString)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hell");

    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);
    // \xFE\xFF
    // validate BOM
    EXPECT_EQ(0xFE, buf[0]);
    EXPECT_EQ(0xFF, buf[1]);
    EXPECT_EQ('H', buf[2]);
    EXPECT_EQ('e', buf[3]);
    EXPECT_EQ('l', buf[4]);
    EXPECT_EQ('l', buf[5]);
    EXPECT_EQ('\0', buf[6]);
    EXPECT_EQ('\0', buf[7]);
    EXPECT_EQ('\0', buf[8]);
    EXPECT_EQ('\0', buf[9]);
}

TEST(EtlTypes, ParseUTF16BEString)
{
    uint8_t buf[10] = {0U};
    SomeIpSerializer set(buf);

    ::etl::string<7U> str("Hell");
    StaticStringSerializer::writeString(set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);

    SomeIpParser parser(buf);

    ::etl::string<7U> str1;
    StaticStringParser::readString(parser, str1, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(4U, str1.size());
    EXPECT_EQ("Hell", str1);
}

TEST(EtlTypes, StaticStringParserInvalidAtParse)
{
    uint8_t const buf[10] = {0U};
    SomeIpParser parser(buf);
    parser.setFailure();

    ::etl::string<7U> str("Hell");
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("Hell", str);
}

TEST(EtlTypes, StaticStringInvalidBOMBytes)
{
    uint8_t const buf[10] = {'a'}; // not a BOM byte
    SomeIpParser parser(buf);

    ::etl::string<7U> str("Hell");
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, StaticStringUTF8NotEnoughBytes)
{
    uint8_t const buf[4] = {0xEF, 0xBB, 0xBF, 'a'};
    SomeIpParser parser(buf);

    ::etl::string<7U> str("Hell");
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, StaticStringUTF16NotEnoughBytes)
{
    uint8_t const buf[3] = {0xFF, 0xFE, 'a'};
    SomeIpParser parser(buf);

    ::etl::string<7U> str("Hell");
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, StaticStringUTF8JustEnoughBytes)
{
    uint8_t buf[10] = {0U};

    buf[0] = 0xEF;
    buf[1] = 0xBB;
    buf[2] = 0xBF;
    buf[3] = 'a';
    buf[4] = 'b';
    buf[5] = 'c';
    buf[6] = 'd';
    buf[7] = 'e';
    buf[8] = 'f';
    buf[9] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ("abcdef", str);
}

TEST(EtlTypes, StaticStringUTF16JustEnoughBytes)
{
    uint8_t buf[10] = {0U};

    buf[0] = 0xFF;
    buf[1] = 0xFE;
    buf[2] = 'a';
    buf[3] = 'b';
    buf[4] = 'c';
    buf[5] = 'd';
    buf[6] = 'e';
    buf[7] = 'f';
    buf[8] = '\0';
    buf[9] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ("abcdef", str);
}

TEST(EtlTypes, DynamicStringParserInvalidAtParse)
{
    uint8_t buf[10] = {0U};
    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    parser.setFailure();

    DynamicStringParser<uint32_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("Hell", str);
}

TEST(EtlTypes, DynamicStringSerializerInvalid)
{
    uint8_t buf[13] = {0U};
    SomeIpSerializer set(buf);

    set.setFailure();

    ::etl::string<7U> str("Hello");

    // shouldn't do anything
    DynamicStringSerializer<uint32_t>::writeString(
        set, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);
    EXPECT_EQ(0U, set.getCurrentPosition());
}

TEST(EtlTypes, DynamicStringZeroLength)
{
    uint8_t buf[10] = {0U};
    buf[0]          = 0U;
    buf[1]          = 0U;
    buf[2]          = 0U;
    buf[3]          = 0U;

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);

    DynamicStringParser<uint32_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16BE);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, DynamicStringInvalidBOMBytes)
{
    uint8_t buf[10] = {0U};

    buf[0] = 5U;
    buf[1] = 'a'; // not a BOM byte
    buf[2] = 0xFE;
    buf[3] = 'a';
    buf[4] = '\0';
    buf[5] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    DynamicStringParser<uint8_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, DynamicStringUTF8JustEnoughBytes)
{
    uint8_t buf[11] = {0U};

    buf[0]  = 10U;
    buf[1]  = 0xEF;
    buf[2]  = 0xBB;
    buf[3]  = 0xBF;
    buf[4]  = 'a';
    buf[5]  = 'b';
    buf[6]  = 'c';
    buf[7]  = 'd';
    buf[8]  = 'e';
    buf[9]  = 'f';
    buf[10] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    DynamicStringParser<uint8_t>::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF8);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ("abcdef", str);
}

TEST(EtlTypes, DynamicStringUTF16JustEnoughBytes)
{
    uint8_t buf[11] = {0U};

    buf[0]  = 10U;
    buf[1]  = 0xFF;
    buf[2]  = 0xFE;
    buf[3]  = 'a';
    buf[4]  = 'b';
    buf[5]  = 'c';
    buf[6]  = 'd';
    buf[7]  = 'e';
    buf[8]  = 'f';
    buf[9]  = '\0';
    buf[10] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    DynamicStringParser<uint8_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ("abcdef", str);
}

TEST(EtlTypes, StaticStringUTF16EmbeddedNulls)
{
    uint8_t buf[10] = {0U};

    buf[0] = 0xFF;
    buf[1] = 0xFE;
    buf[2] = 'a';
    buf[3] = '\0';
    buf[4] = 'c';
    buf[5] = 'd';
    buf[6] = 'e';
    buf[7] = 'f';
    buf[8] = '\0';
    buf[9] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(6U, str.size());
    EXPECT_EQ('a', str[0]);
    EXPECT_EQ('\0', str[1]);
    EXPECT_EQ('c', str[2]);
    EXPECT_EQ('d', str[3]);
    EXPECT_EQ('e', str[4]);
    EXPECT_EQ('f', str[5]);
}

TEST(EtlTypes, StaticStringUTF16InvalidNullChars)
{
    uint8_t buf[10] = {0U};

    buf[0] = 0xFF;
    buf[1] = 0xFE;
    buf[2] = 'a';
    buf[3] = 'b';
    buf[4] = 'c';
    buf[5] = 'd';
    buf[6] = 'e';
    buf[7] = 'f';
    buf[8] = '\0';
    buf[9] = 'd';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    StaticStringParser::readString(parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, DynamicStringUTF16NotEnoughBytes)
{
    uint8_t buf[11] = {0U};

    buf[0]  = 11U;
    buf[1]  = 0xFF;
    buf[2]  = 0xFE;
    buf[3]  = 'a';
    buf[4]  = 'b';
    buf[5]  = 'c';
    buf[6]  = 'd';
    buf[7]  = 'e';
    buf[8]  = 'f';
    buf[9]  = '\0';
    buf[10] = '\0';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    DynamicStringParser<uint8_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, DynamicStringUTF16InvalidNullBytes)
{
    uint8_t buf[11] = {0U};

    buf[0]  = 10U;
    buf[1]  = 0xFF;
    buf[2]  = 0xFE;
    buf[3]  = 'a';
    buf[4]  = 'b';
    buf[5]  = 'c';
    buf[6]  = 'd';
    buf[7]  = 'e';
    buf[8]  = 'f';
    buf[9]  = '\0';
    buf[10] = 'a';

    ::etl::string<7U> str("Hell");
    SomeIpParser parser(buf);
    DynamicStringParser<uint8_t>::readString(
        parser, str, ::someip::Encoding::SOMEIP_ENCODING_UTF16LE);

    EXPECT_FALSE(parser.isGood());
    EXPECT_EQ("", str);
}

TEST(EtlTypes, LengthSerializerHelper)
{
    uint8_t buf[100] = {0U};
    SomeIpSerializer set(buf);

    {
        LengthSerializerHelper<uint8_t> h1(set);
        set << uint8_t(1);
        set << uint8_t(2);

        {
            LengthSerializerHelper<uint8_t> h2(set);
            set << uint8_t(3);
            set << uint8_t(4);
        }
    }

    EXPECT_EQ(6U, set.getCurrentPosition());
    EXPECT_EQ(5U, buf[0]);
    EXPECT_EQ(1U, buf[1]);
    EXPECT_EQ(2U, buf[2]);
    EXPECT_EQ(2U, buf[3]);
    EXPECT_EQ(3U, buf[4]);
    EXPECT_EQ(4U, buf[5]);
}

TEST(EtlTypes, LengthParserHelper_InvalidParser)
{
    uint8_t buf[100] = {0U};
    SomeIpParser parser(buf);

    parser.setFailure();

    EXPECT_EQ(0U, LengthParserHelper<uint16_t>::parseLength(parser));
}

} // anonymous namespace
