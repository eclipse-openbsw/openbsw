/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcHandler.h"

#include "someip/EventReceiverMock.h"
#include "someip/ISomeIpSerializable.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcChannelMock.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/SomeIpSerializableMock.h"
#include "someip/TpTransceiverMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <ip/IPAddress.h>

#include <gtest/esr_extensions.h>
#include <gtest/gtest.h>

namespace
{

using namespace ::testing;
using namespace ::common;
using namespace someip;
using namespace ::ip;

namespace
{
// clang-format off
uint8_t requestData[] = {
    0x12, 0x34, 0x56, 0x78, // service id, method id
    0x00, 0x00, 0x00, 0x0C, // length
    0x00, 0x00, 0x00, 0x00, // request id
    0x00, 0x01, 0x00, 0x00, // version, message type = REQUEST, return code
    0x01, 0x02, 0x03, 0x04, // payload
};
// clang-format on

uint16_t const serviceId       = 0x1234;
uint16_t const methodId        = 0x5678;
uint8_t const serviceProto     = proto::SD_L4_PROTO_UDP;
uint16_t const destinationPort = 20U;
IPAddress const remoteIp       = make_ip4(192U, 0U, 2U, 0U);
IPEndpoint const clientEndpoint(remoteIp, destinationPort);
} // namespace

class RpcHandlerTest : public Test
{
public:
    RpcHandlerTest() : _asyncMock(), _testContext(_ethernetContext)
    {
        _providedService                          = ProvidedService(_handlerMock);
        _providedService.description.serviceId    = serviceId;
        _providedService.description.majorVersion = 1U;
        _providedService.description.instanceId   = 1U;
        _providedService.description.port  = destinationPort; // local port == destination port
        _providedService.description.proto = serviceProto;

        EXPECT_TRUE(_serviceManager.registerService(_providedService));
        _resourceMock.incRefCounter();

        _testContext.handleAll();
    }

protected:
    ::someip::CallDoneClosure* getRequestCallback();

    StrictMock<ServiceHandlerMock<1U>> _handlerMock;
    ::someip::NetworkResourceMock _resourceMock;
    StrictMock<NetworkMock> _network;
    StrictMock<TpTransceiverMock> _tpTransceiver;
    async::ContextType _ethernetContext{0U};
    ::someip::declare::ServiceManager<1U> _serviceManager;
    NiceMock<ServiceRegistryMock> _serviceRegistry;
    RpcHandler _rpcHandler{
        _network, _ethernetContext, _tpTransceiver, _serviceManager, _serviceRegistry};
    ProvidedService _providedService;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

TEST_F(RpcHandlerTest, testHandleRequest)
{
    ISomeIpSerializable* nothing = nullptr;
    ::someip::MethodDetail details
        = {methodId,
           ::someip::SomeIpMethodType::METHOD_NO_PARAMETERS,
           ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE};

    EXPECT_CALL(_handlerMock, getRequest(methodId)).Times(1U).WillOnce(Return(nothing));
    EXPECT_CALL(_handlerMock, getMethodDetail(methodId)).WillRepeatedly(Return(&details));
    EXPECT_CALL(_handlerMock, getResponse(methodId)).Times(1U).WillOnce(Return(nothing));
    EXPECT_CALL(_handlerMock, callMethod(methodId, nothing, nothing, _)).Times(1U);

    SomeIpMessage message(requestData);

    RpcHandler::ErrorCode const errorCode
        = _rpcHandler.handleRequest(message, clientEndpoint, destinationPort, serviceProto);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_OK, errorCode);
}

/**
 * Make sure RpcHandler handles unknown method correctly.
 */
TEST_F(RpcHandlerTest, handleRequest_unknown_method)
{
    EXPECT_CALL(_handlerMock, getMethodDetail(methodId)).WillRepeatedly(Return(nullptr));

    SomeIpMessage message(requestData);

    RpcHandler::ErrorCode const errorCode
        = _rpcHandler.handleRequest(message, clientEndpoint, destinationPort, serviceProto);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_UNKNOWN_METHOD, errorCode);
}

/**
 * Make sure SEMANTIC_REQUEST_RESPONSE will be
 * handled as RPC_HANDLER_WRONG_MESSAGE_TYPE_REQUEST_RESPONSE.
 */
