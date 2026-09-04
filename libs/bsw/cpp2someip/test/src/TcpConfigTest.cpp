/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpConfig.h"

#include "TestConstants.h"
#include "someip/NetworkListenerMock.h"

#include <tcp/socket/AbstractServerSocketMock.h>
#include <tcp/socket/AbstractSocketMock.h>

#include <gmock/gmock.h>

namespace tcp
{
struct AbstractServerSocketMock2 : public AbstractServerSocketMock
{
    // AbstractServerSocket::getLocalPort() not virtual
    void mockLocalPort(uint16_t port) { _port = port; }
};

} // namespace tcp

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;
using namespace ::test;

void mockServerSocketClosed(TcpServer* server, bool isClosed)
{
    ::tcp::AbstractServerSocketMock2& s
        = (::tcp::AbstractServerSocketMock2&)server->getServerSocket();

    EXPECT_CALL(s, isClosed()).Times(1).WillOnce(Return(isClosed));
}

void mockSocketClosed(TcpProxy* proxy, bool isClosed)
{
    ::tcp::AbstractSocketMock* s = (::tcp::AbstractSocketMock*)(&(proxy->getSocket()));

    EXPECT_CALL(*s, isClosed()).Times(1).WillOnce(Return(isClosed));
}

struct TcpConfigTest : ::testing::Test
{
    TcpConfigTest() { _config._tcpProxies.getProxy(0U)->setListener(_listener); }

    ::etl::array<uint8_t, BUFFER_SIZE> _inputBuffer{};
    ::etl::array<uint8_t, BUFFER_SIZE> _outputBuffer{};

    ::someip::internal::TcpResources<
        StrictMock<::tcp::AbstractServerSocketMock2>,
        1U,
        StrictMock<::tcp::AbstractSocketMock>,
        1U,
        BUFFER_SIZE>
        _resources{_inputBuffer, _outputBuffer};

    TcpConfig& _config{_resources};

    StrictMock<NetworkListenerMock> _listener;
};

/**
 * Test initializing TcpConfig successful.
 */
TEST_F(TcpConfigTest, test_initialization)
{
    EXPECT_EQ(1U, _config._tcpServers.getSize());
    EXPECT_EQ(1U, _config._tcpProxies.getSize());
    auto portResult = _config._tcpServers.getPort(0U);
    EXPECT_FALSE(portResult.has_value());
    EXPECT_TRUE(_config._tcpProxies.getProxy(0U)->isInitialized());
    EXPECT_TRUE(_config._tcpServers.getServer(0U)->isInitialized());
}

TEST_F(TcpConfigTest, InitTcpPort)
{
    auto portResult1 = _config._tcpServers.getPort(0U);
    EXPECT_FALSE(portResult1.has_value());
    EXPECT_TRUE(_config._tcpServers.initPort(16U));
    auto portResult2 = _config._tcpServers.getPort(0U);
    ASSERT_TRUE(portResult2.has_value());
    EXPECT_EQ(16U, portResult2.value());
    EXPECT_FALSE(_config._tcpServers.initPort(17U));
    auto portResult3 = _config._tcpServers.getPort(0U);
    ASSERT_TRUE(portResult3.has_value());
    EXPECT_EQ(16U, portResult3.value());
}

TEST_F(TcpConfigTest, GetSetBufferPool)
{
    ::etl::vector<::etl::span<uint8_t>, 1U> pool;
    {
        EXPECT_FALSE(_config._tcpServers.setBufferPool(16U, pool));
    }
    {
        ::someip::internal::TcpResources<
            StrictMock<::tcp::AbstractServerSocketMock2>,
            1U,
            StrictMock<::tcp::AbstractSocketMock>,
            1U,
            0U>
            _resources{_inputBuffer, _outputBuffer};

        TcpConfig& config{_resources};

        EXPECT_TRUE(config._tcpServers.setBufferPool(16U, pool));
        EXPECT_FALSE(config._tcpServers.setBufferPool(17U, pool));
        EXPECT_EQ(&pool, config._tcpServers.getBufferPool(16U));
        EXPECT_EQ(nullptr, config._tcpServers.getBufferPool(17U));
    }
}

