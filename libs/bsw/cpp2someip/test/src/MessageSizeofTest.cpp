/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpMessage.h"

#include <gtest/gtest.h>

namespace
{
/**
 * Make sure SomeIPMessage has size 16U.
 */
TEST(MessageSizeof, Sizeof)
{
    if (sizeof(size_t) == 8U)
    {
        EXPECT_EQ(16U, sizeof(::someip::SomeIpMessage));
    }
}
} // anonymous namespace
