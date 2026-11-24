/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/bindings/someip/SomeIpCodec.h"

#include <etl/array.h>
#include <etl/vector.h>
#include <gmock/gmock.h>

namespace
{

using ::middleware::bindings::someip::SomeIpSerializableValue;
using ::someip::SomeIpParser;
using ::someip::SomeIpSerializer;
namespace codec = ::middleware::bindings::someip::codec;

using Array3u16  = ::etl::array<uint16_t, 3U>;
using Array4u8   = ::etl::array<uint8_t, 4U>;
using Vector8u16 = ::etl::vector<uint16_t, 8U>;
using Row4u16    = ::etl::vector<uint16_t, 4U>;
using Rows4u16   = ::etl::vector<Row4u16, 4U>;
using Rows1u16   = ::etl::vector<Row4u16, 1U>;
using String6    = ::etl::array<uint8_t, 6U>;
using Bytes16    = ::etl::vector<uint8_t, 16U>;

struct Array3u16Tag
{
    using Value = Array3u16;
};

/** Member 1 is a uint16_t, member 2 a uint32_t. */
struct UnionMembers
{
    static bool isMember(uint32_t const type, uint32_t const length)
    {
        return ((type == 1U) && (length == 2U)) || ((type == 2U) && (length == 4U));
    }

    static uint32_t correct(uint32_t const length)
    {
        if (length == 2U)
        {
            return 1U;
        }
        if (length == 4U)
        {
            return 2U;
        }
        return 0U;
    }
};

using UnionCodec = codec::Union<Bytes16, UnionMembers>;

} // namespace

namespace middleware::bindings::someip
{
template<>
struct SomeIpCodec<Array3u16Tag> : codec::FixedArray<Array3u16, uint16_t, 3U>
{};
} // namespace middleware::bindings::someip

