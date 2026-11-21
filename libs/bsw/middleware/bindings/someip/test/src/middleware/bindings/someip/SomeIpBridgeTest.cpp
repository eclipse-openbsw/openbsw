/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/bindings/someip/SomeIpConnection.h"
#include "middleware/bindings/someip/SomeIpServiceEndpoint.h"

#include "middleware/bindings/someip/IServiceEndpoint.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/MemberBinding.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/bindings/someip/SomeIpBridgeConfig.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ProxyBase.h"
#include "someip/CallDoneClosureMock.h"
#include "someip/EventSenderMock.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcSenderMock.h"
#include "someip/SomeIpStackMock.h"

#include <etl/optional.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <gmock/gmock.h>

namespace
{

using namespace ::testing;
using ::middleware::bindings::someip::SomeIpBridgeConfig;
using ::middleware::core::ErrorState;
using ::middleware::core::HRESULT;
using ::middleware::core::Message;
using ::middleware::core::MessagePayloadBuilder;
using Kind          = ::middleware::bindings::someip::MemberBinding::Kind;
using ServiceStatus = ::someip::IServiceListener::ServiceStatus;

constexpr uint16_t SERVICE_ID          = 0x1234U;
constexpr uint16_t PROVIDED_INSTANCE   = 1U;
constexpr uint16_t CONSUMED_INSTANCE   = 2U;
constexpr uint16_t METHOD_ID           = 0x0001U;
constexpr uint16_t EVENT_ID            = 0x8001U;
constexpr uint16_t EVENTGROUP_ID       = 0x0002U;
constexpr uint8_t LOCAL_CLUSTER        = 2U;
constexpr uint8_t REMOTE_CLUSTER       = 3U;
constexpr uint8_t MAJOR_VERSION        = 1U;
constexpr uint32_t MINOR_VERSION       = 3U;
constexpr uint16_t PROVIDER_PORT       = 30555U;
constexpr uint16_t MULTICAST_PORT      = 30600U;
constexpr uint16_t PEER_PORT           = 30556U;
constexpr uint32_t REQUEST_TIMEOUT_MS  = 500U;
constexpr uint16_t PROXY_REQUEST_ID    = 7U;
constexpr uint8_t PROXY_ADDRESS_ID     = 5U;
constexpr uint8_t PROTO                = ::someip::proto::SD_L4_PROTO_UDP;
constexpr ::ip::IPAddress MULTICAST_IP = ::ip::make_ip4(239U, 1U, 2U, 3U);
constexpr ::ip::IPAddress PEER_IP      = ::ip::make_ip4(192U, 168U, 0U, 200U);

// One request in flight per direction, so the limits are easy to reach.
using Endpoint = ::middleware::bindings::someip::SomeIpServiceEndpoint<8U, 1U>;
using Slot     = ::middleware::bindings::someip::internal::PayloadSlot<uint16_t>;

SomeIpBridgeConfig makeConfig()
{
    SomeIpBridgeConfig config{};
    config.majorVersion          = MAJOR_VERSION;
    config.minorVersion          = MINOR_VERSION;
    config.instanceId            = PROVIDED_INSTANCE;
    config.providerPort          = PROVIDER_PORT;
    config.consumedInstanceId    = CONSUMED_INSTANCE;
    config.consumedEventGroupId  = EVENTGROUP_ID;
    config.proto                 = PROTO;
    config.requestTimeoutMs      = REQUEST_TIMEOUT_MS;
    config.eventMulticastAddress = MULTICAST_IP;
    config.eventMulticastPort    = MULTICAST_PORT;
    return config;
}

/** The bytes \p value puts on the wire. */
::etl::vector<uint8_t, 8U> wireBytesOf(::someip::ISomeIpSerializable const& value)
{
    uint8_t buf[8] = {0U};
    ::someip::SomeIpSerializer serializer(buf);
    value.serializeToArray(serializer);
    return ::etl::vector<uint8_t, 8U>(buf, buf + serializer.getCurrentPosition());
}

/** Fills \p target the way the stack does from a received big-endian uint16_t. */
void parseFromWire(::someip::ISomeIpSerializable& target, uint16_t const value)
{
    uint8_t const wire[] = {static_cast<uint8_t>(value >> 8U), static_cast<uint8_t>(value & 0xFFU)};
    ::someip::SomeIpParser parser(wire);
    target.parseFromArray(parser);
}

Message withPayload(Message msg, uint16_t const value)
{
    EXPECT_EQ(HRESULT::Ok, MessagePayloadBuilder::getInstance().allocate(value, msg));
    return msg;
}

/** Stands in for the SomeIpConnection: records what the endpoint hands to the middleware. */
class ConnectionStub : public ::middleware::bindings::someip::ISomeIpConnection
{
public:
    void add(::middleware::bindings::someip::internal::IServiceEndpoint& endpoint) override
    {
        added = &endpoint;
    }

