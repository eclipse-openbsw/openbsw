/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/Network.h"

#include "someip/NetworkConfig.h"
#include "someip/NetworkListenerMock.h"
#include "someip/SdEndpoint.h"

#include <ip/IPAddress.h>
#include <ip/IPEndpoint.h>
#include <tcp/socket/AbstractServerSocketMock.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

namespace
{
using namespace testing;
using namespace ::ip;

class NetworkTest : public Test
{
public:
    NetworkTest()
    {
        _buffer = ::etl::span<uint8_t>(_data);
        _networkConfigPtr.reset(new SdConfig(_multicastAddr, _localAddr, 0U));
    }

protected:
    using SdConfig = ::someip::declare::SdNetworkConfig<
        StrictMock<::udp::AbstractDatagramSocketMock>,
        1U,
        StrictMock<::tcp::AbstractServerSocketMock>,
        1U,
        StrictMock<::tcp::AbstractSocketMock>,
        1U,
        1500U,
        0U,
        false,
        0U>;

    std::unique_ptr<SdConfig> _networkConfigPtr;
    ::ip::IPAddress _multicastAddr = make_ip4(224U, 1U, 255U, 255U);
    ::ip::IPAddress _localAddr     = make_ip4(192U, 0U, 2U, 0U);
    uint16_t const _remotePort     = 15U;
    uint16_t const _localPort      = 10U;
    uint16_t const _localPort2     = 11U;
    ::someip::NetworkListenerMock _listenerMock;
    uint8_t _data[1U];
    ::etl::span<uint8_t> _buffer;
};

TEST_F(NetworkTest, openTcpChannelWithExternalReassembleBuffer)
{
    ::ip::IPAddress addr = make_ip4(192U, 0U, 2U, 1U);
    ::ip::IPEndpoint endpoint(addr, _remotePort);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    networkConfig._tcpConfig._tcpProxies.getProxy(0U)->setListener(_listenerMock);
    EXPECT_TRUE(networkConfig._tcpConfig._tcpProxies.getProxy(0U)->isInitialized());

    ::tcp::AbstractSocketMock& socketMock = dynamic_cast<::tcp::AbstractSocketMock&>(
        networkConfig._tcpConfig._tcpProxies.getProxy(0U)->getSocket());

    {
        // get new connection
        EXPECT_CALL(socketMock, isClosed()).Times(4U).WillRepeatedly(Return(true));
        EXPECT_CALL(socketMock, getLocalPort()).Times(AtMost(1)).WillRepeatedly(Return(_localPort));
        EXPECT_CALL(socketMock, bind(_, _))
            .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
        EXPECT_CALL(socketMock, disableNagleAlgorithm());
        EXPECT_CALL(socketMock, connect(_, _, _))
            .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));

        ::etl::optional<::someip::NetworkChannel> channel
            = network.openTcpChannelWithExternalReassembleBuffer(_localPort, endpoint, _buffer);

        EXPECT_TRUE(channel.has_value());
        EXPECT_EQ(endpoint, channel->getRemoteEndpoint());
        EXPECT_EQ(
            _buffer.data(),
            networkConfig._tcpConfig._tcpProxies.getProxy(0U)->getInternalBuffer().data());
        EXPECT_EQ(
            _buffer.size(),
            networkConfig._tcpConfig._tcpProxies.getProxy(0U)->getInternalBuffer().size());

        Mock::VerifyAndClearExpectations(&socketMock);