TEST_F(RpcHandlerTest, handleRequest_SEMANTIC_REQUEST_RESPONSE)
{
    ::someip::MethodDetail details
        = {methodId,
           ::someip::SomeIpMethodType::METHOD_NO_PARAMETERS,
           ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE};

    EXPECT_CALL(_handlerMock, getMethodDetail(methodId)).WillRepeatedly(Return(&details));

    SomeIpMessage message(requestData);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);
    RpcHandler::ErrorCode const errorCode
        = _rpcHandler.handleRequest(message, clientEndpoint, destinationPort, serviceProto);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_REQUEST_RESPONSE, errorCode);
}

/**
 * Make sure SEMANTIC_FIRE_AND_FORGET will be
 * handled as RPC_HANDLER_WRONG_MESSAGE_TYPE_FIRE_AND_FORGET.
 */
TEST_F(RpcHandlerTest, handleRequest_SEMANTIC_FIRE_AND_FORGET)
{
    ::someip::MethodDetail details
        = {methodId,
           ::someip::SomeIpMethodType::METHOD_NO_PARAMETERS,
           ::someip::SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET};

    EXPECT_CALL(_handlerMock, getMethodDetail(methodId)).WillRepeatedly(Return(&details));

    SomeIpMessage message(requestData);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST);
    RpcHandler::ErrorCode const errorCode
        = _rpcHandler.handleRequest(message, clientEndpoint, destinationPort, serviceProto);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_FIRE_AND_FORGET, errorCode);
}

/**
 * Make sure it is handled as RPC_HANDLER_ERROR if no callbacks are available.
 */
TEST_F(RpcHandlerTest, handleRequest_no_callbacks_available)
{
    EXPECT_TRUE(_handlerMock.hasAvailableCallback());

    // use up the only callback
    _handlerMock.getCallback();
    EXPECT_FALSE(_handlerMock.hasAvailableCallback());

    ::someip::MethodDetail details
        = {methodId,
           ::someip::SomeIpMethodType::METHOD_NO_PARAMETERS,
           ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE};
    EXPECT_CALL(_handlerMock, getMethodDetail(methodId)).WillRepeatedly(Return(&details));

    SomeIpMessage message(requestData);
    RpcHandler::ErrorCode const errorCode
        = _rpcHandler.handleRequest(message, clientEndpoint, destinationPort, serviceProto);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_ERROR, errorCode);
}

::someip::CallDoneClosure* RpcHandlerTest::getRequestCallback()
{
    ISomeIpSerializable* nothing = nullptr;
    ::someip::MethodDetail details
        = {methodId,
           ::someip::SomeIpMethodType::METHOD_NO_PARAMETERS,
           ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE};

    ::someip::CallDoneClosure* requestCallbackPtr = nullptr;

    EXPECT_CALL(_handlerMock, getRequest(methodId)).Times(1U).WillOnce(Return(nothing));
    EXPECT_CALL(_handlerMock, getMethodDetail(methodId)).WillRepeatedly(Return(&details));
    EXPECT_CALL(_handlerMock, getResponse(methodId)).Times(1U).WillOnce(Return(nothing));
    EXPECT_CALL(_handlerMock, callMethod(methodId, nothing, nothing, _))
        .Times(1U)
        .WillOnce(SaveRef<3>(&requestCallbackPtr));

    SomeIpMessage message(requestData);

    RpcHandler::ErrorCode const errorCode
        = _rpcHandler.handleRequest(message, clientEndpoint, destinationPort, serviceProto);
    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_OK, errorCode);

    return requestCallbackPtr;
}

TEST_F(RpcHandlerTest, testRequestDone)
{
    ::someip::NetworkChannel channel(_resourceMock, clientEndpoint, false);
    ::someip::CallDoneClosure* requestCallbackPtr = getRequestCallback();
    uint32_t const expectedResponseLength         = 16U;

    InSequence inSequence;
    EXPECT_CALL(
        _network, getRpcChannel(_providedService.description.port, clientEndpoint, serviceProto))
        .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(channel)));
    EXPECT_CALL(_resourceMock, getProto()).WillOnce(Return(serviceProto));
    EXPECT_CALL(_resourceMock, isOpen()).WillOnce(Return(true));
    EXPECT_CALL(_resourceMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(_resourceMock, send(expectedResponseLength)).Times(1U);
    (*requestCallbackPtr)(ServiceResultCode::RPC_POSITIVE_RESPONSE);
    uint8_t const returnCode = _resourceMock.buffer[expectedResponseLength - 1U];
    EXPECT_EQ(0U, returnCode);
    Mock::VerifyAndClearExpectations(&_resourceMock);
}