    uint8_t getSourceClusterId() const override { return LOCAL_CLUSTER; }

    uint8_t getTargetClusterId() const override { return REMOTE_CLUSTER; }

    bool write(Message const&) const override { return false; }

    size_t registeredTransceiversCount(uint16_t const) const override { return 0U; }

    HRESULT dispatchMessage(Message const& msg) const override
    {
        dispatched.push_back(msg);
        return dispatchResult;
    }

    HRESULT subscribe(::middleware::core::ProxyBase&, uint16_t const) override
    {
        return HRESULT::Ok;
    }

    void unsubscribe(::middleware::core::ProxyBase&, uint16_t const) override {}

    HRESULT subscribe(::middleware::core::SkeletonBase&, uint16_t const) override
    {
        return HRESULT::Ok;
    }

    void unsubscribe(::middleware::core::SkeletonBase&, uint16_t const) override {}

    void registerTimeoutTransceiver(::middleware::core::ITimeoutHandler&) override {}

    void unregisterTimeoutTransceiver(::middleware::core::ITimeoutHandler&) override {}

    void updateTimeouts() override {}

    ::middleware::bindings::someip::internal::IServiceEndpoint* added{nullptr};
    mutable ::etl::vector<Message, 8U> dispatched;
    HRESULT dispatchResult{HRESULT::Ok};
};

class SomeIpServiceEndpointTest : public Test
{
public:
    SomeIpServiceEndpointTest()
    {
        _resource.incRefCounter();
        ON_CALL(_resource, isOpen()).WillByDefault(Return(true));
    }

protected:
    /** SD reports the consumed instance at the peer, and the endpoint opens its channel. */
    void findConsumedInstance()
    {
        ::ip::IPEndpoint const peer(PEER_IP, PEER_PORT);
        EXPECT_CALL(_network, openUdpChannel(PROVIDER_PORT, peer))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(_resource, peer))));
        _endpoint.consumer().serviceStatusChanged(
            consumedService(), ServiceStatus::SERVICE_AVAILABLE);
    }

    static ::someip::ServiceDescription consumedService()
    {
        ::someip::ServiceDescription service{};
        service.serviceId  = SERVICE_ID;
        service.instanceId = CONSUMED_INSTANCE;
        service.ipAddress  = PEER_IP;
        service.port       = PEER_PORT;
        return service;
    }

    /** Expects the request on the wire and hands back the channel its answer comes on. */
    void expectRequestOnTheWire(::someip::IRpcChannel*& channel, bool const isResponseExpected)
    {
        EXPECT_CALL(
            _rpcSender,
            sendRequest(
                NotNull(),
                SERVICE_ID,
                METHOD_ID,
                MAJOR_VERSION,
                isResponseExpected,
                _,
                isResponseExpected ? REQUEST_TIMEOUT_MS : 0U))
            .WillOnce(Invoke(
                [&channel](
                    ::someip::ISomeIpSerializable const* const request,
                    auto,
                    auto,
                    auto,
                    auto,
                    ::someip::IRpcChannel& rpcChannel,
                    auto)
                {
                    EXPECT_THAT(wireBytesOf(*request), ElementsAre(0x12U, 0x34U));
                    channel = &rpcChannel;
                    return ::someip::RPC_SENT_SUCCESSFULLY;
                }));
    }

    static Message proxyRequest()
    {
        return withPayload(
            Message::createRequest(
                SERVICE_ID,
                METHOD_ID,
                PROXY_REQUEST_ID,
                CONSUMED_INSTANCE,
                LOCAL_CLUSTER,
                REMOTE_CLUSTER,
                PROXY_ADDRESS_ID),
            0x1234U);
    }

    StrictMock<::someip::SomeIpStackMock> _stack;
    StrictMock<::someip::EventSenderMock> _eventSender;
    StrictMock<::someip::NetworkMock> _network;
    StrictMock<::someip::RpcSenderMock> _rpcSender;
    NiceMock<::someip::NetworkResourceMock> _resource;
    ConnectionStub _connection;
    Slot _requestSlot;
    Slot _responseSlot;
    Endpoint _endpoint{
        _connection, _stack, _eventSender, _network, _rpcSender, SERVICE_ID, makeConfig()};
};

TEST_F(SomeIpServiceEndpointTest, EndpointAddsItselfToItsConnection)
{
    EXPECT_EQ(&_endpoint, _connection.added);
}

