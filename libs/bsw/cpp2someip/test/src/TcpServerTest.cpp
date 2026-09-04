/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpServer.h"

#include "someip/NetworkListenerMock.h"
#include "someip/TcpConfig.h"
#include "someip/TcpProxyMock.h"

#include <tcp/socket/AbstractServerSocketMock.h>
#include <tcp/socket/AbstractSocketMock.h>

#include <etl/vector.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;
using namespace ::tcp;
using namespace ::ip;

struct TcpServerTest : Test
{
    TcpServerTest() : _proxyConfig(_proxyList, TcpProxyConfig::BufferType::Internal)
    {
        for (auto& i : _socketArray)
        {
            _proxyList.emplace_back(i);
            _proxyList.back().setInputBuffer(
                ::etl::span<uint8_t>(_fakeBuffer, sizeof(_fakeBuffer)));
            _proxyList.back().setOutputBuffer(
                ::etl::span<uint8_t>(_fakeBuffer, sizeof(_fakeBuffer)));
            _proxyList.back().setInternalBuffer(
                ::etl::span<uint8_t>(_fakeBuffer, sizeof(_fakeBuffer)));
            _proxyList.back().setListener(_listenerMock);
            _proxyList.back().setConnectionListener(&_proxyConnectionListenerMock);
            EXPECT_TRUE(_proxyList.back().isInitialized());
        }
        // Update proxy config with populated proxy list
        _proxyConfig.setProxies(_proxyList);
    }

    AbstractSocketMock _socketArray[3U];
    StrictMock<AbstractServerSocketMock> _serverSocketMock;
    TcpProxyConfig _proxyConfig;
    ::someip::NetworkListenerMock _listenerMock;
    ::etl::vector<TcpProxy, 3U> _proxyList;
    uint8_t _fakeBuffer[1U];
    NiceMock<TcpProxyConnectionListenerMock> _proxyConnectionListenerMock;
};

TEST_F(TcpServerTest, ExternalBuffers)
{
    TcpProxyConfig proxyConfigWithExternalBuffer(_proxyList, TcpProxyConfig::BufferType::External);
    TcpServer server(_serverSocketMock, proxyConfigWithExternalBuffer);

    ::etl::vector<::etl::span<uint8_t>, 2U> buffers(2U);
    server.setClientBufferPool(buffers);

    EXPECT_CALL(_socketArray[0U], isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(_proxyList[0U].isIdle());
    EXPECT_CALL(_socketArray[0U], isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_EQ(&_proxyList[0U], _proxyConfig.nextProxy());

    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    EXPECT_CALL(_socketArray[0U], isClosed()).Times(4).WillRepeatedly(Return(true));
    EXPECT_EQ(&_socketArray[0U], server.getSocket(addr, 15U));

    EXPECT_CALL(_socketArray[1U], isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_EQ(&_proxyList[1U], _proxyConfig.nextProxy());
    EXPECT_FALSE(_proxyList[0U].isIdle());

    EXPECT_CALL(_socketArray[1U], isClosed()).Times(3).WillRepeatedly(Return(true));
    EXPECT_EQ(&_socketArray[1U], server.getSocket(addr, 16U));

    // no buffers left
    EXPECT_CALL(_socketArray[2U], isClosed()).Times(2).WillRepeatedly(Return(true));
    EXPECT_EQ(nullptr, server.getSocket(addr, 17U));

    // connection dropped
    EXPECT_CALL(_socketArray[0U], isClosed()).Times(1).WillOnce(Return(true));
    server.connectionChanged(_proxyList[0U]);
    EXPECT_CALL(_socketArray[0U], isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(_proxyList[0U].isIdle());
}

TEST_F(TcpServerTest, GetProxies)
{
    TcpServer server(_serverSocketMock, _proxyConfig);

    EXPECT_EQ(3U, server.getNumProxies());
    EXPECT_EQ(&_proxyList.at(0U), server.getProxy(0U));
}

/**
 * Make sure TcpServer is only initialized if TcpClientPool > 0.
 */
TEST_F(TcpServerTest, test_isInitialized)
{
    {
        TcpServer server(_serverSocketMock, _proxyConfig);
        EXPECT_TRUE(server.isInitialized());
    }
    {
        ::etl::vector<TcpProxy, 1U> emptyProxyList;
        TcpProxyConfig configWithoutProxies(emptyProxyList, TcpProxyConfig::BufferType::Internal);
        TcpServer server(_serverSocketMock, configWithoutProxies);
        EXPECT_FALSE(server.isInitialized());
    }
}

/**
 * Make sure a TcpServer is open only if it is initialized and socket is not closed.
 */
TEST_F(TcpServerTest, test_isOpen)
{
    TcpServer server(_serverSocketMock, _proxyConfig);

    // not initialized
    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_FALSE(server.isOpen());

    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(false));
    EXPECT_TRUE(server.isOpen());
}

/**
 * Make sure TcpServer cannot be opened if it is uninitialized.
 */
TEST_F(TcpServerTest, cannot_open_uninitialized_server)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    IPEndpoint endpoint(addr, 15U);

    ::etl::vector<TcpProxy, 1U> emptyProxyList;
    TcpProxyConfig configWithoutProxies(emptyProxyList, TcpProxyConfig::BufferType::Internal);
    TcpServer server(_serverSocketMock, configWithoutProxies);
    EXPECT_FALSE(server.open(endpoint));
}

/**
 * If TcpServe is already open, nothing changes on calling open().
 */
TEST_F(TcpServerTest, calling_open_if_already_open)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    IPEndpoint endpoint(addr, 15U);
    TcpServer server(_serverSocketMock, _proxyConfig);

    // initialized, and already opened
    EXPECT_CALL(_serverSocketMock, isClosed()).WillRepeatedly(Return(false));
    EXPECT_TRUE(server.open(endpoint));
}

TEST_F(TcpServerTest, Open_InitializedCantBindSocket)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    IPEndpoint endpoint(addr, 15U);
    TcpServer server(_serverSocketMock, _proxyConfig);

    // initialized, not open, can't bind socket
    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_serverSocketMock, bind(Eq(addr), Eq(15U))).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(server.open(endpoint));
}

