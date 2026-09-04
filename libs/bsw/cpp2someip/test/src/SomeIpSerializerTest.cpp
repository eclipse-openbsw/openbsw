/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpSerializer.h"

#include "someip/SomeIpParser.h"

#include <util/meta/Bitmask.h>

#include <etl/span.h>

#include <etl/unaligned_type.h>
#include <gmock/gmock.h>

#include <cmath>

namespace
{
using namespace ::someip;
using namespace ::testing;
using namespace ::util::meta;

bool validateSerializer(SomeIpSerializer const& set, ErrorCode code, uint32_t length)
{
    EXPECT_EQ(code, set.getStatus());
    EXPECT_EQ(length, set.getCurrentPosition());
    return (code == set.getStatus()) && (length == set.getCurrentPosition());
}

/**
 * Make sure SomeIpSerializer is valid after construction.
 */
TEST(SomeIpSerializer, test_default)
{
    uint8_t buffer[20U];
    SomeIpSerializer set(buffer);
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 0U));
}

/**
 * Make sure getPayload() returns underlying payload correctly.
 */
TEST(SomeIpSerializer, test_getPayload)
{
    uint8_t buffer[10U] = {0U};
    SomeIpSerializer set(buffer);

    set << uint8_t(10U);

    ::etl::span<uint8_t const> buf = set.getPayload();
    EXPECT_EQ(1U, buf.size());
    EXPECT_EQ(10U, buf[0U]);
}

/**
 * Make sure serializer is valid after writing booleans to buffer and items have been serialized
 * correctly.
 */
TEST(SomeIpSerializer, test_writing_bool)
{
    uint8_t buffer[2U] = {0U};
    SomeIpSerializer set(buffer);

    set << true;
    set << false;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_EQ(0x1U, buffer[0]);
    EXPECT_EQ(0x0U, buffer[1]);
}

/**
 * Make sure serializer is valid after writing uint8 to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_uint8)
{
    uint8_t buffer[1U] = {0U};
    SomeIpSerializer set(buffer);

    uint8_t item = 0x10U;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 1U));
    EXPECT_EQ(0x10U, buffer[0]);

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 1U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 1U));
}

/**
 * Make sure serializer is valid after writing int8 to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_int8)
{
    uint8_t buffer[1U] = {0U};
    SomeIpSerializer set(buffer);

    int8_t item = -128;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 1U));
    EXPECT_EQ(-128, (int8_t)buffer[0]);

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 1U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 1U));
}

/**
 * Make sure serializer is valid after writing uint16 to buffer and items have been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_uint16)
{
    uint8_t buffer[sizeof(uint16_t) * 2U] = {0U};
    SomeIpSerializer set(buffer);

    uint16_t const item  = 0x1020U;
    uint16_t const item2 = 0x1122U;
    set.bigEndian();
    set << item;
    set.littleEndian();
    set << item2;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));

    EXPECT_EQ(item, etl::be_uint16_t{&buffer[0]});
    EXPECT_EQ(item2, etl::le_uint16_t{&buffer[2]});
    EXPECT_EQ(0x10U, buffer[0]);
    EXPECT_EQ(0x20U, buffer[1]);

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 4U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 4U));
}

/**
 * Make sure serializer is valid after writing int16 to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_int16)
{
    uint8_t buffer[sizeof(int16_t)] = {0U};
    SomeIpSerializer set(buffer);

    int16_t item = -32765;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_EQ(item, etl::be_int16_t{&buffer[0]});

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 2U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 2U));
}

/**
 * Make sure serializer is valid after writing uint32 to buffer and items have been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_uint32)
{
    uint8_t buffer[sizeof(uint32_t) * 2U] = {0U};
    SomeIpSerializer set(buffer);

    uint32_t const item  = 0x10203040U;
    uint32_t const item2 = 0x11223344U;
    set.bigEndian();
    set << item;
    set.littleEndian();
    set << item2;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(item, etl::be_uint32_t{&buffer[0]});
    EXPECT_EQ(item2, etl::le_uint32_t{&buffer[4]});

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 8U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 8U));
}

/**
 * Make sure serializer is valid after writing int32 to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_int32)
{
    uint8_t buffer[sizeof(int32_t)] = {0U};
    SomeIpSerializer set(buffer);

    int32_t const item = -32765;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(item, etl::be_int32_t{&buffer[0]});
    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 4U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 4U));
}

/**
 * Make sure serializer is valid after writing uint64 to buffer and items have been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_uint64)
{
    uint8_t buffer[sizeof(uint64_t) * 2U] = {0U};
    SomeIpSerializer set(buffer);

    uint64_t const item  = 0x1020304050LLU;
    uint64_t const item2 = 0x1020304060LLU;
    set.bigEndian();
    set << item;
    set.littleEndian();
    set << item2;

    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 16U));
    EXPECT_EQ(item, etl::be_uint64_t{&buffer[0]});
    EXPECT_EQ(item2, etl::le_uint64_t{&buffer[8]});
    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 16U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 16U));
}

/**
 * Make sure serializer is valid after writing int64 to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_int64)
{
    uint8_t buffer[sizeof(int64_t)] = {0U};
    SomeIpSerializer set(buffer);

    int64_t const item = -327651234;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(item, etl::be_int64_t{&buffer[0]});

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 8U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 8U));
}

TEST(SomeIpSerializer, TestSliceUint8)
{
    uint8_t serializerBuffer[sizeof(uint8_t) * 10U] = {0U};
    SomeIpSerializer set(serializerBuffer);

    uint8_t sliceBuffer[sizeof(uint8_t) * 10U] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U};
    ::etl::span<uint8_t const> buffer(sliceBuffer);
    set << buffer;

    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 10U));
    for (size_t i = 0U; i < 10U; i++)
    {
        EXPECT_EQ(buffer[i], set.getPayload()[i]);
    }
}

template<uint32_t N>
struct BinaryDigit;

/** Binary 0 */
template<>
struct BinaryDigit<0U>
{
    enum
    {
        value = 0U
    };
};

