/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/EnumSerializable.h"

#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include <gtest/gtest.h>

using namespace ::someip;

namespace
{
struct MyEnums
{
    enum type
    {
        NONE   = 0,
        WITH_A = 1,
        WITH_B = 2
    };
};

/**
 * Tests construction of serializable enum.
 */
TEST(EnumSerializable, test_constructors)
{
    EnumSerializable<MyEnums, uint8_t> value;
    EXPECT_EQ(1U, value.getSize());

    value = decltype(value)(MyEnums::WITH_A);

    EnumSerializable<MyEnums, uint8_t> copy(value);
    EXPECT_EQ(MyEnums::WITH_A, copy.value());

    EnumSerializable<MyEnums, uint8_t> valueCopy(2);
    EXPECT_EQ(MyEnums::WITH_B, valueCopy.value());

    value = valueCopy;
    EXPECT_EQ(MyEnums::WITH_B, value.value());
}

/**
 * Tests getSize() for serializable enum.
 */
TEST(EnumSerializable, test_getSize_serializable_enum)
{
    EnumSerializable<MyEnums, uint8_t> value(MyEnums::WITH_A);

    EXPECT_EQ(1U, value.getSize());
}

TEST(EnumSerializable, TestSerialize)
{
    EnumSerializable<MyEnums, uint8_t> value(MyEnums::WITH_A);

    uint8_t buffer[10] = {0U};
    SomeIpSerializer serializer(buffer);

    serializer << value;
    EXPECT_EQ(1U, serializer.getCurrentPosition());
    EXPECT_EQ(0x01, buffer[0]);
}

TEST(EnumSerializable, TestParse)
{
    EnumSerializable<MyEnums, uint8_t> value(MyEnums::WITH_A);

    uint8_t const buffer[10] = {0x02};
    SomeIpParser parser(buffer);

    parser >> value;
    EXPECT_EQ(MyEnums::WITH_B, value.value());
    EXPECT_EQ(1U, parser.getCurrentPosition());
}
} // anonymous namespace
