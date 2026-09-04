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

#include <gtest/gtest.h>

namespace
{
/**
 * Make sure sizeof SomeIpParser == 32U and SomeIpSerializer == 32U.
 */
TEST(SerializationSizeof, Sizeof)
{
    if (sizeof(size_t) == 8U)
    {
        EXPECT_EQ(32U, sizeof(::someip::SomeIpParser));
        EXPECT_EQ(32U, sizeof(::someip::SomeIpSerializer));
    }
}

} // anonymous namespace