TEST_F(SomeIpServiceEndpointTest, OfferServiceRegistersTheProvidedService)
{
    ::someip::ProvidedService const* offered = nullptr;
    EXPECT_CALL(_stack, registerProvidedService(_))
        .WillOnce(Invoke(
            [&offered](::someip::ProvidedService& service)
            {
                offered = &service;
                return true;
            }));

    _endpoint.offerService();

    ASSERT_NE(nullptr, offered);
    EXPECT_EQ(SERVICE_ID, offered->description.serviceId);
    EXPECT_EQ(PROVIDED_INSTANCE, offered->description.instanceId);
    EXPECT_EQ(MAJOR_VERSION, offered->description.majorVersion);
    EXPECT_EQ(MINOR_VERSION, offered->description.minorVersion);
    EXPECT_EQ(PROVIDER_PORT, offered->description.port);
    EXPECT_EQ(PROTO, offered->description.proto);
}

TEST_F(SomeIpServiceEndpointTest, MethodDetailFollowsTheBindingKind)
{
    _endpoint.bindMethod(1U, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    _endpoint.bindMethod(2U, Kind::FireAndForget, &_requestSlot, nullptr);
    _endpoint.bindMethod(3U, Kind::RequestOnly, &_requestSlot, nullptr);
    _endpoint.bindMethod(4U, Kind::ResponseOnly, nullptr, &_responseSlot);
    _endpoint.bindEvent(EVENT_ID, _requestSlot);

    ASSERT_NE(nullptr, _endpoint.provider().getMethodDetail(1U));
    EXPECT_EQ(
        ::someip::SomeIpMethodType::METHOD_REQUEST_RESPONSE,
        _endpoint.provider().getMethodDetail(1U)->methodType);
    ASSERT_NE(nullptr, _endpoint.provider().getMethodDetail(2U));
    EXPECT_EQ(
        ::someip::SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET,
        _endpoint.provider().getMethodDetail(2U)->callSemantic);
    ASSERT_NE(nullptr, _endpoint.provider().getMethodDetail(3U));
    EXPECT_EQ(
        ::someip::SomeIpMethodType::METHOD_REQUEST_ONLY,
        _endpoint.provider().getMethodDetail(3U)->methodType);
    ASSERT_NE(nullptr, _endpoint.provider().getMethodDetail(4U));
    EXPECT_EQ(
        ::someip::SomeIpMethodType::METHOD_RESPONSE_ONLY,
        _endpoint.provider().getMethodDetail(4U)->methodType);

    // Events never arrive as a method call, and nothing is known of an unbound id.
    EXPECT_EQ(nullptr, _endpoint.provider().getMethodDetail(EVENT_ID));
    EXPECT_EQ(nullptr, _endpoint.provider().getMethodDetail(0x0099U));

    EXPECT_EQ(&_requestSlot.serializable(), _endpoint.provider().getRequest(1U));
    EXPECT_EQ(&_responseSlot.serializable(), _endpoint.provider().getResponse(1U));
    EXPECT_EQ(nullptr, _endpoint.provider().getResponse(2U));
    EXPECT_EQ(nullptr, _endpoint.provider().getRequest(4U));
}

TEST_F(SomeIpServiceEndpointTest, MethodCallReachesTheSkeletonAndItsResponseCompletesTheCall)
{
    _endpoint.bindMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    StrictMock<::someip::CallDoneClosureMock> done;

    parseFromWire(*_endpoint.provider().getRequest(METHOD_ID), 0x1234U);
    _endpoint.provider().dispatchMethod(
        METHOD_ID,
        _endpoint.provider().getRequest(METHOD_ID),
        _endpoint.provider().getResponse(METHOD_ID),
        done);

    ASSERT_EQ(1U, _connection.dispatched.size());
    Message const request = _connection.dispatched[0];
    EXPECT_TRUE(request.isRequest());
    EXPECT_EQ(SERVICE_ID, request.getHeader().serviceId);
    EXPECT_EQ(METHOD_ID, request.getHeader().memberId);
    EXPECT_EQ(PROVIDED_INSTANCE, request.getHeader().serviceInstanceId);
    // Stamped from the receiver's side: from the remote cluster to the local one.
    EXPECT_EQ(REMOTE_CLUSTER, request.getHeader().srcClusterId);
    EXPECT_EQ(LOCAL_CLUSTER, request.getHeader().tgtClusterId);
    EXPECT_EQ(0x1234U, MessagePayloadBuilder::readPayload<uint16_t>(request));

    // The SOME/IP call completes only with the skeleton's response, carrying its value.
    EXPECT_CALL(done, invoke(::someip::RPC_POSITIVE_RESPONSE))
        .WillOnce(Invoke(
            [this](::someip::ServiceResultCode const result)
            {
                EXPECT_THAT(
                    wireBytesOf(*_endpoint.provider().getResponse(METHOD_ID)),
                    ElementsAre(0xABU, 0xCDU));
                return result;
            }));
    EXPECT_TRUE(_endpoint.send(withPayload(
        Message::createResponse(
            SERVICE_ID,
            METHOD_ID,
            request.getHeader().requestId,
            PROVIDED_INSTANCE,
            LOCAL_CLUSTER,
            REMOTE_CLUSTER,
            request.getHeader().addressId),
        0xABCDU)));
}

TEST_F(SomeIpServiceEndpointTest, FireAndForgetIsDispatchedAndCompletedAtOnce)
{
    _endpoint.bindMethod(METHOD_ID, Kind::FireAndForget, &_requestSlot, nullptr);
    StrictMock<::someip::CallDoneClosureMock> done;
    EXPECT_CALL(done, invoke(::someip::RPC_SENT_SUCCESSFULLY_NO_RESPONSE_EXPECTED))
        .WillOnce(ReturnArg<0>());

    parseFromWire(*_endpoint.provider().getRequest(METHOD_ID), 0x0042U);
    _endpoint.provider().dispatchMethod(
        METHOD_ID, _endpoint.provider().getRequest(METHOD_ID), nullptr, done);

    ASSERT_EQ(1U, _connection.dispatched.size());
    EXPECT_TRUE(_connection.dispatched[0].isFireAndForgetRequest());
    EXPECT_EQ(0x0042U, MessagePayloadBuilder::readPayload<uint16_t>(_connection.dispatched[0]));
}

TEST_F(SomeIpServiceEndpointTest, UnboundMethodIsNotAvailable)
{
    StrictMock<::someip::CallDoneClosureMock> done;
    EXPECT_CALL(done, invoke(::someip::RPC_METHOD_NOT_AVAILABLE)).WillOnce(ReturnArg<0>());

    _endpoint.provider().dispatchMethod(METHOD_ID, nullptr, nullptr, done);

    EXPECT_TRUE(_connection.dispatched.empty());
}

TEST_F(SomeIpServiceEndpointTest, CallTheSkeletonCannotTakeFailsAndFreesItsRequestId)
{
    _endpoint.bindMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    _connection.dispatchResult = HRESULT::ServiceNotFound;
    StrictMock<::someip::CallDoneClosureMock> refused;
    EXPECT_CALL(refused, invoke(::someip::RPC_UNDEFINED_ERROR)).WillOnce(ReturnArg<0>());

    _endpoint.provider().dispatchMethod(
        METHOD_ID,
        _endpoint.provider().getRequest(METHOD_ID),
        _endpoint.provider().getResponse(METHOD_ID),
        refused);

    // The only request id was freed, so the next call is dispatched again.
    _connection.dispatchResult = HRESULT::Ok;
    StrictMock<::someip::CallDoneClosureMock> next;
    _endpoint.provider().dispatchMethod(
        METHOD_ID,
        _endpoint.provider().getRequest(METHOD_ID),
        _endpoint.provider().getResponse(METHOD_ID),
        next);
    EXPECT_EQ(2U, _connection.dispatched.size());
}

TEST_F(SomeIpServiceEndpointTest, CallsBeyondThePendingLimitAreRefused)
{
    _endpoint.bindMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    StrictMock<::someip::CallDoneClosureMock> pending;
    StrictMock<::someip::CallDoneClosureMock> refused;
    EXPECT_CALL(refused, invoke(::someip::RPC_UNDEFINED_ERROR)).WillOnce(ReturnArg<0>());

    _endpoint.provider().dispatchMethod(
        METHOD_ID,
        _endpoint.provider().getRequest(METHOD_ID),
        _endpoint.provider().getResponse(METHOD_ID),
        pending);
    _endpoint.provider().dispatchMethod(
        METHOD_ID,
        _endpoint.provider().getRequest(METHOD_ID),
        _endpoint.provider().getResponse(METHOD_ID),
        refused);

    EXPECT_EQ(1U, _connection.dispatched.size());
}

TEST_F(SomeIpServiceEndpointTest, ResponseWithoutAPendingCallIsNotSent)
{
    _endpoint.bindMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);

    EXPECT_FALSE(_endpoint.send(withPayload(
        Message::createResponse(
            SERVICE_ID, METHOD_ID, 0U, PROVIDED_INSTANCE, LOCAL_CLUSTER, REMOTE_CLUSTER, 0U),
        0xABCDU)));
}

