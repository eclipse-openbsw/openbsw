/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkChannel.h"

#include "someip/NetworkResourceMock.h"

#include <ip/IPEndpoint.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

struct NetworkChannelTest : Test
{
    NetworkChannelTest() { _resource.incRefCounter(); }

    StrictMock<NetworkResourceMock> _resource;

    IPEndpoint _remoteEndpoint{make_ip4(192U, 0U, 2U, 0U), 15U};
    NetworkChannel _channel{_resource, _remoteEndpoint, false};
    NetworkChannel _defChannel;
};

/**
 * Make sure that local port returns error for default channel or 16U otherwise.
 */
TEST_F(NetworkChannelTest, test_getLocalPort)
{
    // default channel
    auto result = _defChannel.getLocalPort();
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(PortError::NOT_INITIALIZED, result.error());

    EXPECT_CALL(_resource, getLocalPort()).Times(1).WillOnce(Return(16U));

    auto portResult = _channel.getLocalPort();
    ASSERT_TRUE(portResult.has_value());
    EXPECT_EQ(16U, portResult.value());
}

/**
 * Make sure that buffer size is 0U for default channel or 1500U otherwise.
 */
TEST_F(NetworkChannelTest, test_buffer_size)
{
    EXPECT_EQ(0U, _defChannel.getInputBuffer().size());
    EXPECT_EQ(0U, _defChannel.getOutputBuffer().size());

    EXPECT_EQ(1500U, _channel.getInputBuffer().size());
    EXPECT_EQ(1500U, _channel.getOutputBuffer().size());
}

/**
 * Make sure default channel is initially not open while channels after adding open NetworkResource
 * are.
 */
TEST_F(NetworkChannelTest, test_isOpen)
{
    EXPECT_FALSE(_defChannel.isOpen());

    // simulate open resource
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(true));

    EXPECT_TRUE(_channel.isOpen());
}

/**
 * Make sure default channel is not multicast and setting multicast flag is reflected correctly.
 */
TEST_F(NetworkChannelTest, test_isMulticast)
{
    EXPECT_FALSE(_defChannel.isMulticast());

    // simulate closed resource
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(_channel.isMulticast());

    _channel = NetworkChannel(_resource, _remoteEndpoint, true);

    // simulate open resource
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(_channel.isMulticast());
}

TEST_F(NetworkChannelTest, Send)
{
    EXPECT_FALSE(_defChannel.send(15U));

    // not open
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(_channel.send(15U));

    // open but not connected
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_resource, isConnected()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(_resource, send(_, 15U)).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(_channel.send(15U));

    // open and connected
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_resource, isConnected()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_resource, send(16U)).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(_channel.send(16U));
}

TEST_F(NetworkChannelTest, Close)
{
    InSequence inSequence;

    _resource.decRefCounter();

    {
        NetworkChannel channel;
        channel.close();
    }
    {
        EXPECT_CALL(_resource, close());
        _channel.close();
        Mock::VerifyAndClearExpectations(&_resource);
    }
    {
        NetworkChannel channel(_resource, _remoteEndpoint, false);
        NetworkChannel channel2(_resource, _remoteEndpoint, false);
        channel.close();
        EXPECT_CALL(_resource, close());
        channel2.close();
        Mock::VerifyAndClearExpectations(&_resource);
    }
    {
        NetworkChannel channel(_resource, _remoteEndpoint, false);
        NetworkChannel channel2(channel);
        channel.close();
        EXPECT_CALL(_resource, close());
        channel2.close();
        Mock::VerifyAndClearExpectations(&_resource);
    }
    {
        NetworkChannel channel(_resource, _remoteEndpoint, false);
        StrictMock<NetworkResourceMock> resource2;
        NetworkChannel channel2(resource2, _remoteEndpoint, false);
        EXPECT_CALL(resource2, close());
        channel2 = channel;

        channel.close();
        EXPECT_CALL(_resource, close());
        channel2.close();
        Mock::VerifyAndClearExpectations(&_resource);
    }
}

TEST_F(NetworkChannelTest, destructor)
{
    InSequence inSequence;
    _resource.decRefCounter();
    EXPECT_CALL(_resource, close());
    _channel.close();
    Mock::VerifyAndClearExpectations(&_resource);

    {
        NetworkChannel channel;
    }
    Mock::VerifyAndClearExpectations(&_resource);

    {
        NetworkChannel channel(_resource, _remoteEndpoint, false);
        EXPECT_CALL(_resource, close());
    }
    Mock::VerifyAndClearExpectations(&_resource);

    {
        NetworkChannel channel(_resource, _remoteEndpoint, false);
        NetworkChannel channel2(_resource, _remoteEndpoint, false);
        EXPECT_CALL(_resource, close());
    }
    Mock::VerifyAndClearExpectations(&_resource);
}

} // anonymous namespace