TEST_F(TcpServerTest, Open_InitializedCantAcceptSocket)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    IPEndpoint endpoint(addr, 15U);
    TcpServer server(_serverSocketMock, _proxyConfig);

    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_serverSocketMock, bind(Eq(addr), Eq(15U))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_serverSocketMock, accept()).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(server.open(endpoint));
}

TEST_F(TcpServerTest, Open_Success)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    IPEndpoint endpoint(addr, 15U);
    TcpServer server(_serverSocketMock, _proxyConfig);

    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_serverSocketMock, bind(Eq(addr), Eq(15U))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_serverSocketMock, accept()).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(server.open(endpoint));
}

TEST_F(TcpServerTest, GetSocket_NoProxies)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    ::etl::vector<TcpProxy, 1U> emptyProxyList;
    TcpProxyConfig configWithoutProxies(emptyProxyList, TcpProxyConfig::BufferType::Internal);
    TcpServer server(_serverSocketMock, configWithoutProxies);

    EXPECT_THAT(server.getSocket(addr, 15U), IsNull());
}

TEST_F(TcpServerTest, GetSocket_ProxyNotInitialized)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    ::etl::vector<TcpProxy, 1U> proxyList;
    proxyList.emplace_back(_socketArray[0U]);
    TcpProxyConfig configWithoutProxies(proxyList, TcpProxyConfig::BufferType::Internal);
    TcpServer server(_serverSocketMock, configWithoutProxies);

    EXPECT_FALSE(proxyList.back().isInitialized());
    EXPECT_THAT(server.getSocket(addr, 15U), IsNull());
}

TEST_F(TcpServerTest, GetSocket_ProxyAlreadyOpened)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    TcpServer server(_serverSocketMock, _proxyConfig);

    EXPECT_CALL(_socketArray[0U], isClosed()).Times(2).WillRepeatedly(Return(false));

    EXPECT_THAT(server.getSocket(addr, 15U), IsNull());
}

TEST_F(TcpServerTest, GetSocket_ProxyAlreadyOpenedAddressReused)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    TcpProxyConfig proxyConfigWithExternalBuffer(_proxyList, TcpProxyConfig::BufferType::External);
    _serverSocketMock.setPort(15U);
    TcpServer server(_serverSocketMock, proxyConfigWithExternalBuffer);
    ::etl::vector<::etl::span<uint8_t>, 2U> buffers(2U);
    server.setClientBufferPool(buffers);

    EXPECT_CALL(_socketArray[0U], isClosed()).WillRepeatedly(Return(true));
    EXPECT_CALL(_socketArray[1U], isClosed()).WillRepeatedly(Return(true));
    EXPECT_CALL(_socketArray[2U], isClosed()).WillRepeatedly(Return(true));

    EXPECT_CALL(_socketArray[0U], getRemotePort()).WillRepeatedly(Return(0U));
    EXPECT_CALL(_socketArray[1U], getRemotePort()).WillRepeatedly(Return(0U));
    EXPECT_CALL(_socketArray[2U], getRemotePort()).WillRepeatedly(Return(0U));
    EXPECT_CALL(_socketArray[0U], getLocalPort()).WillRepeatedly(Return(0U));

    EXPECT_EQ(&_socketArray[0U], server.getSocket(addr, 15U));

    EXPECT_CALL(_socketArray[0U], isClosed()).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(_socketArray[0U], getRemoteIPAddress()).WillOnce(Return(addr));
    EXPECT_CALL(_socketArray[0U], getRemotePort()).WillOnce(Return(15U));
    EXPECT_CALL(_socketArray[0U], getLocalPort()).WillOnce(Return(15U));

    EXPECT_CALL(_socketArray[0U], abort());
    EXPECT_EQ(&_socketArray[0U], server.getSocket(addr, 15U));
    EXPECT_EQ(make_ip4(192U, 0U, 2U, 1U), addr);
}