TEST_F(SomeIpServiceEndpointTest, EventIsSentToItsEventgroupSubscribers)
{
    uint16_t const eventGroups[] = {EVENTGROUP_ID};
    _endpoint.bindEvent(EVENT_ID, _requestSlot, eventGroups);

    EXPECT_CALL(
        _eventSender,
        sendEvent(
            SERVICE_ID,
            MAJOR_VERSION,
            PROVIDED_INSTANCE,
            EVENT_ID,
            _,
            0U,
            NotNull(),
            PROVIDER_PORT,
            PROTO,
            _))
        .WillOnce(Invoke(
            [](auto,
               auto,
               auto,
               auto,
               ::etl::span<uint16_t const> const groups,
               auto,
               ::someip::ISomeIpSerializable const* const payload,
               auto,
               auto,
               auto)
            {
                EXPECT_EQ(1U, groups.size());
                EXPECT_EQ(EVENTGROUP_ID, groups[0]);
                EXPECT_THAT(wireBytesOf(*payload), ElementsAre(0x01U, 0x02U));
                return ::someip::IEventSender::ErrorCode::EVENT_SEND_OK;
            }));

    EXPECT_TRUE(_endpoint.send(withPayload(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER), 0x0102U)));
}

TEST_F(SomeIpServiceEndpointTest, MulticastEventIsSentToTheMulticastGroup)
{
    uint16_t const eventGroups[] = {EVENTGROUP_ID};
    _endpoint.bindEvent(EVENT_ID, _requestSlot, eventGroups, /*isMulticast=*/true);

    EXPECT_CALL(
        _eventSender,
        sendMulticastEvent_0_10(
            SERVICE_ID,
            MAJOR_VERSION,
            PROVIDED_INSTANCE,
            EVENT_ID,
            _,
            0U,
            NotNull(),
            PROVIDER_PORT,
            PROTO,
            ::ip::IPEndpoint(MULTICAST_IP, MULTICAST_PORT)));
    EXPECT_CALL(_eventSender, sendMulticastEvent_11(_));

    EXPECT_TRUE(_endpoint.send(withPayload(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER), 0x0102U)));
}

