/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceQuery.h"

#include "someip/init.h"

#include <gtest/gtest.h>

namespace
{
using ::someip::ServiceDescription;
using ::someip::ServiceQuery;

/**
 * Make sure it is detected correctly whether ServiceQuery multicastAddress contains invalid IP.
 */
TEST(ServiceQueryTest, tests_is_multicast_ip_invalid)
{
    auto query = ::someip::make<ServiceQuery>();
    EXPECT_FALSE(query.multicastAddress.isSet());

    ::ip::IPEndpoint endpoint(::ip::make_ip4(52U), 10U);
    query.multicastAddress = endpoint;
    EXPECT_TRUE(query.multicastAddress.isSet());

    query.multicastAddress.clear();
    EXPECT_FALSE(query.multicastAddress.isSet());
}
} // anonymous namespace
