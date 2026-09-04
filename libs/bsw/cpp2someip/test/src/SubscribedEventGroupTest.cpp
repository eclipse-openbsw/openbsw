/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SubscribedEventGroup.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

/**
 * Make sure two SubscribedEventGroups are recognized as equal
 * if both are clear (default) or both have the same serviceId, majorVersion, instanceId and
 * eventGroup.
 */
TEST(SubscribedEventGroup, test_equality)
{
    {
        SubscribedEventGroup a(1U, 2U, 3U, 4U);
        SubscribedEventGroup b(1U, 2U, 3U, 4U);
        EXPECT_TRUE(a == b);
    }
    {
        SubscribedEventGroup a(1U, 2U, 3U, 4U);
        SubscribedEventGroup b(1U, 2U, 3U, 5U);
        EXPECT_FALSE(a == b);
    }
    {
        SubscribedEventGroup a(1U, 2U, 3U, 4U);
        SubscribedEventGroup b(1U, 2U, 4U, 5U);
        EXPECT_FALSE(a == b);
    }
    {
        SubscribedEventGroup a(1U, 2U, 3U, 4U);
        SubscribedEventGroup b(1U, 3U, 4U, 5U);
        EXPECT_FALSE(a == b);
    }
    {
        SubscribedEventGroup a(1U, 2U, 3U, 4U);
        SubscribedEventGroup b(2U, 3U, 4U, 5U);
        EXPECT_FALSE(a == b);
    }
    {
        SubscribedEventGroup a(1U, 2U, 3U, 4U);
        SubscribedEventGroup b(1U, 2U, 3U, 4U);
        EXPECT_TRUE(a == b);
        b.clear();
        EXPECT_FALSE(a == b);

        // default == clear
        SubscribedEventGroup c;
        EXPECT_TRUE(c == b);
    }
}
} // anonymous namespace
