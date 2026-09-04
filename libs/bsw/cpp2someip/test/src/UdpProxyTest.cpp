/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/UdpProxy.h"

#include "someip/NetworkListenerMock.h"
#include "someip/SomeIpConstants.h"

#include <ip/IPAddress.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <etl/array.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

/**
 * Make sure UdpProxy needs input buffer, output buffer, listener and socket in order to be
 * successfully initialized.
 */
TEST(UdpProxy, test_isInitialized)
{
    UdpProxy proxy;

    // nothing is initialized
    EXPECT_FALSE(proxy.isInitialized());

    EXPECT_FALSE(proxy.isConnected());

    // now initialize NetworkResource
    StrictMock<NetworkListenerMock> listener;

    ::etl::array<uint8_t, 10U> input{}, output{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setListener(listener);

    // slice it off
    NetworkResource* res = &proxy;
    EXPECT_TRUE(res->NetworkResource::isInitialized());
    EXPECT_FALSE(proxy.isInitialized());

    // set the socket
    StrictMock<::udp::AbstractDatagramSocketMock> socket;
    proxy.setSocket(socket);
    EXPECT_TRUE(proxy.isInitialized());
}

/**
 * Make sure sending is unsuccessful if UdpProx is not initialized and therefore not open.
 */
TEST(UdpProxy, sending_fails_if_proxy_not_open)
{
    UdpProxy proxy;

    // not initialized
    EXPECT_FALSE(proxy.send(16U));
}

/**
 * Make sure it is not possible to send no data.
 */
TEST(UdpProxy, sending_no_data_fails)
{
    UdpProxy proxy;
    StrictMock<NetworkListenerMock> listener;
    StrictMock<::udp::AbstractDatagramSocketMock> socket;
    ::etl::array<uint8_t, 10U> input{}, output{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setListener(listener);
    proxy.setSocket(socket);

    IPAddress address = make_ip4(192U, 0U, 2U, 1U);
    uint16_t port     = 30000U;
    IPEndpoint endpoint(address, port);

    EXPECT_CALL(socket, isBound()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_TRUE(proxy.isOpen());

    // can't send 0 bytes.
    EXPECT_FALSE(proxy.send(endpoint, 0U));
}

/**
 * Make sure sending is unsuccessful in case of socket error.
 */
TEST(UdpProxy, sending_fails_if_socket_error)
{
    UdpProxy proxy;
    StrictMock<NetworkListenerMock> listener;
    StrictMock<::udp::AbstractDatagramSocketMock> socket;
    ::etl::array<uint8_t, 10U> input{}, output{};
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setListener(listener);
    proxy.setSocket(socket);

    IPAddress address = make_ip4(192U, 0U, 2U, 1U);
    uint16_t port     = 30000U;
    IPEndpoint endpoint(address, port);

    EXPECT_CALL(socket, isBound()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(socket, send(An<::udp::DatagramPacket const&>()))
        .Times(1)
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK));

    EXPECT_TRUE(proxy.isOpen());
    EXPECT_FALSE(proxy.send(endpoint, 5U));
}

/**
 * Make sure getProto() returns SD_L4_PROTO_UDP.
 */
TEST(UdpProxy, test_getProto)
{
    UdpProxy proxy;
    EXPECT_EQ(static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_UDP), proxy.getProto());
}

/**
 * Test opening and closing proxy.
 */
TEST(UdpProxy, test_opening_and_closing_proxy)
{
    IPAddress address = make_ip4(192U, 0U, 2U, 1U);
    uint16_t port     = 30000U;
    IPEndpoint endpoint(address, port);
    UdpProxy proxy;
    EXPECT_FALSE(proxy.open(endpoint));

    StrictMock<NetworkListenerMock> listener;
    StrictMock<::udp::AbstractDatagramSocketMock> socket;
    ::etl::array<uint8_t, 10U> input, output;
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setListener(listener);
    proxy.setSocket(socket);

    EXPECT_CALL(socket, isBound()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_TRUE(proxy.open(endpoint));

    EXPECT_CALL(socket, isBound()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(socket, close());
    proxy.close();
}

/**
 * Make sure flush() is invoked if data to be received exceeds input buffer.
 * If not data is received, nothing happens.
 */
TEST(UdpProxy, DataReceived_Flush)
{
    IPAddress sourceAddress      = make_ip4(192U, 0U, 2U, 1U);
    IPAddress destinationAddress = make_ip4(192U, 0U, 2U, 41U);
    uint16_t sourcePort          = 30000U;
    UdpProxy proxy;
    StrictMock<NetworkListenerMock> listener;
    StrictMock<::udp::AbstractDatagramSocketMock> socket;
    ::etl::array<uint8_t, 10U> input, output;
    proxy.setInputBuffer(input);
    proxy.setOutputBuffer(output);
    proxy.setListener(listener);
    proxy.setSocket(socket);

    uint16_t length = 0U; // nothing happens
    proxy.dataReceived(socket, sourceAddress, sourcePort, destinationAddress, length);
    length = 16U;                               // length > inputBuffer.size() --> Fail
    EXPECT_CALL(socket, read(nullptr, length)); // in; flush()
    proxy.dataReceived(socket, sourceAddress, sourcePort, destinationAddress, length);

    length = 10U; // valid case
    EXPECT_CALL(listener, received(_, _));
    EXPECT_CALL(socket, read(_, _)).Times(AtLeast(1)).WillRepeatedly(Return(10U));
    proxy.dataReceived(socket, sourceAddress, sourcePort, destinationAddress, length);
}

} // anonymous namespace
