/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains
 */

#include "someip/BufferedEventSender.h"
#include "someip/EventTransceiver.h"

#include <gtest/gtest.h>

namespace
{
/**
 * Make sure BufferedEventSender size is 80U and EventTransceiver size is 56U for RpcEvents.
 */
TEST(RpcEventsSizeof, Sizeof)
{
    if (sizeof(size_t) == 8U)
    {
        EXPECT_EQ(80U, sizeof(::someip::BufferedEventSender));
        EXPECT_EQ(56U, sizeof(::someip::EventTransceiver));
    }
}

} // anonymous namespace