TEST_F(TcpServerTest, GetSocket_Success)
{
    IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    TcpServer server(_serverSocketMock, _proxyConfig);
    EXPECT_CALL(_socketArray[0U], isClosed()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(_socketArray[1U], isClosed()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(_socketArray[2U], isClosed()).Times(1).WillRepeatedly(Return(true));

    EXPECT_EQ(&_socketArray[0U], server.getSocket(addr, 15U));
}

/**
 * Make sure nothing happens on calling close() if server is not open.
 */
TEST_F(TcpServerTest, call_close_if_server_not_open)
{
    TcpServer server(_serverSocketMock, _proxyConfig);

    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(true));

    // shouldn't call this one!
    EXPECT_CALL(_serverSocketMock, close()).Times(0);
    server.close();
}

/**
 * Test successfully calling close on TcpServer.
 */
TEST_F(TcpServerTest, successfully_call_close)
{
    TcpServer server(_serverSocketMock, _proxyConfig);
    EXPECT_CALL(_serverSocketMock, isClosed()).Times(1).WillOnce(Return(false));

    EXPECT_CALL(_serverSocketMock, close()).Times(1);
    server.close();
}

/**
 * Test successfully invoking connectionAccepted().
 */
TEST_F(TcpServerTest, test_connectionAccepted)
{
    TcpServer server(_serverSocketMock, _proxyConfig);
    StrictMock<AbstractSocketMock> socket;
    IPAddress addr2 = make_ip4(192U, 0U, 2U, 2U);

    uint16_t const LOCAL_PORT = 10U;
    EXPECT_CALL(socket, disableNagleAlgorithm());
    EXPECT_CALL(socket, getLocalPort()).Times(1).WillOnce(Return(LOCAL_PORT));
    EXPECT_CALL(socket, getRemotePort()).Times(1).WillOnce(Return(20U));
    EXPECT_CALL(socket, getRemoteIPAddress()).Times(1).WillOnce(Return(addr2));

    server.connectionAccepted(socket);
}

TEST_F(TcpServerTest, ConnectionAccepted_shouldIncProxyRefCounter)
{
    TcpServer server(_serverSocketMock, _proxyConfig);
    AbstractSocketMock& socket = _socketArray[0];
    TcpProxy& tcpProxy         = _proxyList[0];

    EXPECT_CALL(socket, isClosed()).WillRepeatedly(Return(false));

    server.connectionAccepted(socket);

    EXPECT_CALL(socket, close()).Times(0);

    tcpProxy.tryClose();
}

TEST_F(TcpServerTest, ConnectionChanged_shouldDecProxyRefCounter)
{
    TcpServer server(_serverSocketMock, _proxyConfig);
    AbstractSocketMock& socket = _socketArray[0];
    TcpProxy& tcpProxy         = _proxyList[0];

    // new socket connection accepted increase the resource counter by 1
    EXPECT_CALL(socket, isClosed()).WillRepeatedly(Return(false));
    server.connectionAccepted(socket);

    // connection changed notification decrease the resource counter by 1
    // this notification will be triggered by the tcpProxy when it will receive the connectionClosed
    // event from the socket, at this point socket will be already closed
    EXPECT_CALL(socket, isClosed()).WillRepeatedly(Return(true));
    server.connectionChanged(tcpProxy);

    // socket should be closed when we try to close tcpProxy because resource counter is 0
    // simulate the socket state as open, just to check that close method will be called.
    EXPECT_CALL(socket, close()).Times(1);
    EXPECT_CALL(socket, isClosed()).WillRepeatedly(Return(false));

    tcpProxy.tryClose();
}

} // anonymous namespace