/** Binary 1 */
template<>
struct BinaryDigit<1U>
{
    enum
    {
        value = 1U
    };
};

template<uint32_t N>
struct binary
{
    enum
    {
        value = BinaryDigit<N % 10U>::value + (binary<N / 10U>::value << 1U)
    };
};

template<>
struct binary<0U>
{
    enum
    {
        value = 0U
    };
};

/**
 * Make sure serializer is valid after writing float to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_float)
{
    uint8_t buffer[sizeof(float_t)] = {0};
    SomeIpSerializer set(buffer);

    float_t const item = 0.15625F;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(binary<111110>::value, buffer[0]);
    EXPECT_EQ(binary<100000>::value, buffer[1]);
    EXPECT_EQ(binary<0>::value, buffer[2]);
    EXPECT_EQ(binary<0>::value, buffer[3]);

    SomeIpParser parser(buffer);
    float other = 1.0F;
    parser.bigEndian();
    parser >> other;
    EXPECT_EQ(other, item);

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 4U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 4U));
}

/**
 * Make sure serializer is valid after writing float positive infinity to buffer and item has been
 * serialized correctly.
 */
TEST(SomeIpSerializer, test_writing_float_positive_infinity)
{
    uint8_t buffer[sizeof(float_t)] = {0};
    SomeIpSerializer set(buffer);

    float_t const item = INFINITY;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(binary<1111111>::value, buffer[0]);
    EXPECT_EQ(binary<10000000>::value, buffer[1]);
    EXPECT_EQ(binary<0>::value, buffer[2]);
    EXPECT_EQ(binary<0>::value, buffer[3]);

    SomeIpParser parser(buffer);
    float_t other = 1.0F;
    parser.bigEndian();
    parser >> other;
    EXPECT_EQ(other, item);
}

/**
 * Make sure serializer is valid after writing float positive zero to buffer and item has been
 * serialized correctly.
 */
TEST(SomeIpSerializer, test_float_positive_zero)
{
    uint8_t buffer[sizeof(float_t)] = {0};
    SomeIpSerializer set(buffer);

    float_t item = float_t(0);
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(binary<0>::value, buffer[0]);
    EXPECT_EQ(binary<0>::value, buffer[1]);
    EXPECT_EQ(binary<0>::value, buffer[2]);
    EXPECT_EQ(binary<0>::value, buffer[3]);

    SomeIpParser parser(buffer);
    float_t other = 1.0F;
    parser.bigEndian();
    parser >> other;
    EXPECT_EQ(other, item);
}

TEST(SomeIpSerializer, test_float_negative_infinity)
{
    uint8_t buffer[sizeof(float_t)] = {0};
    SomeIpSerializer set(buffer);

    float_t item = -INFINITY;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(binary<11111111>::value, buffer[0]);
    EXPECT_EQ(binary<10000000>::value, buffer[1]);
    EXPECT_EQ(binary<0>::value, buffer[2]);
    EXPECT_EQ(binary<0>::value, buffer[3]);

    SomeIpParser parser(buffer);
    float_t other = 1.0F;
    parser.bigEndian();
    parser >> other;
    EXPECT_EQ(other, item);
}

/**
 * Make sure serializer is valid after writing float negative zero to buffer and item has been
 * serialized correctly.
 */
