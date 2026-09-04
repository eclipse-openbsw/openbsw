/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcChannelMock.h"
#include "someip/RpcHandler.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/SomeIpMessage.h"
#include "someip/TpTransceiverMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <ip/IPAddress.h>

#include <etl/array.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::ip;
using namespace ::common;
using namespace ::someip;
using namespace ::testing;

struct ChannelSetupTest : Test
{
    ChannelSetupTest()
    : _endpoint(make_ip4(192U, 0U, 2U, 0U), 10U), _asyncMock(), _testContext(_ethernetContext)
    {
        _testContext.handleAll();
    }

    void setupChannel(IRpcChannel& channel);

    StrictMock<NetworkMock> _network;
    StrictMock<TpTransceiverMock> _tpTransceiver;
    async::ContextType _ethernetContext{0U};
    ::someip::declare::ServiceManager<1U> _serviceManager;
    NiceMock<ServiceRegistryMock> _serviceRegistry;
    RpcHandler _rpcHandler{
        _network, _ethernetContext, _tpTransceiver, _serviceManager, _serviceRegistry};
    IPEndpoint _endpoint;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

void ChannelSetupTest::setupChannel(IRpcChannel& channel)
{
    StrictMock<NetworkResourceMock> resource;
    resource.incRefCounter();
    EXPECT_CALL(resource, isOpen()).Times(AnyNumber()).WillRepeatedly(Return(true));
    EXPECT_CALL(resource, isConnected()).Times(AnyNumber()).WillRepeatedly(Return(false));
    EXPECT_CALL(resource, send(_, _)).Times(1).WillOnce(Return(true));

    ::etl::optional<NetworkChannel> optional(NetworkChannel(resource, _endpoint));
    EXPECT_CALL(_network, getRpcChannel(_, _, _)).Times(1).WillOnce(Return(optional));

    EXPECT_CALL(resource, getProto()).Times(1).WillOnce(Return(0x11));
    EXPECT_TRUE(
        _rpcHandler.sendRequest(nullptr, channel.getServiceId(), 2U, 3U, true, channel, 1000U));
}

/**
 * Test response if channels are missing.
 */
TEST_F(ChannelSetupTest, test_handleResponse_with_no_channels)
{
    uint8_t data[] = {
        0x12, 0x34, 0x56, 0x78, // service id, method id
        0x00, 0x00, 0x00, 0x0C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x01, 0x02, 0x03, 0x04, // payload
    };
    SomeIpMessage message(data);

    RpcHandler::ErrorCode errorCode = _rpcHandler.handleResponse(message, _endpoint, 20U);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE, errorCode);
}

/**
 * Test response if ip is wrong.
 */
TEST_F(ChannelSetupTest, test_handleResponse_with_wrong_ip)
{
    uint8_t data[] = {
        0x12, 0x34, 0x56, 0x78, // service id, method id
        0x00, 0x00, 0x00, 0x0C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x01, 0x02, 0x03, 0x04, // payload
    };
    SomeIpMessage message(data);
    uint16_t localPort = 20U;

    StrictMock<RpcChannelMock> channel;
    ISomeIpSerializable* nothing = nullptr;

    EXPECT_CALL(channel, getServiceId()).Times(AnyNumber()).WillRepeatedly(Return(0x1234));
    EXPECT_CALL(channel, getClientId()).Times(AnyNumber()).WillRepeatedly(Return(0U));
    EXPECT_CALL(channel, getRemoteIp()).Times(AnyNumber()).WillRepeatedly(ReturnRef(_endpoint));
    EXPECT_CALL(channel, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(localPort));
    EXPECT_CALL(channel, setTimeout(_, _)).Times(AnyNumber());
    EXPECT_CALL(channel, cancelTimeout()).Times(AnyNumber());
    EXPECT_CALL(channel, getProto()).Times(AnyNumber()).WillRepeatedly(Return(0x11));
    EXPECT_CALL(channel, getResponse()).Times(AnyNumber()).WillRepeatedly(Return(nothing));
    EXPECT_CALL(channel, getSessionId()).Times(AnyNumber()).WillRepeatedly(Return(0U));

    setupChannel(channel);
    EXPECT_EQ(1U, _rpcHandler.getNumRegisteredChannels());

    IPEndpoint wrongEndpoint(make_ip4(192U, 0U, 2U, 1U), 10U);
    RpcHandler::ErrorCode errorCode = _rpcHandler.handleResponse(message, wrongEndpoint, localPort);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE, errorCode);

    // free the channel
    EXPECT_CALL(channel, responseReceived(ServiceResultCode::RPC_POSITIVE_RESPONSE)).Times(1);
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK,
        _rpcHandler.handleResponse(message, _endpoint, localPort));
}

/**
 * Test response if service ID is wrong.
 */
