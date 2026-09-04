/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcReceiver.h"

#include "someip/DiagnosticListenerMock.h"
#include "someip/EventReceiverMock.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcHandler.h"
#include "someip/RpcHandlerMock.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/ServiceTracker.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/Statistics.h"
#include "someip/TpTransceiverMock.h"

#include <gtest/gtest.h>

#include <memory>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::common;
using namespace ::someip;

struct RpcReceiverTest : public ::testing::Test
{
    RpcReceiverTest()
    {
        _resource.incRefCounter();
        EXPECT_CALL(_network, setRpcListener(_)).Times(1);
        _handler.reset(new RpcHandler(
            _network, _ethernetContext, _tpTransceiver, _serviceManager, _serviceRegistry));

        _receiver.reset(new RpcReceiver(
            _network,
            _tpTransceiver,
            _eventReceiver,
            _serviceRegistry,
            *_handler,
            _multicastReceptions,
            &_diagnosticListener));
        Statistics::reset();
    }

    ::etl::flat_set<
        ::etl::optional<::someip::NetworkChannel>,
        2U,
        ::someip::internal::NetworkChannelComparator>
        _multicastReceptions;
    NetworkMock _network;
    async::ContextType _ethernetContext{0U};
    ::someip::declare::ServiceManager<1U> _serviceManager;
    NiceMock<ServiceRegistryMock> _serviceRegistry;
    StrictMock<TpTransceiverMock> _tpTransceiver;
    StrictMock<EventReceiverMock> _eventReceiver;
    std::unique_ptr<RpcHandler> _handler;
    std::unique_ptr<RpcReceiver> _receiver;
    someip::declare::ServiceTracker<1U> _serviceTracker;

    StrictMock<DiagnosticListenerMock> _diagnosticListener;
    StrictMock<NetworkResourceMock> _resource;
    NetworkChannel _channel{_resource, IPEndpoint(make_ip4(192U, 0U, 2U, 0U), 10U)};
};

TEST_F(RpcReceiverTest, Received_LengthSmallerThanHeaderSize)
{
    _receiver->init();
    _receiver->received(_channel, 5U);
    EXPECT_EQ(0U, Statistics::getCounter(Statistics::Counter::PDU_RX));
    _receiver->shutdown();
}

TEST_F(RpcReceiverTest, Received_TpMessage)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x0F);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);
    IPAddress localIp = make_ip4(192U, 0U, 2U, 1U);
    EXPECT_CALL(_network, getLocalIp()).Times(1).WillRepeatedly(ReturnRef(localIp));
    EXPECT_CALL(_resource, getProto()).Times(1);
    EXPECT_CALL(_resource, getLocalPort()).Times(2).WillRepeatedly(Return(0));
    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->receivedTpMessage(_channel, message);
}