TEST_F(SomeIpServiceEndpointTest, MulticastEventWithoutAGroupStaysUnicast)
{
    SomeIpBridgeConfig config = makeConfig();
    config.eventMulticastPort = 0U;
    Endpoint unicastOnly(
        _connection, _stack, _eventSender, _network, _rpcSender, SERVICE_ID, config);
    unicastOnly.bindEvent(EVENT_ID, _requestSlot, {}, /*isMulticast=*/true);

    EXPECT_CALL(
        _eventSender,
        sendEvent(SERVICE_ID, MAJOR_VERSION, PROVIDED_INSTANCE, EVENT_ID, _, 0U, _, _, _, _))
        .WillOnce(Return(::someip::IEventSender::ErrorCode::EVENT_SEND_OK));

    EXPECT_TRUE(unicastOnly.send(withPayload(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER), 0x0102U)));
}

TEST_F(SomeIpServiceEndpointTest, EventTheStackCannotSendIsReported)
{
    _endpoint.bindEvent(EVENT_ID, _requestSlot);
    EXPECT_CALL(_eventSender, sendEvent(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Return(::someip::IEventSender::ErrorCode::EVENT_SEND_ERROR));

    EXPECT_FALSE(_endpoint.send(withPayload(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER), 0x0102U)));
}

TEST_F(SomeIpServiceEndpointTest, MessageOfAnUnboundMemberIsNotSent)
{
    EXPECT_FALSE(_endpoint.send(withPayload(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER), 0x0102U)));
}

TEST_F(SomeIpServiceEndpointTest, ProxyRequestFailsUntilTheConsumedInstanceIsFound)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);

    EXPECT_FALSE(_endpoint.isConsumedInstanceAvailable());
    EXPECT_FALSE(_endpoint.send(proxyRequest()));
}