TEST_F(ChannelSetupTest, test_handleResponse_with_wrong_service_id)
{
    uint8_t data[] = {
        0x12, 0x35, 0x56, 0x78, // service id, method id
        0x00, 0x00, 0x00, 0x0C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x01, 0x02, 0x03, 0x04, // payload
    };
    SomeIpMessage message(data);
    uint16_t localPort = 20U;

    StrictMock<RpcChannelMock> channel;
    ISomeIpSerializable* nothing = nullptr;

    EXPECT_CALL(channel, getServiceId()).Times(AnyNumber()).WillRepeatedly(Return(0x1234));
    EXPECT_CALL(channel, getClientId()).Times(AnyNumber()).WillRepeatedly(Return(0U));
    EXPECT_CALL(channel, getRemoteIp()).Times(AnyNumber()).WillRepeatedly(ReturnRef(_endpoint));
    EXPECT_CALL(channel, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(localPort));
    EXPECT_CALL(channel, setTimeout(_, _)).Times(AnyNumber());
    EXPECT_CALL(channel, cancelTimeout()).Times(AnyNumber());
    EXPECT_CALL(channel, getProto()).Times(AnyNumber()).WillRepeatedly(Return(0x11));
    EXPECT_CALL(channel, getResponse()).Times(AnyNumber()).WillRepeatedly(Return(nothing));
    EXPECT_CALL(channel, getSessionId()).Times(AnyNumber()).WillRepeatedly(Return(0U));

    setupChannel(channel);
    EXPECT_EQ(1U, _rpcHandler.getNumRegisteredChannels());

    RpcHandler::ErrorCode errorCode = _rpcHandler.handleResponse(message, _endpoint, localPort);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE, errorCode);

    // free the channel
    data[1] = 0x34;
    EXPECT_CALL(channel, responseReceived(ServiceResultCode::RPC_POSITIVE_RESPONSE)).Times(1);
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK,
        _rpcHandler.handleResponse(message, _endpoint, localPort));
}

/**
 * Test response if local port is wrong.
 */
TEST_F(ChannelSetupTest, test_handleResponse_with_wrong_local_port)
{
    uint8_t data[] = {
        0x12, 0x34, 0x56, 0x78, // service id, method id
        0x00, 0x00, 0x00, 0x0C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x01, 0x02, 0x03, 0x04, // payload
    };
    SomeIpMessage message(data);
    uint16_t localPort = 20U;

    StrictMock<RpcChannelMock> channel;
    ISomeIpSerializable* nothing = nullptr;

    EXPECT_CALL(channel, getServiceId()).Times(AnyNumber()).WillRepeatedly(Return(0x1234));
    EXPECT_CALL(channel, getClientId()).Times(AnyNumber()).WillRepeatedly(Return(0U));
    EXPECT_CALL(channel, getRemoteIp()).Times(AnyNumber()).WillRepeatedly(ReturnRef(_endpoint));
    EXPECT_CALL(channel, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(localPort));
    EXPECT_CALL(channel, getProto()).Times(AnyNumber()).WillRepeatedly(Return(0x11));
    EXPECT_CALL(channel, getResponse()).Times(AnyNumber()).WillRepeatedly(Return(nothing));
    EXPECT_CALL(channel, getSessionId()).Times(AnyNumber()).WillRepeatedly(Return(0U));

    EXPECT_CALL(channel, setTimeout(_ethernetContext, 1000U));
    setupChannel(channel);
    EXPECT_EQ(1U, _rpcHandler.getNumRegisteredChannels());

    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_ERROR,
        _rpcHandler.handleResponse(message, _endpoint, localPort + 1U));

    EXPECT_CALL(channel, cancelTimeout());
    EXPECT_CALL(channel, responseReceived(ServiceResultCode::RPC_POSITIVE_RESPONSE)).Times(1);
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK,
        _rpcHandler.handleResponse(message, _endpoint, localPort));
}

/**
 * Test response if session ID is wrong.
 */
TEST_F(ChannelSetupTest, test_handleResponse_with_wrong_session_id)
{
    uint8_t data[] = {
        0x12, 0x34, 0x56, 0x78, // service id, method id
        0x00, 0x00, 0x00, 0x0C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x01, 0x02, 0x03, 0x04, // payload
    };
    uint16_t wrongSessionId    = 2U;
    uint16_t expectedSessionId = 1U;

    SomeIpMessage message(data);
    message.setSessionId(wrongSessionId);

    uint16_t localPort = 20U;

    StrictMock<RpcChannelMock> channel;
    ISomeIpSerializable* nothing = nullptr;

    EXPECT_CALL(channel, getServiceId()).Times(AnyNumber()).WillRepeatedly(Return(0x1234));
    EXPECT_CALL(channel, getClientId()).Times(AnyNumber()).WillRepeatedly(Return(0U));
    EXPECT_CALL(channel, getRemoteIp()).Times(AnyNumber()).WillRepeatedly(ReturnRef(_endpoint));
    EXPECT_CALL(channel, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(localPort));
    EXPECT_CALL(channel, setTimeout(_, _)).Times(1);
    EXPECT_CALL(channel, getProto()).Times(AnyNumber()).WillRepeatedly(Return(0x11));
    EXPECT_CALL(channel, getResponse()).Times(AnyNumber()).WillRepeatedly(Return(nothing));
    EXPECT_CALL(channel, getSessionId()).WillRepeatedly(Return(1U));

    setupChannel(channel);
    EXPECT_EQ(1U, _rpcHandler.getNumRegisteredChannels());

    RpcHandler::ErrorCode errorCode = _rpcHandler.handleResponse(message, _endpoint, localPort);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE, errorCode);

    // free the channel
    message.setSessionId(expectedSessionId);
    EXPECT_CALL(channel, responseReceived(ServiceResultCode::RPC_POSITIVE_RESPONSE)).Times(1);
    EXPECT_CALL(channel, cancelTimeout());
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK,
        _rpcHandler.handleResponse(message, _endpoint, localPort));
}

} // anonymous namespace
