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
#include "someip/PrimitiveTypes.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;
using namespace ::someip::internal;

/**
 * Test getSize() for char.
 */
TEST(GetSize, getSize_for_char)
{
    auto v = char();
    EXPECT_EQ(1U, GetSizeHelper<char>::getSize(v, 0U));
}

/**
 * Test getSize() for UInt8.
 */
TEST(GetSize, getSize_for_UInt8)
{
    auto v = uint8_t();
    EXPECT_EQ(1U, GetSizeHelper<uint8_t>::getSize(v, 0U));
}

/**
 * Test getSize() for Int8.
 */
TEST(GetSize, getSize_for_Int8)
{
    auto v = int8_t();
    EXPECT_EQ(1U, GetSizeHelper<int8_t>::getSize(v, 0U));
}

/**
 * Test getSize() for UInt16.
 */
TEST(GetSize, getSize_for_UInt16)
{
    auto v = uint16_t();
    EXPECT_EQ(2U, GetSizeHelper<uint16_t>::getSize(v, 0U));
}

/**
 * Test getSize() for Int16.
 */
TEST(GetSize, getSize_for_Int16)
{
    auto v = int16_t();
    EXPECT_EQ(2U, GetSizeHelper<int16_t>::getSize(v, 0U));
}

/**
 * Test getSize() for UInt32.
 */
TEST(GetSize, getSize_for_UInt32)
{
    auto v = uint32_t();
    EXPECT_EQ(4U, GetSizeHelper<uint32_t>::getSize(v, 0U));
}

/**
 * Test getSize() for Int32.
 */
TEST(GetSize, getSize_for_Int32)
{
    auto v = int32_t();
    EXPECT_EQ(4U, GetSizeHelper<int32_t>::getSize(v, 0U));
}

/**
 * Test getSize() for UInt64.
 */
TEST(GetSize, getSize_for_UInt64)
{
    auto v = uint64_t();
    EXPECT_EQ(8U, GetSizeHelper<uint64_t>::getSize(v, 0U));
}

/**
 * Test getSize() for Int64.
 */
TEST(GetSize, getSize_for_Int64)
{
    auto v = int64_t();
    EXPECT_EQ(8U, GetSizeHelper<int64_t>::getSize(v, 0U));
}

using Vector10   = ::etl::vector<uint16_t, 10U>;
using Vector5x10 = ::etl::vector<Vector10, 5U>;

/**
 * Test getSize() for Vector.
 */
TEST(GetSize, getSize_for_vector)
{
    Vector10 v;
    v.push_back(1U);

    EXPECT_EQ(6U, GetSizeHelper<Vector10>::getSize(v, sizeof(uint32_t)));
}

/**
 * Test getSize() for nested Vector.
 */
TEST(GetSize, getSize_for_nested_vector)
{
    Vector5x10 v;
    Vector10 t;
    t.push_back(1U);
    t.push_back(2U);
    v.push_back(t);

    EXPECT_EQ(12U, GetSizeHelper<Vector5x10>::getSize(v, sizeof(uint32_t)));
}

using Array10   = ::etl::array<uint16_t, 10U>;
using Array5x10 = ::etl::array<Array10, 5U>;

/**
 * Test getSize() for Array.
 */
TEST(GetSize, getSize_for_array)
{
    Array10 a;
    a[0] = 1U;

    EXPECT_EQ(20U, GetSizeHelper<Array10>::getSize(a, sizeof(uint32_t)));
}

/**
 * Test getSize() for nested Array.
 */
TEST(GetSize, getSize_for_nested_array)
{
    Array5x10 a;
    Array10 a1;
    for (size_t i = 0U; i < a1.size(); ++i)
    {
        a1[i] = (uint16_t)i;
    }
    a[0] = a1;

    EXPECT_EQ(100U, GetSizeHelper<Array5x10>::getSize(a, sizeof(uint32_t)));
}

using StringType = ::etl::string<7U>;

/**
 * Test getSize() for StringUTF8.
 */
TEST(GetSize, getSize_for_StringUTF8)
{
    StringType str("ABC");

    // Static String
    EXPECT_EQ(
        10U, GetSizeHelper<StringType>::getSize(str, 0U, ::someip::Encoding::SOMEIP_ENCODING_UTF8));

    // Dynamic String
    EXPECT_EQ(
        11U, GetSizeHelper<StringType>::getSize(str, 4U, ::someip::Encoding::SOMEIP_ENCODING_UTF8));
}
} // anonymous namespace