        // get existing
        EXPECT_CALL(socketMock, getLocalPort()).Times(1U).WillRepeatedly(Return(_localPort));
        EXPECT_CALL(socketMock, getRemoteIPAddress()).Times(1U).WillOnce(Return(addr));
        EXPECT_CALL(socketMock, getRemotePort()).Times(1U).WillOnce(Return(_remotePort));
        EXPECT_CALL(socketMock, isClosed()).Times(2U).WillRepeatedly(Return(false));
        ::etl::optional<::someip::NetworkChannel> channel2
            = network.openTcpChannelWithExternalReassembleBuffer(_localPort, endpoint, _buffer);
        EXPECT_TRUE(channel2.has_value());
        EXPECT_EQ(endpoint, channel2->getRemoteEndpoint());
        EXPECT_CALL(socketMock, isClosed()).WillOnce(Return(false));
        EXPECT_CALL(socketMock, close());
    }
    Mock::VerifyAndClearExpectations(&socketMock);

    {
        // no sockets left
        EXPECT_CALL(socketMock, isClosed()).Times(2U).WillRepeatedly(Return(false));
        EXPECT_CALL(socketMock, getLocalPort()).Times(1U).WillRepeatedly(Return(_localPort));
        ::etl::optional<::someip::NetworkChannel> channel
            = network.openTcpChannelWithExternalReassembleBuffer(_localPort2, endpoint, _buffer);
        EXPECT_FALSE(channel.has_value());
    }
    Mock::VerifyAndClearExpectations(&socketMock);
}

TEST_F(NetworkTest, openTcpChannelWithExternalReassembleBuffer_NullptrProxy)
{
    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    ::tcp::AbstractSocketMock& socketMock = dynamic_cast<::tcp::AbstractSocketMock&>(
        networkConfig._tcpConfig._tcpProxies.getProxy(0U)->getSocket());

    EXPECT_CALL(socketMock, isClosed()).WillOnce(Return(false));
    // Non-existing port selected on purpose to allow proxy to be nullptr
    auto channel
        = network.openTcpChannelWithExternalReassembleBuffer(45999U, ::ip::IPEndpoint(), _buffer);
    // Ensure the function returns an empty optional when proxy is nullptr
    EXPECT_EQ(false, channel.has_value());
}

TEST_F(NetworkTest, initTcpPortWithExternalBuffers)
{
    ::etl::vector<::etl::span<uint8_t>, 1U> bufferPool;
    bufferPool.push_back(_buffer);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    network.initTcpPortWithExternalBuffers(_localPort, bufferPool);
    EXPECT_EQ(&bufferPool, networkConfig._tcpConfig._tcpServers.getBufferPool(_localPort));
    auto portResult = networkConfig._tcpConfig._tcpServers.getPort(0U);
    ASSERT_TRUE(portResult.has_value());
    EXPECT_EQ(_localPort, portResult.value());
}

TEST_F(NetworkTest, GetSd)
{
    ::ip::IPAddress addr = make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint endpoint(addr, _remotePort);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    auto sdPortResult = network.getSdPort(false);
    EXPECT_FALSE(sdPortResult.has_value());
    EXPECT_EQ(::someip::PortError::NOT_AVAILABLE, sdPortResult.error());
    auto channel = network.getSdChannel(0x1234, endpoint);
    EXPECT_EQ(false, channel.has_value());
}

TEST_F(NetworkTest, GetRpcChannel)
{
    ::ip::IPAddress addr = make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint endpoint(addr, _remotePort);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    uint8_t proto = static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_UDP);
    auto channel  = network.getRpcChannel(_remotePort, endpoint, proto);
    EXPECT_EQ(false, channel.has_value());
}

TEST_F(NetworkTest, OpenUdpChannel)
{
    ::ip::IPAddress addr = make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint endpoint(addr, _remotePort);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    auto udpChannel = network.openUdpChannel(_remotePort, endpoint);
    EXPECT_EQ(false, udpChannel.has_value());
}