/**
 * Test getting proxy by socket.
 */
TEST_F(TcpConfigTest, test_getProxy_by_socket)
{
    ::tcp::AbstractSocketMock socket;
    EXPECT_EQ(nullptr, _config._tcpProxies.getProxy(socket));

    TcpProxy* existingProxy = _config._tcpProxies.getProxy(0U);
    EXPECT_NE(nullptr, existingProxy);
    ::tcp::AbstractSocket& existingSocket = existingProxy->getSocket();
    EXPECT_EQ(existingProxy, _config._tcpProxies.getProxy(existingSocket));
}

TEST_F(TcpConfigTest, TcpServers)
{
    // the socket is closed
    mockServerSocketClosed(_config._tcpServers.getServer(0U), true);

    TcpServer* server = _config._tcpServers.nextServer();
    EXPECT_TRUE(server != nullptr);

    // socket is in use
    mockServerSocketClosed(_config._tcpServers.getServer(0U), false);
    EXPECT_TRUE(_config._tcpServers.nextServer() == nullptr);

    // close servers
    mockServerSocketClosed(_config._tcpServers.getServer(0U), true);
    _config._tcpServers.close();
}

TEST_F(TcpConfigTest, TcpProxies)
{
    // the socket is closed
    mockSocketClosed(_config._tcpProxies.getProxy(0U), true);
    StrictMock<NetworkListenerMock> listener;

    _config._tcpProxies.setListener(listener);

    TcpProxy* proxy = _config._tcpProxies.nextProxy();
    EXPECT_TRUE(proxy != nullptr);

    // socket is in use
    mockSocketClosed(_config._tcpProxies.getProxy(0U), false);
    EXPECT_TRUE(_config._tcpProxies.nextProxy() == nullptr);
}

/**
 * Make sure no TcpServer is returned by getOpenServer if server is not open.
 */
TEST_F(TcpConfigTest, test_getOpenTcpServer_if_server_is_closed)
{
    mockServerSocketClosed(_config._tcpServers.getServer(0U), true);

    EXPECT_EQ(nullptr, _config._tcpServers.getOpenServer(16U));
}

/**
 * Make sure no TcpServer is returned by getOpenServer if no open server matches local port.
 */
TEST_F(TcpConfigTest, test_getOpenTcpServer_if_local_port_is_wrong)
{
    mockServerSocketClosed(_config._tcpServers.getServer(0U), false);

    ::tcp::AbstractServerSocketMock2& s
        = (::tcp::AbstractServerSocketMock2&)_config._tcpServers.getServer(0U)->getServerSocket();
    s.mockLocalPort(15U);

    EXPECT_EQ(nullptr, _config._tcpServers.getOpenServer(16U));
}

/**
 * Test getOpenTcpServer() successfully returning an open server.
 */
TEST_F(TcpConfigTest, test_getOpenTcpServer_success)
{
    mockServerSocketClosed(_config._tcpServers.getServer(0U), false);

    ::tcp::AbstractServerSocketMock2& s
        = (::tcp::AbstractServerSocketMock2&)_config._tcpServers.getServer(0U)->getServerSocket();
    s.mockLocalPort(15U);

    EXPECT_EQ(_config._tcpServers.getServer(0U), _config._tcpServers.getOpenServer(15U));
}

/**
 * Make sure no TcpProxy is returned by getOpenTcpProxy() if proxy is not open.
 */