TEST_F(SomeIpServiceEndpointTest, ProxyRequestGoesToTheFoundInstanceAndItsResponseComesBack)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    findConsumedInstance();
    EXPECT_TRUE(_endpoint.isConsumedInstanceAvailable());

    ::someip::IRpcChannel* channel = nullptr;
    expectRequestOnTheWire(channel, /*isResponseExpected=*/true);
    EXPECT_TRUE(_endpoint.send(proxyRequest()));
    ASSERT_NE(nullptr, channel);

    // The stack parses the peer's answer into the channel's response, then completes the call.
    parseFromWire(*channel->getResponse(), 0xBEEFU);
    channel->responseReceived(::someip::RPC_POSITIVE_RESPONSE);

    ASSERT_EQ(1U, _connection.dispatched.size());
    Message const response = _connection.dispatched[0];
    EXPECT_TRUE(response.isResponse());
    EXPECT_EQ(METHOD_ID, response.getHeader().memberId);
    EXPECT_EQ(PROXY_REQUEST_ID, response.getHeader().requestId);
    EXPECT_EQ(CONSUMED_INSTANCE, response.getHeader().serviceInstanceId);
    // Addressed back the way the request came.
    EXPECT_EQ(REMOTE_CLUSTER, response.getHeader().srcClusterId);
    EXPECT_EQ(LOCAL_CLUSTER, response.getHeader().tgtClusterId);
    EXPECT_EQ(PROXY_ADDRESS_ID, response.getHeader().addressId);
    EXPECT_EQ(0xBEEFU, MessagePayloadBuilder::readPayload<uint16_t>(response));
}

TEST_F(SomeIpServiceEndpointTest, CallErrorReachesTheProxyAsAnErrorResponse)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    findConsumedInstance();

    struct Case
    {
        ::someip::ServiceResultCode result;
        ErrorState error;
    };

    Case const cases[] = {
        {::someip::RPC_TIMEOUT, ErrorState::ServiceNotFound},
        {::someip::BUSY_ERROR, ErrorState::ServiceBusy},
        {::someip::RPC_INVALID_PAYLOAD, ErrorState::DeserializationError},
    };
    for (Case const& c : cases)
    {
        ::someip::IRpcChannel* channel = nullptr;
        expectRequestOnTheWire(channel, /*isResponseExpected=*/true);
        EXPECT_TRUE(_endpoint.send(proxyRequest()));
        ASSERT_NE(nullptr, channel);

        channel->responseReceived(c.result);

        ASSERT_FALSE(_connection.dispatched.empty());
        EXPECT_TRUE(_connection.dispatched.back().isError());
        EXPECT_EQ(c.error, _connection.dispatched.back().getErrorState());
        EXPECT_EQ(PROXY_REQUEST_ID, _connection.dispatched.back().getHeader().requestId);
    }
}

TEST_F(SomeIpServiceEndpointTest, BusyChannelMakesTheNextProxyRequestFail)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    findConsumedInstance();

    ::someip::IRpcChannel* channel = nullptr;
    expectRequestOnTheWire(channel, /*isResponseExpected=*/true);
    EXPECT_TRUE(_endpoint.send(proxyRequest()));

    EXPECT_FALSE(_endpoint.send(proxyRequest()));
}

TEST_F(SomeIpServiceEndpointTest, LostInstanceMakesProxyRequestsFailAgain)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::RequestResponse, &_requestSlot, &_responseSlot);
    findConsumedInstance();

    _endpoint.consumer().serviceStatusChanged(
        consumedService(), ServiceStatus::SERVICE_UNAVAILABLE);

    EXPECT_FALSE(_endpoint.isConsumedInstanceAvailable());
    EXPECT_FALSE(_endpoint.send(proxyRequest()));
}

TEST_F(SomeIpServiceEndpointTest, ProxyFireAndForgetGoesOutWithoutWaitingForAnAnswer)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::FireAndForget, &_requestSlot, nullptr);
    findConsumedInstance();

    ::someip::IRpcChannel* channel = nullptr;
    expectRequestOnTheWire(channel, /*isResponseExpected=*/false);

    EXPECT_TRUE(_endpoint.send(withPayload(
        Message::createFireAndForgetRequest(
            SERVICE_ID, METHOD_ID, CONSUMED_INSTANCE, LOCAL_CLUSTER, REMOTE_CLUSTER),
        0x1234U)));
}

TEST_F(SomeIpServiceEndpointTest, StatusOfAnInstanceItDoesNotConsumeIsIgnored)
{
    ::someip::ServiceDescription provided = consumedService();
    provided.instanceId                   = PROVIDED_INSTANCE;

    _endpoint.consumer().serviceStatusChanged(provided, ServiceStatus::SERVICE_AVAILABLE);

    EXPECT_FALSE(_endpoint.isConsumedInstanceAvailable());
}