namespace
{

using namespace ::testing;

TEST(SomeIpCodec, ScalarIsStreamedDirectly)
{
    SomeIpSerializableValue<uint16_t> const value(0x1234U);
    EXPECT_EQ(2U, value.getSize());

    uint8_t buf[2] = {0U};
    SomeIpSerializer serializer(buf);
    value.serializeToArray(serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_THAT(buf, ElementsAre(0x12U, 0x34U));

    SomeIpSerializableValue<uint16_t> parsed;
    SomeIpParser parser(buf);
    parsed.parseFromArray(parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(0x1234U, parsed.get());
}

TEST(SomeIpCodec, SerializesAnExternalValueInPlaceOfTheHeldOne)
{
    SomeIpSerializableValue<uint16_t> value(0x1111U);
    uint16_t const external = 0xABCDU;
    uint8_t buf[2]          = {0U};

    value.serializeFrom(&external);
    SomeIpSerializer fromExternal(buf);
    value.serializeToArray(fromExternal);
    EXPECT_THAT(buf, ElementsAre(0xABU, 0xCDU));

    value.serializeFrom(nullptr);
    SomeIpSerializer fromHeld(buf);
    value.serializeToArray(fromHeld);
    EXPECT_THAT(buf, ElementsAre(0x11U, 0x11U));
}

TEST(SomeIpCodec, BytesLeftAfterTheValueAreMalformed)
{
    uint8_t const received[] = {0x12U, 0x34U, 0x56U};

    SomeIpSerializableValue<uint16_t> parsed;
    SomeIpParser parser(received);
    parsed.parseFromArray(parser);
    EXPECT_FALSE(parser.isGood());
}

TEST(SomeIpCodec, GetSizeIsTheCodecSizeInBytes)
{
    SomeIpSerializableValue<Array3u16, Array3u16Tag> const value(Array3u16{1U, 2U, 3U});
    EXPECT_EQ(6U, value.getSize());

    uint8_t buf[6] = {0U};
    SomeIpSerializer serializer(buf);
    value.serializeToArray(serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(value.getSize(), serializer.getCurrentPosition());
}

TEST(SomeIpCodec, FixedArrayOfBytes)
{
    using Codec = codec::FixedArray<Array4u8, uint8_t, 4U>;
    Array4u8 const value{1U, 2U, 3U, 4U};

    uint8_t buf[4] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(4U, Codec::size(value));
    EXPECT_THAT(buf, ElementsAre(1U, 2U, 3U, 4U));
}

TEST(SomeIpCodec, FixedArraySizeIsInBytes)
{
    using Codec = codec::FixedArray<Array3u16, uint16_t, 3U>;
    Array3u16 const value{0x0102U, 0x0304U, 0x0506U};

    uint8_t buf[6] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(6U, Codec::size(value));
    EXPECT_THAT(buf, ElementsAre(0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U));

    Array3u16 parsed{};
    SomeIpParser parser(buf);
    Codec::parse(parsed, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(value, parsed);
}

TEST(SomeIpCodec, DynamicArraySizeIsInBytes)
{
    using Codec = codec::DynamicArray<Vector8u16, uint16_t, uint32_t, 0U>;
    Vector8u16 const value{0x0102U, 0x0304U, 0x0506U};

    // A 4-byte length prefix holding the 6 bytes that follow it.
    uint8_t buf[10] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(10U, Codec::size(value));
    EXPECT_THAT(buf, ElementsAre(0U, 0U, 0U, 6U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U));

    Vector8u16 parsed;
    SomeIpParser parser(buf);
    Codec::parse(parsed, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(value, parsed);
}

TEST(SomeIpCodec, DynamicArrayBelowItsMinimumLength)
{
    using Codec = codec::DynamicArray<Vector8u16, uint16_t, uint32_t, 2U>;
    Vector8u16 const tooShort{0x0102U};

    uint8_t buf[6] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(tooShort, serializer);
    EXPECT_FALSE(serializer.isGood());

    uint8_t const received[] = {0U, 0U, 0U, 2U, 0x01U, 0x02U};
    Vector8u16 parsed;
    SomeIpParser parser(received);
    Codec::parse(parsed, parser);
    EXPECT_FALSE(parser.isGood());
    EXPECT_TRUE(parsed.empty());
}

TEST(SomeIpCodec, DynamicArray2dSizeIsInBytes)
{
    using Codec = codec::DynamicArray2d<Rows4u16, uint16_t, uint32_t, uint32_t, 0U>;
    Rows4u16 value;
    value.push_back(Row4u16{0x0102U});
    value.push_back(Row4u16{0x0304U, 0x0506U});

    // The row count, then each row with the byte length of its elements.
    uint8_t const expected[] = {
        0U, 0U, 0U, 2U, 0U, 0U, 0U, 2U, 0x01U, 0x02U, 0U, 0U, 0U, 4U, 0x03U, 0x04U, 0x05U, 0x06U};
    uint8_t buf[sizeof(expected)] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(sizeof(expected), Codec::size(value));
    EXPECT_THAT(buf, ElementsAreArray(expected));

    Rows4u16 parsed;
    SomeIpParser parser(buf);
    Codec::parse(parsed, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(value, parsed);
}

TEST(SomeIpCodec, DynamicArray2dWithMoreRowsThanFitIsMalformed)
{
    using Codec = codec::DynamicArray2d<Rows1u16, uint16_t, uint32_t, uint32_t, 0U>;
    uint8_t const received[]
        = {0U, 0U, 0U, 2U, 0U, 0U, 0U, 2U, 0x01U, 0x02U, 0U, 0U, 0U, 2U, 0x03U, 0x04U};

    Rows1u16 parsed;
    SomeIpParser parser(received);
    Codec::parse(parsed, parser);
    EXPECT_FALSE(parser.isGood());
    EXPECT_TRUE(parsed.empty());
}

TEST(SomeIpCodec, FixedStringWithItsBom)
{
    using Codec = codec::FixedString<String6, 6U, codec::Encoding::Utf8>;
    String6 const value{0xEFU, 0xBBU, 0xBFU, 'o', 'k', 0U};

    uint8_t buf[6] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(6U, Codec::size(value));
    EXPECT_THAT(buf, ElementsAreArray(value));

    String6 parsed{};
    SomeIpParser parser(buf);
    Codec::parse(parsed, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(value, parsed);
}

TEST(SomeIpCodec, FixedStringWithoutItsBomIsMalformed)
{
    using Codec              = codec::FixedString<String6, 6U, codec::Encoding::Utf8>;
    uint8_t const received[] = {'n', 'o', 'b', 'o', 'm', 0U};

    String6 parsed{};
    SomeIpParser parser(received);
    Codec::parse(parsed, parser);
    EXPECT_FALSE(parser.isGood());
}

TEST(SomeIpCodec, DynamicStringWithItsBom)
{
    using Codec = codec::DynamicString<Bytes16, uint16_t, codec::Encoding::Utf16Be>;
    // The BOM, 'h', then the terminator, all UTF-16BE.
    Bytes16 const value{0xFEU, 0xFFU, 0x00U, 'h', 0x00U, 0x00U};

    uint8_t buf[8] = {0U};
    SomeIpSerializer serializer(buf);
    Codec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_EQ(8U, Codec::size(value));
    EXPECT_THAT(buf, ElementsAre(0U, 6U, 0xFEU, 0xFFU, 0x00U, 'h', 0x00U, 0x00U));

    Bytes16 parsed;
    SomeIpParser parser(buf);
    Codec::parse(parsed, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_EQ(value, parsed);
}

TEST(SomeIpCodec, DynamicStringWithAnotherEncodingsBomIsMalformed)
{
    using Codec              = codec::DynamicString<Bytes16, uint16_t, codec::Encoding::Utf16Be>;
    // The UTF-16LE BOM on a string declared UTF-16BE.
    uint8_t const received[] = {0U, 4U, 0xFFU, 0xFEU, 0x00U, 0x00U};

    Bytes16 parsed;
    SomeIpParser parser(received);
    Codec::parse(parsed, parser);
    EXPECT_FALSE(parser.isGood());
}

TEST(SomeIpCodec, BoundedScalarAboveItsMaximumIsMalformed)
{
    using Codec   = codec::BoundedScalar<uint8_t, 3U>;
    uint8_t value = 0U;

    uint8_t const atMaximum[] = {3U};
    SomeIpParser atMaximumParser(atMaximum);
    Codec::parse(value, atMaximumParser);
    EXPECT_TRUE(atMaximumParser.isGood());
    EXPECT_EQ(3U, value);
    EXPECT_EQ(1U, Codec::size(value));

    uint8_t const aboveMaximum[] = {4U};
    SomeIpParser aboveMaximumParser(aboveMaximum);
    Codec::parse(value, aboveMaximumParser);
    EXPECT_FALSE(aboveMaximumParser.isGood());
}

TEST(SomeIpCodec, UnionKeepsTheBytesItArrivedAs)
{
    // Length 6, type 1, then the 2-byte member.
    uint8_t const received[] = {0U, 0U, 0U, 6U, 0U, 0U, 0U, 1U, 0x12U, 0x34U};

    Bytes16 value;
    SomeIpParser parser(received);
    UnionCodec::parse(value, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_THAT(value, ElementsAreArray(received));
    EXPECT_EQ(sizeof(received), UnionCodec::size(value));

    uint8_t buf[sizeof(received)] = {0U};
    SomeIpSerializer serializer(buf);
    UnionCodec::serialize(value, serializer);
    EXPECT_TRUE(serializer.isGood());
    EXPECT_THAT(buf, ElementsAreArray(received));
}

TEST(SomeIpCodec, UnionWithAnUndeclaredTypeIsCorrectedByItsLength)
{
    // Type 9 is not declared; a 4-byte member can only be type 2.
    uint8_t const received[]  = {0U, 0U, 0U, 8U, 0U, 0U, 0U, 9U, 1U, 2U, 3U, 4U};
    uint8_t const corrected[] = {0U, 0U, 0U, 8U, 0U, 0U, 0U, 2U, 1U, 2U, 3U, 4U};

    Bytes16 value;
    SomeIpParser parser(received);
    UnionCodec::parse(value, parser);
    EXPECT_TRUE(parser.isGood());
    EXPECT_THAT(value, ElementsAreArray(corrected));
}

TEST(SomeIpCodec, UnionShorterThanItsTypeTagIsMalformed)
{
    uint8_t const received[] = {0U, 0U, 0U, 3U, 0U, 0U, 0U};

    Bytes16 value;
    SomeIpParser parser(received);
    UnionCodec::parse(value, parser);
    EXPECT_FALSE(parser.isGood());
}

TEST(SomeIpCodec, UnionMatchingNoDeclaredMemberIsMalformed)
{
    // An undeclared type whose 3-byte member has no declared width either.
    uint8_t const received[] = {0U, 0U, 0U, 7U, 0U, 0U, 0U, 9U, 1U, 2U, 3U};

    Bytes16 value;
    SomeIpParser parser(received);
    UnionCodec::parse(value, parser);
    EXPECT_FALSE(parser.isGood());
}

} // namespace
