/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpParser.h"

#include "someip/SomeIpSerializer.h"

#include <etl/unaligned_type.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::someip;
using namespace ::testing;

bool validateParser(SomeIpParser const& parser, ErrorCode code, uint32_t length)
{
    EXPECT_EQ(code, parser.getStatus());
    EXPECT_EQ(length, parser.getCurrentPosition());
    return (code == parser.getStatus()) && (length == parser.getCurrentPosition());
}

/**
 * Make sure SomeIpParser is valid after construction.
 */
TEST(SomeIpParser, test_default)
{
    uint8_t buffer[20U];
    SomeIpParser parser(buffer);
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 0U));
}

/**
 * Make sure parser is valid after reading booleans from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_bool)
{
    uint8_t const buffer[] = {0x01U, 0x00U};

    SomeIpParser parser(buffer);

    bool a, b;
    parser >> a;
    parser >> b;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_TRUE(a);
    EXPECT_FALSE(b);
}

/**
 * Make sure parser is valid after reading uint8 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_uint8)
{
    uint8_t const buffer[] = {0x10U, 0x12U};
    SomeIpParser parser(buffer);

    uint8_t item = 0x15;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 1U));
    EXPECT_EQ(0x10U, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_EQ(0x12U, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 2U));
    EXPECT_EQ(0x12U, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 2U));
    EXPECT_EQ(0x12U, item);
}

/**
 * Make sure parser is valid after reading int8 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_int8)
{
    uint8_t const buffer[] = {(uint8_t)-128, (uint8_t)-127};

    SomeIpParser parser(buffer);

    int8_t item = 15;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 1U));
    EXPECT_EQ(-128, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_EQ(-127, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 2U));
    EXPECT_EQ(-127, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 2U));
    EXPECT_EQ(-127, item);
}

/**
 * Make sure parser is valid after reading uint16 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_uint16)
{
    uint8_t buffer[sizeof(uint16_t) * 2U];
    etl::be_uint16_ext_t{&buffer[0]} = 0x1020U;
    etl::le_uint16_ext_t{&buffer[2]} = 0x1030U;

    SomeIpParser parser(buffer);

    uint16_t item = 0x15;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_EQ(0x1020U, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(0x1030U, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 4U));
    EXPECT_EQ(0x1030U, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 4U));
    EXPECT_EQ(0x1030U, item);
}

/**
 * Make sure parser is valid after reading int16 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_int16)
{
    uint8_t buffer[sizeof(int16_t) * 2U];
    etl::be_uint16_ext_t{&buffer[0]} = (uint16_t)(-32765);
    etl::le_uint16_ext_t{&buffer[2]} = (uint16_t)(-32764);

    SomeIpParser parser(buffer);

    int16_t item = 56;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 2U));
    EXPECT_EQ(-32765, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(-32764, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 4U));
    EXPECT_EQ(-32764, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 4U));
    EXPECT_EQ(-32764, item);
}

/**
 * Make sure parser is valid after reading uint32 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_uint32)
{
    uint8_t buffer[sizeof(uint32_t) * 2U];
    etl::be_uint32_ext_t{&buffer[0]} = 0x10203040U;
    etl::le_uint32_ext_t{&buffer[4]} = 0x10203050U;

    SomeIpParser parser(buffer);

    uint32_t item = 0x15;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(0x10203040U, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(0x10203050U, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 8U));
    EXPECT_EQ(0x10203050U, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 8U));
    EXPECT_EQ(0x10203050U, item);
}

/**
 * Make sure parser is valid after reading int32 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_int32)
{
    uint8_t buffer[sizeof(int32_t) * 2U];
    etl::be_uint32_ext_t{&buffer[0]} = (uint32_t)(-327651234);
    etl::le_uint32_ext_t{&buffer[4]} = (uint32_t)(-327651235);
    SomeIpParser parser(buffer);

    int32_t item = 56;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(-327651234, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(-327651235, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 8U));
    EXPECT_EQ(-327651235, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 8U));
    EXPECT_EQ(-327651235, item);
}

/**
 * Make sure parser is valid after reading uint64 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_uint64)
{
    uint8_t buffer[sizeof(uint64_t) * 2U];
    etl::be_uint64_ext_t{&buffer[0]} = 0x1020304050LLU;
    etl::le_uint64_ext_t{&buffer[8]} = 0x1020304060LLU;

    SomeIpParser parser(buffer);

    uint64_t item = 0x15;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(0x1020304050LLU, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 16U));
    EXPECT_EQ(0x1020304060LLU, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 16U));
    EXPECT_EQ(0x1020304060LLU, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 16U));
    EXPECT_EQ(0x1020304060LLU, item);
}

/**
 * Make sure parser is valid after reading int64 from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_int64)
{
    uint8_t buffer[sizeof(int64_t) * 2U];
    etl::be_uint64_ext_t{&buffer[0]} = (uint64_t)(-327651234);
    etl::le_uint64_ext_t{&buffer[8]} = (uint64_t)(-327651235);

    SomeIpParser parser(buffer);

    int64_t item = 56;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(-327651234, item);

    parser.littleEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 16U));
    EXPECT_EQ(-327651235, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 16U));
    EXPECT_EQ(-327651235, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 16U));
    EXPECT_EQ(-327651235, item);
}

/**
 * Make sure parser is valid after reading float from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_float)
{
    uint8_t buffer[sizeof(float_t)];

    SomeIpSerializer set(buffer);
    float_t f = -1.234f;
    set << f;

    SomeIpParser parser(buffer);

    float_t item = 56.0f;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 4U));
    EXPECT_EQ(f, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 4U));
    EXPECT_EQ(f, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 4U));
    EXPECT_EQ(f, item);
}

/**
 * Make sure parser is valid after reading double from buffer and read items have been parsed
 * correctly.
 */
