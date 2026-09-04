/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcBaseClient.h"
#include "someip/RpcChannel.h"
#include "someip/RpcHandler.h"
#include "someip/RpcReceiver.h"

#include <gtest/gtest.h>

namespace
{
/**
 * Make sure sizeof RpcBaseClient == 8U, RpcChannel == 112U, RpcHandler == 80U
 * and RpcReceiver == 88U.
 */
TEST(RpcSizeof, Sizeof)
{
    if (sizeof(size_t) == 8U)
    {
#ifdef PLATFORM_SUPPORT_IPV6
        EXPECT_EQ(8U, sizeof(::someip::RpcBaseClient));
        EXPECT_EQ(128U, sizeof(::someip::RpcChannel));
        EXPECT_EQ(80U, sizeof(::someip::RpcHandler));
        EXPECT_EQ(88U, sizeof(::someip::RpcReceiver));
#else
        EXPECT_EQ(8U, sizeof(::someip::RpcBaseClient));
        EXPECT_EQ(112U, sizeof(::someip::RpcChannel));
        EXPECT_EQ(80U, sizeof(::someip::RpcHandler));
        EXPECT_EQ(88U, sizeof(::someip::RpcReceiver));
#endif
    }
}

} // anonymous namespace