TEST(SomeIpSerializer, test_float_negative_zero)
{
    uint8_t buffer[sizeof(float_t)] = {0};
    SomeIpSerializer set(buffer);

    float_t item = -float_t(0);
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(binary<10000000>::value, buffer[0]);
    EXPECT_EQ(binary<0>::value, buffer[1]);
    EXPECT_EQ(binary<0>::value, buffer[2]);
    EXPECT_EQ(binary<0>::value, buffer[3]);

    SomeIpParser parser(buffer);
    float_t other = 1.0F;
    parser.bigEndian();
    parser >> other;
    EXPECT_EQ(other, item);
}

/**
 * Make sure serializer is valid after writing float NaN to buffer and item has been serialized
 * correctly.
 */
TEST(SomeIpSerializer, test_writing_float_NaN)
{
    uint8_t buffer[sizeof(float_t)] = {0};
    SomeIpSerializer set(buffer);

    float_t item = NAN;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 4U));
    uint32_t value    = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3] << 0);
    uint32_t exp      = (value >> 23) & Bitmask<uint32_t, 8>::value;
    uint32_t fraction = value & Bitmask<uint32_t, 23U>::value;
    EXPECT_EQ(0xFFU, exp);
    EXPECT_GT(fraction, 0U);

    SomeIpParser parser(buffer);
    float_t other = 1.0F;
    parser.bigEndian();
    parser >> other;
    EXPECT_NE(other, other);
}

/**
 * Make sure serializer is valid after writing double to buffer and item has been serialized
 * correctly. If there is not enough space left to write, serializer enters error state and nothing
 * will be written anymore.
 */
TEST(SomeIpSerializer, test_writing_double)
{
    uint8_t buffer[sizeof(double_t)] = {0};
    SomeIpSerializer set(buffer);

    double_t item = 3.1415;
    set.bigEndian();
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_OK, 8U));
    SomeIpParser parser(buffer);
    double_t other = 1.0;
    parser.bigEndian();
    parser >> other;
    EXPECT_EQ(other, item);

    // set the error code
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 8U));

    // nothing should change
    set << item;
    EXPECT_TRUE(validateSerializer(set, ErrorCode::SOMEIP_ERROR, 8U));
}

class DoubleTest : public ::testing::TestWithParam<double_t>
{};

/**
 * Test serializing and parsing different double values.
 */
TEST_P(DoubleTest, serialize_and_parse_different_double_values)
{
    double_t value = GetParam();
    double_t parsedValue;

    uint8_t buffer[sizeof(double_t)] = {0};

    SomeIpSerializer set(buffer);

    set.bigEndian();
    set << value;
    EXPECT_EQ(ErrorCode::SOMEIP_OK, set.getStatus());
    EXPECT_EQ(sizeof(double_t), set.getCurrentPosition());

    SomeIpParser parser(buffer);

    parser.bigEndian();
    parser >> parsedValue;

    EXPECT_FLOAT_EQ(value, parsedValue);
}

INSTANTIATE_TEST_SUITE_P(
    PrimitiveTypes,
    DoubleTest,
    ::testing::Values(
        double_t(0.0),
        double_t(1.0),
        double_t(-0.0),
        double_t(-1.0),
        double_t(100),
        double_t(-123.456),
        double_t(INFINITY),
        double_t(-INFINITY)));

/**
 * Make sure writeTypeFieldSize() recognizes field size uint8.
 */
TEST(SomeIpSerializer, test_writeTypeFieldSize_uint8)
{
    uint8_t buffer[9U] = {0U};
    SomeIpSerializer streamer(buffer);
    streamer.setTypeFieldSize(1U);
    streamer.writeTypeFieldSize(0x12U);

    uint8_t const expected[] = {0x12U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    EXPECT_THAT(buffer, ElementsAreArray(expected));
}

/**
 * Make sure writeTypeFieldSize() recognizes field size uint16.
 */
TEST(SomeIpSerializer, test_writeTypeFieldSize_uint16)
{
    uint8_t buffer[9U] = {0U};
    SomeIpSerializer streamer(buffer);
    streamer.setTypeFieldSize(2U);
    streamer.writeTypeFieldSize(0x1415U);

    uint8_t const expected[] = {0x14U, 0x15U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    EXPECT_THAT(buffer, ElementsAreArray(expected));
}

/**
 * Make sure writeTypeFieldSize() recognizes default field size uint32.
 */
TEST(SomeIpSerializer, test_writeTypeFieldSize_default)
{
    uint8_t buffer[9U] = {0U};
    SomeIpSerializer streamer(buffer);
    streamer.setTypeFieldSize(4U);
    streamer.writeTypeFieldSize(0x12131415U);

    uint8_t const expected[] = {0x12U, 0x13U, 0x14U, 0x15U, 0U, 0U, 0U, 0U, 0U};
    EXPECT_THAT(buffer, ElementsAreArray(expected));
}
} // anonymous namespace