TEST_F(RpcReceiverTest, Received_BytesLeftSmallerThanPayloadSizeNoErrorResponse)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x0F);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);

    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->received(_channel, 17U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_BytesLeftSmallerThanPayloadSizeNoErrorResponseNotification)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x0F);
    message.setMessageType(SomeIpMessage::MessageType::NOTIFICATION);

    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->received(_channel, 17U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_BytesLeftSmallerThanPayloadSizeSendRequestError)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x0F);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST);

    EXPECT_CALL(
        _diagnosticListener,
        onError(
            _channel.getRemoteEndpoint(), _, SomeIpMessage::ReturnCode::SOMEIP_E_MALFORMED_MESSAGE))
        .Times(1);

    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(false));
    _receiver->received(_channel, 17U);

    SomeIpMessage error(_resource.getOutputBuffer());
    EXPECT_EQ(SomeIpMessage::ReturnCode::SOMEIP_E_MALFORMED_MESSAGE, error.getReturnCode());

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_InvalidProtocolVersionSendRequestError)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION + 1U);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST);

    EXPECT_CALL(_diagnosticListener, onError(_, _, _)).Times(1);

    // this indicates that we are attempting to call send on the channel
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(false));
    _receiver->received(_channel, 0x28U);

    SomeIpMessage error(_resource.getOutputBuffer());
    EXPECT_EQ(SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_PROTOCOL_VERSION, error.getReturnCode());

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_InvalidProtocolVersionRequestNoReturnNoError)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION + 1U);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);

    // this indicates that we are NOT attempting to call send on the channel
    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->received(_channel, 0x28U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_InvalidProtocolVersionNotificationNoError)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION + 1U);
    message.setMessageType(SomeIpMessage::MessageType::NOTIFICATION);

    // this indicates that we are NOT attempting to call send on the channel
    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->received(_channel, 0x28U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_InvalidReturnCodeNoError)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_NOT_OK);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);

    StrictMock<RpcHandlerMock> priorityHandler;
    _receiver->setPriorityRpcHandler(priorityHandler);

    // this indicates that we are NOT attempting to call send on the channel
    EXPECT_CALL(_resource, isOpen()).Times(0);
    // handleMessage is not called as the return code does not equal SOMEIP_E_OK
    EXPECT_CALL(priorityHandler, handleMessage(_, _)).Times(0);
    _receiver->received(_channel, 0x28U);

    SomeIpMessage error(_resource.getOutputBuffer());
    EXPECT_EQ(SomeIpMessage::ReturnCode::SOMEIP_E_NOT_OK, error.getReturnCode());

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_ValidReturnCode)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_OK);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);

    StrictMock<RpcHandlerMock> priorityHandler;
    _receiver->setPriorityRpcHandler(priorityHandler);

    EXPECT_CALL(_resource, getProto()).Times(1);

    // handleMessage is called as the incoming message is correctly formatted
    EXPECT_CALL(priorityHandler, handleMessage(_, _)).Times(1);
    _receiver->received(_channel, 0x28U);

    SomeIpMessage error(_resource.getOutputBuffer());
    EXPECT_EQ(SomeIpMessage::ReturnCode::SOMEIP_E_OK, error.getReturnCode());

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, HandleMessage_PriorityHandler)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST);

    StrictMock<RpcHandlerMock> priorityHandler;
    _receiver->setPriorityRpcHandler(priorityHandler);

    EXPECT_CALL(_resource, getProto()).Times(1).WillRepeatedly(Return(proto::SD_L4_PROTO_UDP));
    EXPECT_CALL(priorityHandler, handleMessage(_, _))
        .Times(1)
        .WillOnce(Return(IRpcHandler::ErrorCode::RPC_HANDLER_OK));
    _receiver->received(_channel, 0x28U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, HandleMessage_RemovePriorityHandler)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x20U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST);

    StrictMock<RpcHandlerMock> priorityHandler;
    _receiver->setPriorityRpcHandler(priorityHandler);

    EXPECT_CALL(_resource, getProto()).Times(1).WillOnce(Return(proto::SD_L4_PROTO_UDP));

    EXPECT_CALL(priorityHandler, handleMessage(_, _));
    _receiver->received(_channel, 0x28U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));

    _receiver->removePriorityRpcHandler();

    IPAddress localIp = make_ip4(192U, 0U, 2U, 1U);
    EXPECT_CALL(_network, getLocalIp()).Times(1).WillOnce(ReturnRef(localIp));
    EXPECT_CALL(_resource, getLocalPort())
        .Times(2)
        .WillRepeatedly(Return(etl::expected<uint16_t, PortError>(8080)));
    EXPECT_CALL(_resource, getProto()).Times(2).WillRepeatedly(Return(proto::SD_L4_PROTO_UDP));
    EXPECT_CALL(_diagnosticListener, onError(_, _, _));
    EXPECT_CALL(_resource, isOpen()).Times(1);
    _receiver->received(_channel, 0x28U);

    EXPECT_EQ(2U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_MagicCookieFromClient_IsIgnored)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x8U);
    message.setMessageId(MAGIC_COOKIE_CLIENT_MESSAGE_ID);
    message.setRequestId(MAGIC_COOKIE_REQUEST_ID);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);

    // this indicates that we are NOT attempting to call send on the channel
    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->received(_channel, 0x16U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, Received_MagicCookieFromServer_IsIgnored)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x8U);
    message.setMessageId(MAGIC_COOKIE_SERVER_MESSAGE_ID);
    message.setRequestId(MAGIC_COOKIE_REQUEST_ID);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    message.setMessageType(SomeIpMessage::MessageType::NOTIFICATION);

    // this indicates that we are NOT attempting to call send on the channel
    EXPECT_CALL(_resource, isOpen()).Times(0);
    _receiver->received(_channel, 0x16U);

    EXPECT_EQ(1U, Statistics::getCounter(Statistics::Counter::PDU_RX));
}

