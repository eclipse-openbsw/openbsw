/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcChannel.h"

#include "someip/CallDoneClosureMock.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResource.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcSenderMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpSerializableMock.h"

#include <ip/IPEndpoint.h>

#include <etl/optional.h>
#include <etl/span.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;
using namespace ::ip;

struct RpcChannelTest : ::testing::Test
{
    StrictMock<NetworkMock> _network;
    StrictMock<RpcSenderMock> _rpcSender;

    RpcChannel _channel{_network, _rpcSender};
};

TEST_F(RpcChannelTest, OpenTcpWithExternalBuffer)
{
    uint16_t const serviceId = 1U;
    uint16_t const port      = 2U;
    ::ip::IPEndpoint ep;
    uint8_t data[1U];
    ::etl::span<uint8_t> buffer(data);
    ::etl::span<uint8_t> actualBuffer;
    EXPECT_CALL(_network, openTcpChannelWithExternalReassembleBuffer(port, ep, _))
        .WillOnce(DoAll(SaveArg<2U>(&actualBuffer), Return(::etl::nullopt)));
    _channel.openTcpWithExternalReassembleBuffer(serviceId, ep, port, buffer);
    EXPECT_EQ(buffer.data(), actualBuffer.data());
    EXPECT_EQ(buffer.size(), actualBuffer.size());
}

TEST_F(RpcChannelTest, GetRemoteIp_NoNetworkChannel)
{
    ::ip::IPEndpoint endpoint = _channel.getRemoteIp();
    EXPECT_EQ(endpoint.getPort(), NetworkResource::INVALID_ADDRESS.getPort());
    EXPECT_EQ(endpoint.getAddress(), NetworkResource::INVALID_ADDRESS.getAddress());
    EXPECT_EQ(NetworkResource::INVALID_ADDRESS, endpoint);
}

TEST_F(RpcChannelTest, GetLocalPort_NoNetworkChannel)
{
    auto result = _channel.getLocalPort();
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(PortError::NOT_INITIALIZED, result.error());
}

TEST_F(RpcChannelTest, CallMethod_NoNetworkChannel)
{
    StrictMock<CallDoneClosureMock> done;
    ServiceResultCode result = _channel.callMethod(4U, nullptr, 1U, nullptr, done, 1000U);

    EXPECT_EQ(ServiceResultCode::COULD_NOT_DELIVER, result);
}

TEST_F(RpcChannelTest, CallFireAndForgetMethod_NoNetworkChannel)
{
    ServiceResultCode result = _channel.callFireAndForgetMethod(4U, nullptr, 1U);

    EXPECT_EQ(ServiceResultCode::COULD_NOT_DELIVER, result);
}

TEST_F(RpcChannelTest, IsOpen)
{
    EXPECT_FALSE(_channel.isOpen());
    ::etl::optional<::someip::NetworkChannel> channel;
    EXPECT_CALL(_network, openUdpChannel(13U, _channel.getRemoteIp())).WillOnce(Return(channel));
    _channel.openUdp(0x1234, _channel.getRemoteIp(), 13U);
    EXPECT_FALSE(_channel.isOpen());
}

TEST_F(RpcChannelTest, GetProtoPort)
{
    EXPECT_EQ(static_cast<uint8_t>(SomeIpConstants::INVALID_PROTO), _channel.getProto());
    auto result = _channel.getLocalPort();
    EXPECT_FALSE(result.has_value());

    uint16_t const serviceId = 1U;
    uint16_t const port      = 2U;
    ::ip::IPEndpoint const endPoint(make_ip4(192U, 0U, 2U, 0U), port);
    NetworkResourceMock networkResourceMock;
    ::etl::optional<NetworkChannel> channel;
    channel.emplace(networkResourceMock, endPoint, false);

    EXPECT_CALL(_network, openUdpChannel(_, _)).WillOnce(Return(channel));
    _channel.openUdp(serviceId, endPoint, port);

    EXPECT_CALL(networkResourceMock, getLocalPort())
        .WillOnce(Return(::etl::expected<uint16_t, PortError>(2U)));
    auto portResult = _channel.getLocalPort();
    ASSERT_TRUE(portResult.has_value());
    EXPECT_EQ(port, portResult.value());

    EXPECT_CALL(networkResourceMock, getProto()).WillOnce(Return(proto::SD_L4_PROTO_UDP));
    EXPECT_EQ(static_cast<uint8_t>(proto::SD_L4_PROTO_UDP), _channel.getProto());
    // release NetworkChannel copy holding the networkResourceMock which is on the stack.
    EXPECT_CALL(networkResourceMock, close()).Times(1);
    _channel.close();
    Mock::VerifyAndClearExpectations(&_network);
}

TEST_F(RpcChannelTest, GetResponse)
{
    EXPECT_EQ(nullptr, _channel.getResponse());

    uint16_t const serviceId = 1U;
    uint16_t const port      = 2U;
    ::ip::IPEndpoint const endPoint(make_ip4(192U, 0U, 2U, 0U), port);
    NetworkResourceMock networkResourceMock;
    ::etl::optional<NetworkChannel> channel;
    channel.emplace(networkResourceMock, endPoint, false);

    EXPECT_CALL(_network, openUdpChannel(_, _)).WillOnce(Return(channel));
    _channel.openUdp(serviceId, endPoint, port);

    StrictMock<CallDoneClosureMock> done;
    SomeIpSerializableMock request;
    EXPECT_CALL(_rpcSender, sendRequest(_, _, _, _, _, _, _))
        .WillOnce(Return(ServiceResultCode::RPC_SENT_SUCCESSFULLY));
    ServiceResultCode result = _channel.callMethod(4U, nullptr, 1U, &request, done, 1000U);

    EXPECT_EQ(ServiceResultCode::RPC_SENT_SUCCESSFULLY, result);
    EXPECT_EQ(&request, _channel.getResponse());
    // release NetworkChannel copy holding the networkResourceMock which is on the stack.
    EXPECT_CALL(networkResourceMock, close()).Times(1);
    _channel.close();
    Mock::VerifyAndClearExpectations(&_network);
}

TEST_F(RpcChannelTest, OpenClose)
{
    EXPECT_EQ(0U, _channel.getServiceId());

    ::etl::optional<::someip::NetworkChannel> channelUdp;
    EXPECT_CALL(_network, openUdpChannel(13U, _channel.getRemoteIp())).WillOnce(Return(channelUdp));
    _channel.openUdp(0x1234, _channel.getRemoteIp(), 13U);
    EXPECT_EQ(0x1234, _channel.getServiceId());

    ::etl::optional<::someip::NetworkChannel> channelTcp;
    EXPECT_CALL(_network, openTcpChannel(26U, _channel.getRemoteIp())).WillOnce(Return(channelTcp));
    _channel.openTcp(0x2345, _channel.getRemoteIp(), 26U);
    EXPECT_EQ(0x2345, _channel.getServiceId());

    _channel.close();

    EXPECT_EQ(0U, _channel.getServiceId());
}

TEST_F(RpcChannelTest, ClientId)
{
    uint16_t clientId = 0x13;
    _channel.setClientId(clientId);
    EXPECT_EQ(clientId, _channel.getClientId());
}

} // anonymous namespace