TEST_F(RpcHandlerTest, testRequestDoneWithRetCodeSet)
{
    ::someip::NetworkChannel channel(_resourceMock, clientEndpoint, false);
    ::someip::CallDoneClosure* requestCallbackPtr = getRequestCallback();
    uint32_t const expectedResponseLength         = 16U;
    uint8_t const operationResult                 = 1U;
    uint8_t const expectedEncodedReturnCode       = 0x1FU + operationResult;

    ::someip::ServiceHandler::setOperationResult(*requestCallbackPtr, operationResult);

    InSequence inSequence;
    EXPECT_CALL(
        _network, getRpcChannel(_providedService.description.port, clientEndpoint, serviceProto))
        .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(channel)))
        .RetiresOnSaturation();
    EXPECT_CALL(_resourceMock, getProto()).WillOnce(Return(serviceProto));
    EXPECT_CALL(_resourceMock, isOpen()).WillOnce(Return(true));
    EXPECT_CALL(_resourceMock, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(_resourceMock, send(expectedResponseLength)).Times(1);
    (*requestCallbackPtr)(ServiceResultCode::RPC_POSITIVE_RESPONSE);
    uint8_t const returnCode = _resourceMock.buffer[expectedResponseLength - 1U];
    EXPECT_EQ(expectedEncodedReturnCode, returnCode);
    Mock::VerifyAndClearExpectations(&_resourceMock);
}

/**
 * Make sure sendingRequest() with too small buffer is not successful.
 */
TEST_F(RpcHandlerTest, sendRequest_with_too_small_buffer)
{
    StrictMock<SomeIpSerializableMock> request;

    ::etl::array<uint8_t, 20U> buffer{};
    uint8_t bufferToSerialize[30];
    ::etl::span<uint8_t> dataToSerialize(bufferToSerialize);

    StrictMock<RpcChannelMock> channel;
    uint16_t localPort = 10U;
    EXPECT_CALL(channel, getLocalPort()).Times(1).WillOnce(Return(localPort));
    uint8_t proto = 0x11;
    EXPECT_CALL(channel, getProto()).Times(1).WillOnce(Return(proto));
    IPEndpoint remoteEndpoint(make_ip4(192U, 0U, 2U, 1U), 10U);
    EXPECT_CALL(channel, getRemoteIp()).Times(1).WillOnce(ReturnRef(remoteEndpoint));

    _resourceMock.setInputBuffer(buffer);
    _resourceMock.setOutputBuffer(buffer);

    ::etl::optional<NetworkChannel> optional(NetworkChannel(_resourceMock, remoteEndpoint));
    EXPECT_CALL(_network, getRpcChannel(localPort, remoteEndpoint, proto))
        .Times(1)
        .WillOnce(Return(optional));

    EXPECT_CALL(channel, getClientId()).Times(1).WillOnce(Return(0U));
    EXPECT_CALL(channel, getSessionId()).WillRepeatedly(Return(0U));
    EXPECT_CALL(request, serializeToArray(_))
        .Times(1)
        .WillOnce([&dataToSerialize](SomeIpSerializer& s) { s << dataToSerialize; });

    ServiceResultCode const rc
        = _rpcHandler.sendRequest(&request, 1U, 2U, 3U, true, channel, 1000U);

    EXPECT_EQ(ServiceResultCode::COULD_NOT_DELIVER, rc);
}

/**
 * Make sure sendingRequest() is not successful if serializeToArrayWithError occurs.
 */
TEST_F(RpcHandlerTest, sendRequest_with_serialize_error)
{
    StrictMock<SomeIpSerializableMock> request;

    ON_CALL(request, serializeToArray(_))
        .WillByDefault(Invoke(&request, &SomeIpSerializableMock::serializeToArrayWithError));

    ::etl::array<uint8_t, 10240U> buffer{};

    EXPECT_CALL(request, serializeToArray(_)).Times(1);

    StrictMock<RpcChannelMock> channel;
    uint16_t localPort = 10U;
    EXPECT_CALL(channel, getLocalPort()).Times(1).WillOnce(Return(localPort));
    uint16_t proto = 0x11;
    EXPECT_CALL(channel, getProto()).Times(1).WillOnce(Return(proto));
    IPEndpoint remoteEndpoint(make_ip4(192U, 0U, 2U, 1U), 10U);
    EXPECT_CALL(channel, getRemoteIp()).Times(1).WillOnce(ReturnRef(remoteEndpoint));

    _resourceMock.setInputBuffer(buffer);
    _resourceMock.setOutputBuffer(buffer);

    ::etl::optional<NetworkChannel> optional(NetworkChannel(_resourceMock, remoteEndpoint));
    EXPECT_CALL(_network, getRpcChannel(localPort, remoteEndpoint, proto))
        .Times(1)
        .WillOnce(Return(optional));

    EXPECT_CALL(channel, getClientId()).Times(1).WillOnce(Return(0U));
    EXPECT_CALL(channel, getSessionId()).WillRepeatedly(Return(0U));

    ServiceResultCode const rc
        = _rpcHandler.sendRequest(&request, 1U, 2U, 3U, true, channel, 1000U);

    EXPECT_EQ(ServiceResultCode::COULD_NOT_DELIVER, rc);
}