TEST_F(RpcReceiverTest, MulticastReception)
{
    uint8_t proto = proto::SD_L4_PROTO_UDP;
    uint16_t port = 10U;
    IPEndpoint endpoint(make_ip4(224U, 1U, 255U, 253U), port);
    IPEndpoint endpoint2(make_ip4(224U, 1U, 255U, 254U), port);
    IPEndpoint endpoint3(make_ip4(224U, 1U, 255U, 255U), port);
    NetworkChannel nwc(_resource, endpoint);
    NetworkChannel nwc2(_resource, endpoint2);
    NetworkChannel nwc3(_resource, endpoint3);

    // Add 1st multicast reception
    EXPECT_CALL(_network, openUdpChannel(port, endpoint))
        .WillOnce(Invoke([nwc](uint16_t const, ::ip::IPEndpoint const&)
                         { return ::etl::optional<::someip::NetworkChannel>(nwc); }));
    EXPECT_EQ(true, _receiver->requestMulticastReception(endpoint));
    EXPECT_EQ(1U, _multicastReceptions.size());

    // Try to add it again (nothing should change)
    EXPECT_CALL(_network, openUdpChannel(port, endpoint)).Times(0);
    EXPECT_EQ(true, _receiver->requestMulticastReception(endpoint));
    EXPECT_EQ(1U, _multicastReceptions.size());

    // Add a 2nd multicast reception, but no channel is available
    EXPECT_CALL(_network, openUdpChannel(port, endpoint2))
        .WillOnce(Invoke([nwc2](uint16_t const, ::ip::IPEndpoint const&)
                         { return ::etl::optional<::someip::NetworkChannel>(); }));
    EXPECT_EQ(false, _receiver->requestMulticastReception(endpoint2));
    EXPECT_EQ(1U, _multicastReceptions.size());

    // Add a 2nd multicast reception
    EXPECT_CALL(_network, openUdpChannel(port, endpoint2))
        .WillOnce(Invoke([nwc2](uint16_t const, ::ip::IPEndpoint const&)
                         { return ::etl::optional<::someip::NetworkChannel>(nwc2); }));
    EXPECT_EQ(true, _receiver->requestMulticastReception(endpoint2));
    EXPECT_EQ(2U, _multicastReceptions.size());

    // Try to add a 3rd multicast reception
    // This should not be added, since the max_size is configured as 2
    EXPECT_EQ(true, _receiver->requestMulticastReception(endpoint3));
    EXPECT_EQ(2U, _multicastReceptions.size());

    // Remove the 1st multicast reception
    EXPECT_CALL(_network, getRpcChannel(port, endpoint, proto))
        .WillOnce(Invoke([nwc](uint16_t const, ::ip::IPEndpoint const&, uint8_t)
                         { return ::etl::optional<::someip::NetworkChannel>(nwc); }));
    _receiver->cancelMulticastReception(endpoint);
    EXPECT_EQ(1U, _multicastReceptions.size());
}

} // namespace
