/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/UnionBase.h"

#include <gtest/gtest.h>

namespace
{
struct UnionBaseTest
: ::testing::Test
, ::someip::UnionBase
{
    void serializeToArray(::someip::SomeIpSerializer&) const override {}

    void parseFromArray(::someip::SomeIpParser& /* parser */) override {}

    uint32_t getSize() const override { return 0U; }
};

struct Tmp
{
    uint16_t item16;
    uint8_t item8;
};

/**
 * Make sure casting as union type works correctly for const items and non-const items.
 */
TEST_F(UnionBaseTest, test_casts)
{
    Tmp t{};
    t.item16 = 16U;
    t.item8  = 8U;

    uint8_t* p           = (uint8_t*)&t;
    Tmp const* itemConst = castAsConstUnionType<Tmp>(p);
    EXPECT_EQ(16U, itemConst->item16);
    EXPECT_EQ(8U, itemConst->item8);

    Tmp* item = castAsUnionType<Tmp>(p);
    EXPECT_EQ(16U, item->item16);
    EXPECT_EQ(8U, item->item8);
}

/**
 * Make sure memCpy() works correctly.
 */
TEST_F(UnionBaseTest, test_memCpy)
{
    Tmp t{};
    t.item16 = 16U;
    t.item8  = 8U;

    Tmp other{};
    memCpy(&other, &t, sizeof(Tmp));
    EXPECT_EQ(16U, other.item16);
    EXPECT_EQ(8U, other.item8);
}
} // anonymous namespace