TEST_F(RpcHandlerTest, SendRequest_MessageSendError)
{
    StrictMock<SomeIpSerializableMock> request;

    ::etl::array<uint8_t, 10240U> buffer{};

    EXPECT_CALL(request, serializeToArray(_)).Times(1);

    StrictMock<RpcChannelMock> channel;

    EXPECT_CALL(channel, getServiceId()).Times(AnyNumber()).WillRepeatedly(Return(1U));
    EXPECT_CALL(channel, getClientId()).Times(AnyNumber()).WillRepeatedly(Return(0U));
    EXPECT_CALL(channel, setTimeout(_ethernetContext, 1000U));

    uint16_t localPort = 10U;
    EXPECT_CALL(channel, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(localPort));
    uint16_t proto = 0x11;
    EXPECT_CALL(channel, getProto()).Times(AnyNumber()).WillRepeatedly(Return(proto));
    IPEndpoint remoteEndpoint(make_ip4(192U, 0U, 2U, 1U), 10U);
    EXPECT_CALL(channel, getRemoteIp())
        .Times(AnyNumber())
        .WillRepeatedly(ReturnRef(remoteEndpoint));

    _resourceMock.setInputBuffer(buffer);
    _resourceMock.setOutputBuffer(buffer);

    EXPECT_CALL(_resourceMock, isOpen()).Times(AnyNumber()).WillRepeatedly(Return(true));
    EXPECT_CALL(_resourceMock, isConnected()).Times(AnyNumber()).WillRepeatedly(Return(false));
    EXPECT_CALL(_resourceMock, getProto()).Times(1).WillOnce(Return(proto));
    EXPECT_CALL(_resourceMock, send(_, _)).Times(1).WillOnce(Return(false));

    ::etl::optional<NetworkChannel> optional(NetworkChannel(_resourceMock, remoteEndpoint));
    EXPECT_CALL(_network, getRpcChannel(localPort, remoteEndpoint, proto))
        .Times(1)
        .WillOnce(Return(optional));

    EXPECT_EQ(0U, _rpcHandler.getNumRegisteredChannels());

    EXPECT_CALL(channel, getSessionId()).WillRepeatedly(Return(0U));
    // test the conditions
    EXPECT_CALL(channel, cancelTimeout());

    ServiceResultCode const rc
        = _rpcHandler.sendRequest(&request, 1U, 2U, 3U, true, channel, 1000U);

    EXPECT_EQ(ServiceResultCode::COULD_NOT_DELIVER, rc);
    EXPECT_EQ(0U, _rpcHandler.getNumRegisteredChannels());
}

TEST_F(RpcHandlerTest, HandleError_NoChannel)
{
    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(1U);
    message.setClientId(0U);
    message.setMethodId(2U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_METHOD);

    IPEndpoint endpoint(make_ip4(192U, 0U, 2U, 1U), 5U);

    EXPECT_EQ(RpcHandler::ErrorCode::RPC_HANDLER_ERROR, _rpcHandler.handleError(message, endpoint));
}

/**
 * Make sure handleNotification() is successful without an eventReceiver.
 */
TEST_F(RpcHandlerTest, handleNotification_without_eventReceiver)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 1U;
    major_version::type majorVersion = 2U;
    uint16_t localPort               = 20U;

    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(serviceId);
    message.setInterfaceVersion(majorVersion);
    message.setMethodId(2U);

    IPEndpoint endpoint(make_ip4(192U, 0U, 2U, 1U), 5U);

    EXPECT_CALL(
        _serviceRegistry,
        getInstanceId(serviceId, majorVersion, endpoint.getAddress(), endpoint.getPort(), true))
        .Times(1)
        .WillOnce(Return(instanceId));

    EXPECT_CALL(_serviceRegistry, isEventgroupPort(serviceId, instanceId, majorVersion, localPort))
        .Times(1)
        .WillOnce(Return(true));

    _rpcHandler.handleNotification(message, endpoint, localPort);
}

