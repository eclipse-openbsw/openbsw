/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/defer/Defer.h"

#include <gmock/gmock.h>

using namespace ::testing;

namespace
{
static int val2 = 0;

void inc() { val2++; }

TEST(DeferTest, DeferLambda)
{
    int val = 0;
    {
        auto _ = util::defer::defer([&val]() { val++; });
        EXPECT_TRUE(val == 0);
    }
    EXPECT_TRUE(val == 1);
}

TEST(DeferTest, DeferFunctionPointer)
{
    val2 = 0;
    {
        auto _ = util::defer::defer(&inc);
        EXPECT_TRUE(val2 == 0);
    }
    EXPECT_TRUE(val2 == 1);
}

} // namespace
