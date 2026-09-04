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

#include "someip/CallDoneClosureMock.h"
#include "someip/RpcChannelMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;

struct RpcBaseClientTest
: Test
, RpcBaseClient
{
    StrictMock<CallDoneClosureMock> _done;
};

/**
 * Make sure RpcBaseClient invoking callFireAndForget() without channel is not successful.
 */
TEST_F(RpcBaseClientTest, callFireAndForget_without_channel_unsuccessful)
{
    EXPECT_CALL(_done, invoke(::someip::RPC_ERROR_NO_CHANNEL));

    callFireAndForget(1U, nullptr, 0U, _done);
}

/**
 * Make sure RpcBaseClient invoking callMethod() without channel is not successful.
 */
TEST_F(RpcBaseClientTest, callMethod__without_channel_unsuccessful)
{
    EXPECT_CALL(_done, invoke(::someip::RPC_ERROR_NO_CHANNEL));

    callMethod(1U, nullptr, nullptr, 0U, _done);
}

/**
 * Test channel connect and disconnect process to RpcBaseClient.
 */
TEST_F(RpcBaseClientTest, connect_and_disconnect_channel)
{
    ::someip::RpcChannelMock channel;
    EXPECT_FALSE(isConnected());
    connect(channel);
    EXPECT_TRUE(isConnected());
    disconnect();
    EXPECT_FALSE(isConnected());
}

} // anonymous namespace