TEST_F(NetworkTest, OpenUdpChannelAndJoinMulticastOnSameProxy)
{
    auto& rpcProxy = *_networkConfigPtr->_udpConfig._rpcProxies.getProxy(0U);
    StrictMock<::udp::AbstractDatagramSocketMock> rpcSocketMock;
    rpcProxy.setSocket(rpcSocketMock);
    ::etl::array<uint8_t, 10U> input{}, output{};
    rpcProxy.setInputBuffer(input);
    rpcProxy.setOutputBuffer(output);
    StrictMock<::someip::NetworkListenerMock> listener;
    rpcProxy.setListener(listener);

    uint16_t const port = 10U;
    ::ip::IPEndpoint endpoint(_localAddr, port);
    ::ip::IPEndpoint mcEndpoint(_multicastAddr, port);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    ::someip::Network network(networkConfig);

    bool isBound = false;
    EXPECT_CALL(rpcSocketMock, isBound()).WillRepeatedly(Invoke([&isBound]() { return isBound; }));
    EXPECT_CALL(rpcSocketMock, bind(_, port))
        .WillOnce(Invoke(
            [&isBound]()
            {
                isBound = true;
                return ::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
            }));

    auto udpChannel = network.openUdpChannel(port, endpoint);
    EXPECT_EQ(true, udpChannel.has_value());

    // opening it again with a multicast endpoint on the same port will reuse the same proxy and
    // join the multicast group with it
    EXPECT_CALL(rpcSocketMock, join(_multicastAddr))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(rpcSocketMock, getLocalPort()).WillOnce(Return(port));
    auto udpChannelMc = network.openUdpChannel(port, mcEndpoint);
    EXPECT_EQ(true, udpChannelMc.has_value());

    EXPECT_CALL(rpcSocketMock, close()).WillOnce(Return());
}

TEST_F(NetworkTest, InitSdPort)
{
    ::ip::IPAddress addr = make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint endpoint(addr, _remotePort);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    networkConfig._udpConfig._sdProxies.setListener(_listenerMock);
    ::someip::Network network(networkConfig);

    network.initSdPort(_localPort);
    auto portResult = _networkConfigPtr->_udpConfig._sdProxies.getPort(0U);
    ASSERT_TRUE(portResult.has_value());
    EXPECT_EQ(_localPort, portResult.value());

    auto& sdProxy = dynamic_cast<::udp::AbstractDatagramSocketMock&>(
        *(_networkConfigPtr->_udpConfig._sdProxies.getProxy(0U)->getSocket()));

    bool isBound = false;
    EXPECT_CALL(sdProxy, isBound()).WillRepeatedly(Invoke([&isBound]() { return isBound; }));
    EXPECT_CALL(sdProxy, bind(_, _localPort))
        .WillOnce(Invoke(
            [&isBound]()
            {
                isBound = true;
                return ::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
            }));
    EXPECT_CALL(sdProxy, join(_multicastAddr))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    network.start();

    // permanently opened, no close() on socket follows.
    _networkConfigPtr->_udpConfig._sdProxies.getProxy(0U)->tryClose();
}

TEST_F(NetworkTest, InitUdpPort)
{
    ::ip::IPAddress addr = make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint endpoint(addr, _remotePort);

    SdConfig& networkConfig = *(_networkConfigPtr.get());
    networkConfig._udpConfig._rpcProxies.setListener(_listenerMock);
    ::someip::Network network(networkConfig);

    network.initUdpPort(_localPort);
    auto portResult = _networkConfigPtr->_udpConfig._rpcProxies.getPort(0U);
    ASSERT_TRUE(portResult.has_value());
    EXPECT_EQ(_localPort, portResult.value());

    auto& rpcProxy = dynamic_cast<::udp::AbstractDatagramSocketMock&>(
        *(_networkConfigPtr->_udpConfig._rpcProxies.getProxy(0U)->getSocket()));
    EXPECT_CALL(rpcProxy, isBound()).WillRepeatedly(Return(false));
    EXPECT_CALL(rpcProxy, bind(_, _localPort))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    network.start();

    // permanently opened, no close() on socket follows.
    _networkConfigPtr->_udpConfig._rpcProxies.getProxy(0U)->tryClose();
}
} // anonymous namespace