/**
 * Test handleNotification() with eventReceiver.
 */
TEST_F(RpcHandlerTest, handleNotification_with_eventReceiver)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 1U;
    uint16_t eventId                 = 2U;
    major_version::type majorVersion = 3U;
    uint16_t localPort               = 20U;

    ::etl::array<uint8_t, 80U> buffer = {0U};
    SomeIpMessage message(buffer);
    message.setServiceId(serviceId);
    message.setInterfaceVersion(majorVersion);
    message.setMethodId(eventId);
    message.setInterfaceVersion(3U);

    IPEndpoint endpoint(make_ip4(192U, 0U, 2U, 1U), 5U);

    EXPECT_CALL(
        _serviceRegistry,
        getInstanceId(serviceId, majorVersion, endpoint.getAddress(), endpoint.getPort(), true))
        .Times(1)
        .WillOnce(Return(instanceId));

    EXPECT_CALL(_serviceRegistry, isEventgroupPort(serviceId, instanceId, majorVersion, localPort))
        .Times(1)
        .WillOnce(Return(true));

    StrictMock<EventReceiverMock> eventReceiver;

    _rpcHandler.setEventReceiver(eventReceiver);

    EXPECT_CALL(eventReceiver, eventReceived(serviceId, eventId, _, majorVersion, _)).Times(1);
    _rpcHandler.handleNotification(message, endpoint, localPort);
}

/**
 * Make sure handleNotification() with wrong local port is not successful.
 */
TEST_F(RpcHandlerTest, handleNotification_with_wrong_local_port)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 1U;
    major_version::type majorVersion = 2U;
    uint16_t localPort               = 20U;

    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(serviceId);
    message.setInterfaceVersion(majorVersion);
    message.setMethodId(2U);

    IPEndpoint endpoint(make_ip4(192U, 0U, 2U, 1U), 5U);

    EXPECT_CALL(
        _serviceRegistry,
        getInstanceId(serviceId, majorVersion, endpoint.getAddress(), endpoint.getPort(), true))
        .Times(1)
        .WillOnce(Return(instanceId));

    EXPECT_CALL(
        _serviceRegistry, isEventgroupPort(serviceId, instanceId, majorVersion, localPort + 1U))
        .Times(1)
        .WillOnce(Return(false));

    _rpcHandler.handleNotification(message, endpoint, localPort + 1U);
}

class RpcHandlerErrorCodeTest : public ::testing::Test
{
public:
    RpcHandlerErrorCodeTest();
    ~RpcHandlerErrorCodeTest() override;

protected:
    StrictMock<NetworkResourceMock> _resource;
    StrictMock<NetworkMock> _network;
    StrictMock<TpTransceiverMock> _tpTransceiver;
    ::someip::declare::ServiceManager<1U> _serviceManager;
    StrictMock<ServiceRegistryMock> _serviceRegistry;
    ::etl::array<uint8_t, 200U> _resourceBuffer;

    uint16_t _localPort;
    uint8_t _proto;
    IPEndpoint remoteEndpoint;
    ::etl::optional<NetworkChannel> _optional;