TEST_F(SomeIpServiceEndpointTest, ConsumedMethodDetailFollowsTheBindingKind)
{
    _endpoint.bindConsumedMethod(METHOD_ID, Kind::FireAndForget, &_requestSlot, nullptr);
    _endpoint.bindConsumedEvent(EVENT_ID, _requestSlot);
    ::someip::IServiceListener const& listener = _endpoint.consumer();

    ASSERT_NE(nullptr, listener.getMethodDetail(METHOD_ID));
    EXPECT_EQ(
        ::someip::SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET,
        listener.getMethodDetail(METHOD_ID)->callSemantic);
    // Events never arrive as a method call.
    EXPECT_EQ(nullptr, listener.getMethodDetail(EVENT_ID));
}

TEST_F(SomeIpServiceEndpointTest, ConsumedEventIsDeliveredToTheProxy)
{
    _endpoint.bindConsumedEvent(EVENT_ID, _requestSlot);
    uint8_t const wire[] = {0x01U, 0x02U};
    ::someip::SomeIpParser parser(wire);

    _endpoint.consumer().eventReceived(
        SERVICE_ID, EVENT_ID, CONSUMED_INSTANCE, MAJOR_VERSION, parser);

    ASSERT_EQ(1U, _connection.dispatched.size());
    Message const event = _connection.dispatched[0];
    EXPECT_TRUE(event.isEvent());
    EXPECT_EQ(EVENT_ID, event.getHeader().memberId);
    EXPECT_EQ(CONSUMED_INSTANCE, event.getHeader().serviceInstanceId);
    EXPECT_EQ(LOCAL_CLUSTER, event.getHeader().srcClusterId);
    EXPECT_EQ(0x0102U, MessagePayloadBuilder::readPayload<uint16_t>(event));
}

TEST_F(SomeIpServiceEndpointTest, EventsItDoesNotConsumeAreIgnored)
{
    _endpoint.bindConsumedEvent(EVENT_ID, _requestSlot);
    uint8_t const wire[] = {0x01U, 0x02U};

    ::someip::SomeIpParser otherService(wire);
    _endpoint.consumer().eventReceived(
        0x9999U, EVENT_ID, CONSUMED_INSTANCE, MAJOR_VERSION, otherService);
    ::someip::SomeIpParser otherInstance(wire);
    _endpoint.consumer().eventReceived(
        SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, MAJOR_VERSION, otherInstance);
    ::someip::SomeIpParser unboundEvent(wire);
    _endpoint.consumer().eventReceived(
        SERVICE_ID, 0x8099U, CONSUMED_INSTANCE, MAJOR_VERSION, unboundEvent);

    // A uint16_t payload cut short is malformed.
    uint8_t const truncated[] = {0x01U};
    ::someip::SomeIpParser malformed(truncated);
    _endpoint.consumer().eventReceived(
        SERVICE_ID, EVENT_ID, CONSUMED_INSTANCE, MAJOR_VERSION, malformed);

    EXPECT_TRUE(_connection.dispatched.empty());
}

TEST_F(SomeIpServiceEndpointTest, QueryingFindsAndSubscribesToTheConsumedInstance)
{
    ::middleware::bindings::someip::internal::IServiceEndpoint& endpoint = _endpoint;
    ::someip::ServiceQuery const* find                                   = nullptr;
    ::someip::ServiceQuery const* subscribe                              = nullptr;
    EXPECT_CALL(_stack, registerServiceQuery(_))
        .WillOnce(Invoke(
            [&find](::someip::ServiceQuery& query)
            {
                find = &query;
                return true;
            }))
        .WillOnce(Invoke(
            [&subscribe](::someip::ServiceQuery& query)
            {
                subscribe = &query;
                return true;
            }));
    EXPECT_CALL(_stack, addEventListener(Ref(_endpoint.consumer())));

    EXPECT_TRUE(endpoint.startQuerying());
    // Starting again while querying changes nothing.
    EXPECT_TRUE(endpoint.startQuerying());

    ASSERT_NE(nullptr, find);
    ASSERT_NE(nullptr, subscribe);
    EXPECT_EQ(SERVICE_ID, find->description.serviceId);
    EXPECT_EQ(CONSUMED_INSTANCE, find->description.instanceId);
    EXPECT_EQ(::someip::eventgroup_id::ALL, find->description.eventGroup);
    EXPECT_EQ(EVENTGROUP_ID, subscribe->description.eventGroup);
    EXPECT_EQ(PROVIDER_PORT, subscribe->description.port);

    EXPECT_CALL(_stack, unregisterServiceQuery(_)).Times(2);
    EXPECT_CALL(_stack, removeEventListener(Ref(_endpoint.consumer())));
    endpoint.stopQuerying();
}

