/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpProxy.h"

#include "someip/NetworkListenerMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/TcpProxyMock.h"

#include <ip/IPAddress.h>
#include <tcp/socket/AbstractSocket.h>
#include <tcp/socket/AbstractSocketMock.h>

#include <etl/array.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

MATCHER_P(SliceSize, m, "") { return Matches(m)(arg.size()); }

/**
 * Make sure TcpProxy is only initialized if it has an input and output buffer != 0U and a listener.
 */
TEST(TcpProxy, test_isInitialized)
{
    ::tcp::AbstractSocketMock socketMock;
    TcpProxy proxy(socketMock);

    // nothing is initialized
    EXPECT_FALSE(proxy.isInitialized());

    // now initialize NetworkResource
    StrictMock<NetworkListenerMock> listener;

    ::etl::array<uint8_t, 10U> input{}, output{}, internal{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setInternalBuffer(internal);
    proxy.setListener(listener);

    // slice it off
    NetworkResource* res = &proxy;
    EXPECT_TRUE(res->NetworkResource::isInitialized());
    EXPECT_TRUE(proxy.isInitialized());
}

/**
 * Make sure sending is unsuccessful if proxy is not initialized.
 */
TEST(TcpProxy, send_fails_if_uninitialized)
{
    ::tcp::AbstractSocketMock socketMock;
    TcpProxy proxy(socketMock);

    // not initialized
    EXPECT_FALSE(proxy.send(16U));
}

/**
 * Make sure sending 0 bytes is unsuccessful.
 */
TEST(TcpProxy, sending_zero_bytes_fails)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    StrictMock<NetworkListenerMock> listener;
    ::etl::array<uint8_t, 10U> input{}, output{}, internal{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setInternalBuffer(internal);
    proxy.setListener(listener);

    EXPECT_CALL(socketMock, isEstablished()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_TRUE(proxy.isConnected());

    // can't send 0 bytes.
    EXPECT_FALSE(proxy.send(0U));
}

TEST(TcpProxy, SendSocketError)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    StrictMock<NetworkListenerMock> listener;
    ::etl::array<uint8_t, 10U> input{}, output{}, internal{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setInternalBuffer(internal);
    proxy.setListener(listener);

    EXPECT_CALL(socketMock, isEstablished()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(socketMock, send(SliceSize(Eq(5))))
        .Times(1)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK));

    EXPECT_TRUE(proxy.isConnected());
    EXPECT_FALSE(proxy.send(5U));
}

TEST(TcpProxy, ConnectionListener)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    StrictMock<NetworkListenerMock> listener;
    ::etl::array<uint8_t, 10U> input{}, output{}, internal{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setInternalBuffer(internal);
    proxy.setListener(listener);
    StrictMock<TcpProxyConnectionListenerMock> listener2;
    proxy.setConnectionListener(&listener2);

    IPAddress localIp  = make_ip4(192U, 0U, 2U, 1U);
    uint16_t localPort = 10U;
    IPEndpoint local(localIp, localPort);

    IPAddress remoteIp  = make_ip4(192U, 0U, 2U, 2U);
    uint16_t remotePort = 20U;
    IPEndpoint remote(remoteIp, remotePort);

    // open
    EXPECT_CALL(socketMock, isClosed()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(socketMock, bind(localIp, localPort))
        .Times(1)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(socketMock, disableNagleAlgorithm()).Times(2);
    ::tcp::AbstractSocket::ConnectedDelegate connected;
    EXPECT_CALL(socketMock, connect(remoteIp, remotePort, _))
        .Times(1)
        .WillOnce(
            DoAll(SaveArg<2>(&connected), Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
    EXPECT_TRUE(proxy.open(local, remote));
    EXPECT_CALL(socketMock, getLocalPort()).Times(1).WillOnce(Return(localPort));
    EXPECT_CALL(listener2, connectionChanged(Ref(proxy))).Times(1);
    connected(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK);

    // close
    EXPECT_CALL(socketMock, isClosed()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(socketMock, close())
        .Times(1)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(listener2, connectionChanged(Ref(proxy))).Times(1);
    proxy.close();
}

/**
 * Make sure getProto returns SD_L4_PROTO_TCP for TcpProxy.
 */
TEST(TcpProxy, test_getProto)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    EXPECT_EQ(static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP), proxy.getProto());
}

TEST(TcpProxy, Send)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    IPAddress address = make_ip4(192U, 0U, 2U, 1U);
    uint16_t port     = 30000U;
    // can't send 0 bytes.
    EXPECT_FALSE(proxy.send(IPEndpoint(address, port), 0U));
}

TEST(TcpProxy, ConnectionClosed)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    StrictMock<TcpProxyConnectionListenerMock> listener2;
    proxy.setConnectionListener(&listener2);
    EXPECT_CALL(listener2, connectionChanged(Ref(proxy))).Times(1);

    proxy.connectionClosed(::tcp::IDataListener::ErrorCode::ERR_CONNECTION_CLOSED);
}

TEST(TcpProxy, DataReceived_MessageLengthTooShort)
{
    StrictMock<::tcp::AbstractSocketMock> socketMock;
    TcpProxy proxy(socketMock);
    ::etl::array<uint8_t, 10U> input, output, internal;
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setInternalBuffer(internal);

    IPAddress address = make_ip4(192U, 0U, 2U, 1U);
    uint16_t port     = 30000U;

    EXPECT_CALL(socketMock, getRemoteIPAddress()).WillOnce(Return(address));
    EXPECT_CALL(socketMock, getRemotePort()).WillOnce(Return(port));
    EXPECT_CALL(socketMock, read(_, _)).WillRepeatedly(Return(5U));
    proxy.dataReceived(5U);
}

} // anonymous namespace