    StrictMock<SomeIpSerializableMock> _req;
    StrictMock<RpcChannelMock> _rpcChannel;
    async::ContextType _ethernetContext{0U};
    RpcHandler _rpcHandler;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

RpcHandlerErrorCodeTest::RpcHandlerErrorCodeTest()
: _resource()
, _network()
, _resourceBuffer()
, _localPort(30001U)
, _proto(0x11)
, remoteEndpoint(make_ip4(192U, 0U, 2U, 1U), 30001U)
, _optional()
, _req()
, _rpcChannel()
, _rpcHandler(_network, _ethernetContext, _tpTransceiver, _serviceManager, _serviceRegistry)
, _asyncMock()
, _testContext(_ethernetContext)
{
    _resource.incRefCounter();
    _resource.setInputBuffer(_resourceBuffer);
    _resource.setOutputBuffer(_resourceBuffer);

    _testContext.handleAll();

    _optional = ::etl::optional<NetworkChannel>(NetworkChannel(_resource, remoteEndpoint));
    EXPECT_CALL(_network, getRpcChannel(_localPort, remoteEndpoint, _proto))
        .Times(1)
        .WillOnce(Return(_optional));
    EXPECT_CALL(_resource, send(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_resource, isOpen()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(_resource, isConnected()).Times(1).WillOnce(Return(true));

    EXPECT_CALL(_req, serializeToArray(_)).Times(1);

    EXPECT_CALL(_rpcChannel, getLocalPort()).Times(AtLeast(1)).WillRepeatedly(Return(_localPort));
    EXPECT_CALL(_rpcChannel, getProto()).Times(AtLeast(1)).WillRepeatedly(Return(_proto));
    EXPECT_CALL(_rpcChannel, getRemoteIp())
        .Times(AtLeast(1))
        .WillRepeatedly(ReturnRef(remoteEndpoint));
    EXPECT_CALL(_rpcChannel, getServiceId()).Times(AtLeast(1)).WillRepeatedly(Return(1U));
    EXPECT_CALL(_rpcChannel, getClientId()).Times(AtLeast(1)).WillRepeatedly(Return(0U));
    EXPECT_CALL(_rpcChannel, getSessionId()).WillRepeatedly(Return(0U));

    EXPECT_CALL(_resource, getProto()).Times(1).WillOnce(Return(_proto));
    EXPECT_CALL(_rpcChannel, setTimeout(_ethernetContext, 1000U));

    ServiceResultCode const rc
        = _rpcHandler.sendRequest(&_req, 1U, 2U, 3U, true, _rpcChannel, 1000U);

    EXPECT_EQ(ServiceResultCode::RPC_SENT_SUCCESSFULLY, rc);
}

RpcHandlerErrorCodeTest::~RpcHandlerErrorCodeTest() = default;

/**
 * Make sure handleError() handles SOMEIP_E_NOT_OK successfully.
 */
TEST_F(RpcHandlerErrorCodeTest, handleError_SOMEIP_E_NOT_OK)
{
    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(1U);
    message.setClientId(0U);
    message.setMethodId(2U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_NOT_OK);

    // test the condition
    EXPECT_CALL(_rpcChannel, responseReceived(ServiceResultCode::RPC_UNDEFINED_ERROR));

    EXPECT_CALL(_rpcChannel, cancelTimeout());
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK, _rpcHandler.handleError(message, remoteEndpoint));
}

/**
 * Make sure handleError() handles SOMEIP_E_WRONG_PROTOCOL_VERSION successfully.
 */
TEST_F(RpcHandlerErrorCodeTest, handleError_SOMEIP_E_WRONG_PROTOCOL_VERSION)
{
    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(1U);
    message.setClientId(0U);
    message.setMethodId(2U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_PROTOCOL_VERSION);

    // test the condition
    EXPECT_CALL(_rpcChannel, responseReceived(ServiceResultCode::RPC_WRONG_PROTOCOL_VERSION));

    EXPECT_CALL(_rpcChannel, cancelTimeout());
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK, _rpcHandler.handleError(message, remoteEndpoint));
}

/**
 * Make sure handleError() handles SOMEIP_E_WRONG_INTERFACE_VERSION successfully.
 */
TEST_F(RpcHandlerErrorCodeTest, handleError_SOMEIP_E_WRONG_INTERFACE_VERSION)
{
    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(1U);
    message.setClientId(0U);
    message.setMethodId(2U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_INTERFACE_VERSION);

    // test the condition
    EXPECT_CALL(_rpcChannel, responseReceived(ServiceResultCode::RPC_WRONG_INTERFACE_VERSION));

    EXPECT_CALL(_rpcChannel, cancelTimeout());
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK, _rpcHandler.handleError(message, remoteEndpoint));
}

/**
 * Make sure handleError() handles undefined error successfully.
 */
TEST_F(RpcHandlerErrorCodeTest, handleError_undefined_error)
{
    ::etl::array<uint8_t, 80U> buffer{};
    SomeIpMessage message(buffer);
    message.setServiceId(1U);
    message.setClientId(0U);
    message.setMethodId(2U);
    message.setReturnCode(static_cast<SomeIpMessage::ReturnCode>(50U));

    // test the condition
    EXPECT_CALL(_rpcChannel, responseReceived(ServiceResultCode::RPC_UNDEFINED_ERROR));

    EXPECT_CALL(_rpcChannel, cancelTimeout());
    EXPECT_EQ(
        RpcHandler::ErrorCode::RPC_HANDLER_OK, _rpcHandler.handleError(message, remoteEndpoint));
}

} // anonymous namespace