TEST(SomeIpParser, test_reading_double)
{
    uint8_t buffer[sizeof(double_t)];

    SomeIpSerializer set(buffer);
    double_t other = 3.1415;
    set << other;

    SomeIpParser parser(buffer);

    double_t item = 56.0;
    parser.bigEndian();
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 8U));
    EXPECT_EQ(other, item);

    // set the error code
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 8U));
    EXPECT_EQ(other, item);

    // nothing should change
    parser >> item;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_ERROR, 8U));
    EXPECT_EQ(other, item);
}

/**
 * Make sure skipping specified number of bytes using skip member function works correctly
 * if there are enough bytes to read left and sets parser to error state otherwise.
 */
TEST(SomeIpParser, test_skip_member_function)
{
    uint8_t const buffer[] = {0U, 1U, 2U, 3U, 4U};

    SomeIpParser parser(buffer);

    uint8_t a;
    parser >> a;
    EXPECT_EQ(0U, a);

    parser.skip(3U); // 1, 2, 3
    parser >> a;
    EXPECT_EQ(4U, a);

    parser.skip(2U); // not enough bytes!
    EXPECT_FALSE(parser.isGood());
}

/**
 * Make sure skipping specified number of bytes using global function works correctly for uint8.
 */
TEST(SomeIpParser, test_skip_global_function_uint8)
{
    uint8_t const buffer[] = {1U, 2U, 3U, 4U, 5U};

    SomeIpParser parser(buffer);

    skip(parser, 0U); // uint8_t
    uint8_t a;
    parser >> a;
    EXPECT_EQ(2U, a);
}

/**
 * Make sure skipping specified number of bytes using global function works correctly for uint16.
 */
TEST(SomeIpParser, test_skip_global_function_uint16)
{
    uint8_t const buffer[] = {1U, 2U, 3U, 4U, 5U};

    SomeIpParser parser(buffer);

    skip(parser, (1U << 12U)); // uint16_t
    uint8_t a;
    parser >> a;
    EXPECT_EQ(3U, a);
}

/**
 * Make sure skipping specified number of bytes using global function works correctly for uint32.
 */
TEST(SomeIpParser, test_skip_global_function_uint32)
{
    uint8_t const buffer[] = {1U, 2U, 3U, 4U, 5U};
    SomeIpParser parser(buffer);

    skip(parser, (2U << 12U)); // uint32_t
    uint8_t a;
    parser >> a;
    EXPECT_EQ(5U, a);
}

/**
 * Make sure skipping specified number of bytes using global function works correctly for uint64.
 */
TEST(SomeIpParser, test_skip_global_function_uint64)
{
    uint8_t const buffer[9U] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U};
    SomeIpParser parser(buffer);

    skip(parser, (3U << 12U)); // uint64_t
    uint8_t a;
    parser >> a;
    EXPECT_EQ(9U, a);
}

/**
 * Make sure skipping specified number of bytes using global function works correctly for default
 * case.
 */
TEST(SomeIpParser, test_skip_global_function_default)
{
    uint8_t const buffer[9U] = {0U, 0U, 0U, 1U, 2U, 3U};
    SomeIpParser parser(buffer);

    skip(parser, (4U << 12U)); // default case
    uint8_t a;
    parser >> a;
    EXPECT_EQ(3U, a);
}

/**
 * Make sure readTypeFieldSize() recognizes uint8 field size.
 */
TEST(SomeIpParser, test_readTypeFieldSize_uint8)
{
    uint8_t const buffer[9U] = {115U};
    SomeIpParser parser(buffer);
    parser.setTypeFieldSize(1U);
    EXPECT_EQ(115U, parser.readTypeFieldSize());
}

/**
 * Make sure readTypeFieldSize() recognizes uint16 field size.
 */
TEST(SomeIpParser, test_readTypeFieldSize_uint16)
{
    uint8_t const buffer[9U] = {0x14, 0x15};
    SomeIpParser parser(buffer);
    parser.setTypeFieldSize(2U);
    EXPECT_EQ(0x1415U, parser.readTypeFieldSize());
}

/**
 * Make sure readTypeFieldSize() recognizes default field size uint32.
 */
TEST(SomeIpParser, test_readTypeFieldSize_default)
{
    uint8_t const buffer[9U] = {0x12, 0x13, 0x14, 0x15};
    SomeIpParser parser(buffer);
    parser.setTypeFieldSize(4U);
    EXPECT_EQ(0x12131415U, parser.readTypeFieldSize());
}

/**
 * Make sure reseting current position of parser works correctly and leads to valid parser state.
 */
TEST(SomeIpParser, test_resetCurrentPosition)
{
    uint8_t const buffer[] = {0x01U};
    SomeIpParser parser(buffer);

    bool a, b;

    parser >> a;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 1U));
    EXPECT_TRUE(a);

    parser.resetCurrentPosition();
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 0U));

    parser >> b;
    EXPECT_TRUE(validateParser(parser, ErrorCode::SOMEIP_OK, 1U));
    EXPECT_TRUE(b);
}
} // anonymous namespace
