/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpStreamer.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

/**
 * Make sure isGood() recognizes whether streamer is in a good state or in an error state.
 * Once streamer is in error state, can't be reset back to SOMEIP_OK.
 */
TEST(SomeIpStreamer, SetFailure)
{
    SomeIpStreamer s;
    EXPECT_TRUE(s.isGood());
    s.setFailure(::someip::ErrorCode::SOMEIP_ERROR);
    EXPECT_FALSE(s.isGood());

    s.setFailure(::someip::ErrorCode::SOMEIP_OK);
    EXPECT_FALSE(s.isGood());
}

/**
 * Make sure validate() only changes streamer state to failure mode if streamer state is good and
 * second parameter is false. Once set to failure mode invoking validate() does not affect failure
 * mode.
 */
TEST(SomeIpStreamer, Validate)
{
    SomeIpStreamer s;
    EXPECT_TRUE(s.isGood());

    // won't change it to failure
    validate(s, true);
    EXPECT_TRUE(s.isGood());

    // change it to failure!
    validate(s, false);
    EXPECT_FALSE(s.isGood());

    // should still be in failure mode
    validate(s, true);
    EXPECT_FALSE(s.isGood());
}
} // anonymous namespace
