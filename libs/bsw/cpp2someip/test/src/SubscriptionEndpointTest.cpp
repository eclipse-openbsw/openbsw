/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SubscriptionEndpoint.h"

#include "someip/ServiceDescription.h"

#include <ip/IPEndpoint.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::ip;
using ::someip::ServiceDescription;
using ::someip::SubscriptionEndpoint;

/**
 * Make sure a SubscriptionEndpoint is only valid if it is set and its ttl is also valid.
 */
TEST(SubscriptionEndpoint, test_isValid)
{
    SubscriptionEndpoint e;
    EXPECT_FALSE(e.isValid());

    SubscriptionEndpoint e1(make_ip4(192U, 0U, 2U, 10U), 20U);
    EXPECT_FALSE(e1.isValid());

    e1.ttl = 20U;
    EXPECT_TRUE(e1.isValid());
}

/**
 * Test assignment of SubscriptionEndpoint.
 */
TEST(SubscriptionEndpoint, test_assignment)
{
    SubscriptionEndpoint e(make_ip4(192U, 0U, 2U, 10U), 20U);
    EXPECT_TRUE(e.isSet());
    EXPECT_FALSE(e.isValid());

    SubscriptionEndpoint e1(e);

    EXPECT_EQ(IPEndpoint(make_ip4(192U, 0U, 2U, 10U), 20U), e1);
    EXPECT_EQ(::someip::ttl::INVALID, e1.ttl);

    SubscriptionEndpoint e2;
    e2.setAddress(make_ip4(192U, 0U, 2U, 20U));
    e2.setPort(30U);
    e2.ttl = 50U;
    EXPECT_TRUE(e2.isSet());
    EXPECT_TRUE(e2.isValid());

    SubscriptionEndpoint* other = &e2;
    e2                          = *other;
    EXPECT_EQ(IPEndpoint(make_ip4(192U, 0U, 2U, 20U), 30U), e2);
    EXPECT_EQ(50U, e2.ttl);

    e2 = e1;
    EXPECT_EQ(IPEndpoint(make_ip4(192U, 0U, 2U, 10U), 20U), e2);
    EXPECT_EQ(::someip::ttl::INVALID, e2.ttl);
}
} // anonymous namespace
