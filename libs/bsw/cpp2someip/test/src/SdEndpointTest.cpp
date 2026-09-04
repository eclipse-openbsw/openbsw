/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdEndpoint.h"

#include "someip/SomeIpConstants.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using ::someip::SdEndpoint;
using namespace ::someip::SomeIpConstants;

/**
 * Make sure SdEndpoint constructors create valid endpoints
 * and cleared endpoints or endpoints with invalid proto are not valid.
 */
TEST(SdEndpoint, constructors_and_validation)
{
    SdEndpoint e;
    EXPECT_FALSE(e.isValid());

    IPAddress ip = make_ip4(224U, 1U, 255U, 255U);
    SdEndpoint e1(ip, 0x10, 1U);

    e = e1;

    SdEndpoint e2(e1);

    EXPECT_TRUE(e1.isValid());
    EXPECT_TRUE(e2.isValid());

    // self-assignment
    SdEndpoint& other = e1;
    e1                = other;
    EXPECT_TRUE(e1.isValid());

    // invalid
    e.clear();
    EXPECT_FALSE(e.isValid());
    e1.setProto(INVALID_PROTO);
    EXPECT_FALSE(e1.isValid());
}
} // anonymous namespace
