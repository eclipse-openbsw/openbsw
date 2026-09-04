/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/UdpConfig.h"

#include "TestConstants.h"
#include "someip/NetworkListenerMock.h"

#include <udp/socket/AbstractDatagramSocketMock.h>

#include <etl/span.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::someip;
using namespace ::test;

void mockUdpSocketClosed(UdpProxy* proxy, bool isClosed)
{
    ::udp::AbstractDatagramSocketMock* s = (::udp::AbstractDatagramSocketMock*)proxy->getSocket();

    EXPECT_CALL(*s, isBound()).Times(1).WillOnce(Return(!isClosed));
}

void mockUdpLocalPort(UdpProxy* proxy, uint16_t localPort)
{
    ::udp::AbstractDatagramSocketMock* s = (::udp::AbstractDatagramSocketMock*)proxy->getSocket();

    EXPECT_CALL(*s, getLocalPort()).Times(1).WillOnce(Return(localPort));
}

struct UdpConfigTest : Test
{
    UdpConfigTest()
    {
        _config._sdProxies.setListener(_listener);
        _config._rpcProxies.setListener(_listener);
    }

    ::etl::array<uint8_t, BUFFER_SIZE> _inputBuffer{};
    ::etl::array<uint8_t, BUFFER_SIZE> _outputBuffer{};

    ::someip::internal::UdpResources<StrictMock<::udp::AbstractDatagramSocketMock>, 1U, 1U>
        _resources{_inputBuffer, _outputBuffer};

    UdpConfig& _config{_resources};

    StrictMock<NetworkListenerMock> _listener;
};

/**
 * Make sure UdpConfig is initialized correctly.
 */
TEST_F(UdpConfigTest, test_initialization)
{
    auto sdPortResult = _config._sdProxies.getPort(0U);
    EXPECT_FALSE(sdPortResult.has_value());
    auto rpcPortResult = _config._rpcProxies.getPort(0U);
    EXPECT_FALSE(rpcPortResult.has_value());
    EXPECT_THAT(_config._sdProxies.getProxy(0U)->getListener(), NotNull());
    EXPECT_THAT(_config._sdProxies.getProxy(0U)->getSocket(), NotNull());
    EXPECT_TRUE(_config._sdProxies.getProxy(0U)->isInitialized());
    EXPECT_TRUE(_config._rpcProxies.getProxy(0U)->isInitialized());
}

TEST_F(UdpConfigTest, InitSdPort)
{
    auto portResult1 = _config._sdProxies.getPort(0U);
    EXPECT_FALSE(portResult1.has_value());

    EXPECT_TRUE(_config._sdProxies.initPort(16U));
    auto portResult2 = _config._sdProxies.getPort(0U);
    ASSERT_TRUE(portResult2.has_value());
    EXPECT_EQ(16U, portResult2.value());

    EXPECT_FALSE(_config._sdProxies.initPort(17U));
    auto portResult3 = _config._sdProxies.getPort(0U);
    ASSERT_TRUE(portResult3.has_value());
    EXPECT_EQ(16U, portResult3.value());
}

TEST_F(UdpConfigTest, InitRpcPort)
{
    auto portResult1 = _config._rpcProxies.getPort(0U);
    EXPECT_FALSE(portResult1.has_value());

    EXPECT_TRUE(_config._rpcProxies.initPort(16U));
    auto portResult2 = _config._rpcProxies.getPort(0U);
    ASSERT_TRUE(portResult2.has_value());
    EXPECT_EQ(16U, portResult2.value());

    EXPECT_FALSE(_config._rpcProxies.initPort(17U));
    auto portResult3 = _config._rpcProxies.getPort(0U);
    ASSERT_TRUE(portResult3.has_value());
    EXPECT_EQ(16U, portResult3.value());
}

TEST_F(UdpConfigTest, NextSdProxy_Available)
{
    mockUdpSocketClosed(_config._sdProxies.getProxy(0U), false);

    EXPECT_THAT(_config._sdProxies.nextProxy(), IsNull());
}

TEST_F(UdpConfigTest, GetOpenSdProxy_NotOpen)
{
    mockUdpSocketClosed(_config._sdProxies.getProxy(0U), true);

    EXPECT_THAT(_config._sdProxies.getOpenProxy(15U), IsNull());
}

TEST_F(UdpConfigTest, GetOpenSdProxy_WrongPort)
{
    mockUdpSocketClosed(_config._sdProxies.getProxy(0U), false);
    mockUdpLocalPort(_config._sdProxies.getProxy(0U), 15U);

    EXPECT_THAT(_config._sdProxies.getOpenProxy(16U), IsNull());
}

TEST_F(UdpConfigTest, GetOpenSdProxy_Success)
{
    mockUdpSocketClosed(_config._sdProxies.getProxy(0U), false);
    mockUdpLocalPort(_config._sdProxies.getProxy(0U), 15U);

    EXPECT_EQ(_config._sdProxies.getProxy(0U), _config._sdProxies.getOpenProxy(15U));
}

TEST_F(UdpConfigTest, NextRpcProxy_Available)
{
    mockUdpSocketClosed(_config._rpcProxies.getProxy(0U), false);

    EXPECT_THAT(_config._rpcProxies.nextProxy(), IsNull());
}

TEST_F(UdpConfigTest, GetOpenRpcProxy_NotOpen)
{
    mockUdpSocketClosed(_config._rpcProxies.getProxy(0U), true);

    EXPECT_THAT(_config._rpcProxies.getOpenProxy(15U), IsNull());
}

TEST_F(UdpConfigTest, GetOpenRpcProxy_WrongPort)
{
    mockUdpSocketClosed(_config._rpcProxies.getProxy(0U), false);
    mockUdpLocalPort(_config._rpcProxies.getProxy(0U), 15U);

    EXPECT_THAT(_config._rpcProxies.getOpenProxy(16U), IsNull());
}

TEST_F(UdpConfigTest, GetOpenRpcProxy_Success)
{
    mockUdpSocketClosed(_config._rpcProxies.getProxy(0U), false);
    mockUdpLocalPort(_config._rpcProxies.getProxy(0U), 15U);

    EXPECT_EQ(_config._rpcProxies.getProxy(0U), _config._rpcProxies.getOpenProxy(15U));
}

TEST_F(UdpConfigTest, Close)
{
    mockUdpSocketClosed(_config._rpcProxies.getProxy(0U), false);
    mockUdpLocalPort(_config._rpcProxies.getProxy(0U), 15U);

    EXPECT_EQ(_config._rpcProxies.getProxy(0U), _config._rpcProxies.getOpenProxy(15U));

    mockUdpSocketClosed(_config._rpcProxies.getProxy(0U), true);
    _config._rpcProxies.close();
}

} // anonymous namespace