TEST_F(TcpConfigTest, test_getOpenTcpProxy_if_proxy_is_closed)
{
    TcpProxy* proxy = _config._tcpProxies.getProxy(0U);
    mockSocketClosed(proxy, true);

    // Even though proxy is closed, getLocalPort() might still be called
    // in some code paths, so set up expectation
    ::tcp::AbstractSocketMock& s = dynamic_cast<::tcp::AbstractSocketMock&>(proxy->getSocket());
    EXPECT_CALL(s, getLocalPort()).Times(AtMost(1)).WillOnce(Return(16U));

    EXPECT_THAT(
        _config._tcpProxies.getOpenProxy(16U, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 14U)),
        IsNull());
}

/**
 * Make sure no TcpProxy is returned by getOpenTcpProxy() if no proxy matches local port.
 */
TEST_F(TcpConfigTest, test_getOpenTcpProxy_if_local_port_is_wrong)
{
    mockSocketClosed(_config._tcpProxies.getProxy(0U), false);

    ::tcp::AbstractSocketMock& s
        = dynamic_cast<::tcp::AbstractSocketMock&>(_config._tcpProxies.getProxy(0U)->getSocket());
    EXPECT_CALL(s, getLocalPort()).Times(1).WillOnce(Return(15U));

    EXPECT_EQ(
        nullptr,
        _config._tcpProxies.getOpenProxy(16U, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 14U)));
}

TEST_F(TcpConfigTest, GetOpenTcpProxy_WrongRemotePort)
{
    mockSocketClosed(_config._tcpProxies.getProxy(0U), false);

    ::tcp::AbstractSocketMock& s
        = dynamic_cast<::tcp::AbstractSocketMock&>(_config._tcpProxies.getProxy(0U)->getSocket());
    EXPECT_CALL(s, getLocalPort()).Times(1).WillOnce(Return(15U));
    EXPECT_CALL(s, getRemoteIPAddress()).Times(1).WillOnce(Return(make_ip4(192U, 0U, 2U, 1U)));
    EXPECT_CALL(s, getRemotePort()).Times(1).WillOnce(Return(13U));

    EXPECT_EQ(
        nullptr,
        _config._tcpProxies.getOpenProxy(15U, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 14U)));
}

/**
 * Make sure no TcpProxy is returned by getOpenTcpProxy() if no proxy matches remote address.
 */
TEST_F(TcpConfigTest, test_getOpenTcpProxy_if_remote_address_is_wrong)
{
    mockSocketClosed(_config._tcpProxies.getProxy(0U), false);

    ::tcp::AbstractSocketMock& s
        = dynamic_cast<::tcp::AbstractSocketMock&>(_config._tcpProxies.getProxy(0U)->getSocket());
    EXPECT_CALL(s, getLocalPort()).Times(1).WillOnce(Return(15U));
    EXPECT_CALL(s, getRemoteIPAddress()).Times(1).WillOnce(Return(make_ip4(192U, 0U, 2U, 2U)));
    EXPECT_CALL(s, getRemotePort()).Times(1).WillOnce(Return(14U));

    EXPECT_EQ(
        nullptr,
        _config._tcpProxies.getOpenProxy(15U, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 14U)));
}

/**
 * Test getOpenTcpProxy() successfully returning an open proxy.
 */
TEST_F(TcpConfigTest, test_getOpenTcpProxy_success)
{
    mockSocketClosed(_config._tcpProxies.getProxy(0U), false);

    ::tcp::AbstractSocketMock& s
        = dynamic_cast<::tcp::AbstractSocketMock&>(_config._tcpProxies.getProxy(0U)->getSocket());
    EXPECT_CALL(s, getLocalPort()).Times(1).WillOnce(Return(15U));
    EXPECT_CALL(s, getRemoteIPAddress()).Times(1).WillOnce(Return(make_ip4(192U, 0U, 2U, 1U)));
    EXPECT_CALL(s, getRemotePort()).Times(1).WillOnce(Return(14U));

    EXPECT_EQ(
        _config._tcpProxies.getProxy(0U),
        _config._tcpProxies.getOpenProxy(15U, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 14U)));
}

} // anonymous namespace