TEST_F(SomeIpServiceEndpointTest, QueryingThatCannotBeRegisteredIsUndone)
{
    ::middleware::bindings::someip::internal::IServiceEndpoint& endpoint = _endpoint;
    EXPECT_CALL(_stack, registerServiceQuery(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(_stack, unregisterServiceQuery(_));

    EXPECT_FALSE(endpoint.startQuerying());
}

class ServiceEndpointMock : public ::middleware::bindings::someip::internal::IServiceEndpoint
{
public:
    MOCK_METHOD(uint16_t, getServiceId, (), (const, override));
    MOCK_METHOD(uint16_t, getProvidedInstanceId, (), (const, override));
    MOCK_METHOD(uint16_t, getConsumedInstanceId, (), (const, override));
    MOCK_METHOD(bool, send, (Message const&), (override));
    MOCK_METHOD(bool, startQuerying, (), (override));
    MOCK_METHOD(void, stopQuerying, (), (override));
};

class TestProxy : public ::middleware::core::ProxyBase
{
public:
    uint16_t getServiceId() const override { return SERVICE_ID; }

    HRESULT onNewMessageReceived(Message const&) override { return HRESULT::Ok; }
};

class SomeIpConnectionTest : public Test
{
public:
    SomeIpConnectionTest()
    {
        ON_CALL(_endpoint, getServiceId()).WillByDefault(Return(SERVICE_ID));
        ON_CALL(_endpoint, getProvidedInstanceId()).WillByDefault(Return(PROVIDED_INSTANCE));
        ON_CALL(_endpoint, getConsumedInstanceId()).WillByDefault(Return(CONSUMED_INSTANCE));
        _connection.add(_endpoint);
    }

protected:
    NiceMock<ServiceEndpointMock> _endpoint;
    ::middleware::bindings::someip::SomeIpConnection<2U, 2U> _connection{
        LOCAL_CLUSTER, REMOTE_CLUSTER};
};

TEST_F(SomeIpConnectionTest, ReportsItsClusters)
{
    EXPECT_EQ(LOCAL_CLUSTER, _connection.getSourceClusterId());
    EXPECT_EQ(REMOTE_CLUSTER, _connection.getTargetClusterId());
}

TEST_F(SomeIpConnectionTest, WriteRoutesAMessageToTheEndpointOfItsService)
{
    EXPECT_CALL(_endpoint, send(_)).Times(2).WillRepeatedly(Return(true));

    // Both the provided and the consumed instance belong to the endpoint.
    EXPECT_TRUE(_connection.write(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER)));
    EXPECT_TRUE(_connection.write(Message::createRequest(
        SERVICE_ID, METHOD_ID, 1U, CONSUMED_INSTANCE, LOCAL_CLUSTER, REMOTE_CLUSTER, 0U)));

    EXPECT_FALSE(_connection.write(
        Message::createEvent(0x9999U, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER)));
}

TEST_F(SomeIpConnectionTest, WriteTheEndpointRefusesIsReported)
{
    EXPECT_CALL(_endpoint, send(_)).WillOnce(Return(false));

    EXPECT_FALSE(_connection.write(
        Message::createEvent(SERVICE_ID, EVENT_ID, PROVIDED_INSTANCE, LOCAL_CLUSTER)));
}

TEST_F(SomeIpConnectionTest, ProxySubscriptionStartsAndStopsQuerying)
{
    TestProxy proxy;
    EXPECT_CALL(_endpoint, startQuerying()).WillOnce(Return(true));

    EXPECT_EQ(HRESULT::Ok, _connection.subscribe(proxy, CONSUMED_INSTANCE));
    EXPECT_EQ(1U, _connection.registeredTransceiversCount(SERVICE_ID));

    EXPECT_CALL(_endpoint, stopQuerying());
    _connection.unsubscribe(proxy, SERVICE_ID);
    EXPECT_EQ(0U, _connection.registeredTransceiversCount(SERVICE_ID));
}

TEST_F(SomeIpConnectionTest, ProxyOfAnInstanceNoEndpointConsumesIsRefused)
{
    TestProxy proxy;
    EXPECT_CALL(_endpoint, startQuerying()).Times(0);

    EXPECT_EQ(HRESULT::InstanceNotFound, _connection.subscribe(proxy, PROVIDED_INSTANCE));
}

TEST_F(SomeIpConnectionTest, SubscriptionWhoseQueryingFailsIsUndone)
{
    TestProxy proxy;
    EXPECT_CALL(_endpoint, startQuerying()).WillOnce(Return(false));

    EXPECT_EQ(
        HRESULT::TransceiverInitializationFailed, _connection.subscribe(proxy, CONSUMED_INSTANCE));
    EXPECT_EQ(0U, _connection.registeredTransceiversCount(SERVICE_ID));
}

} // namespace
