/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/EventTransceiver.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/BufferedEventSender.h"
#include "someip/EventListenerMock.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializableBufferMock.h"
#include "someip/SubscriptionManager.h"
#include "someip/SubscriptionManagerMock.h"
#include "someip/TpTransceiverMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <etl/span.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;

struct EventTransceiverTest : Test
{
    EventTransceiverTest()
    : _network()
    , _networkResource()
    , _networkChannel()
    , _eventSender(_network, _ethernetContext, _tpTransceiver)
    , _subscriptionManager()
    , _eventTransceiver(_eventSender, _subscriptionManager, _subscriptionEndpoints)
    , _asyncMock()
    , _testContext(_ethernetContext)
    {
        _networkResource.incRefCounter();
        _testContext.handleAll();
    }

    StrictMock<NetworkMock> _network;
    StrictMock<NetworkResourceMock> _networkResource;
    ::etl::optional<NetworkChannel> _networkChannel;
    ::etl::optional<NetworkChannel> _networkChannel2;
    StrictMock<TpTransceiverMock> _tpTransceiver;
    async::ContextType _ethernetContext{0U};
    ::someip::declare::BufferedEventSender<8U> _eventSender;
    ::etl::vector<SubscriptionEndpoint, 2U> _subscriptionEndpoints;

    StrictMock<SubscriptionManagerMock> _subscriptionManager;
    EventTransceiver _eventTransceiver;
    StrictMock<SystemTimerMock> _stm;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

/**
 * Test EventTransceiver sending multicast event without event groups or subscribers.
 */
TEST_F(EventTransceiverTest, SendMulticastEvent_without_event_groups_or_subscribers)
{
    ::etl::span<uint8_t const> const buffer;
    ::etl::vector<uint16_t, 1U> ids;

    SomeIpSerializableBufferMock event(&buffer);
    _eventTransceiver.sendMulticastEvent(
        1U, 2U, 3U, 4U, ids, 5U, &event, 6U, 0x11, ::ip::IPEndpoint());
}

/**
 * Test EventTransceiver sending multicast event with one event group and no subscribers.
 */
TEST_F(EventTransceiverTest, SendMulticastEvent_with_one_event_group_and_no_subscribers)
{
    ::etl::span<uint8_t const> const buffer;
    ::etl::vector<uint16_t, 1U> ids;
    ids.push_back(0U);

    SubscriptionEndpointList subscriptions;
    EXPECT_CALL(_subscriptionManager, getSubscriptions(_))
        .Times(1)
        .WillOnce(Return(&subscriptions));

    SomeIpSerializableBufferMock event(&buffer);
    _eventTransceiver.sendMulticastEvent(
        1U, 2U, 3U, 4U, ids, 5U, &event, 6U, 0x11, ::ip::IPEndpoint());
}

/**
 * Test EventTransceiver sending multicast event with one event group and one subscriber.
 */
TEST_F(EventTransceiverTest, SendMulticastEvent_with_one_event_group_and_one_subscriber)
{
    ::etl::span<uint8_t const> const buffer;
    ::etl::vector<uint16_t, 1U> ids;
    ids.push_back(4U);

    uint16_t const port      = 20U;
    ::ip::IPAddress const ip = ::ip::make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint const endpoint(ip, port);

    SubscriptionEndpointList subscriptions;
    SubscriptionEndpoint subscription(ip, port);
    subscriptions.push_front(subscription);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(_subscriptionManager, getSubscriptions(_))
        .Times(1)
        .WillOnce(Return(&subscriptions));

    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(1);
    // EXPECT_CALL(_timeoutManager, set(_, _, _))
    //     .Times(1)
    //     .WillOnce(Return(::common::TimeoutManager2Mock::TIMEOUT_OK));

    SomeIpSerializableBufferMock event(&buffer);

    _networkChannel          = NetworkChannel(_networkResource, endpoint);
    uint16_t const localPort = 6U;
    EXPECT_CALL(_network, getRpcChannel(localPort, endpoint, proto::SD_L4_PROTO_UDP))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    _eventTransceiver.sendMulticastEvent(
        1U, 2U, 3U, 4U, ids, 5U, &event, localPort, proto::SD_L4_PROTO_UDP, endpoint);

    // free the subscription
    subscriptions.remove(subscription);
}

/**
 * Test EventTransceiver event received by one listener.
 */
TEST_F(EventTransceiverTest, event_received_by_one_listener)
{
    StrictMock<EventListenerMock> listener;
    _eventTransceiver.addEventListener(listener);

    uint8_t const buffer[] = {0x01U};
    SomeIpParser parser(buffer);

    EXPECT_CALL(listener, eventReceived(1U, 2U, 3U, 4U, Ref(parser))).Times(1);
    _eventTransceiver.eventReceived(1U, 2U, 3U, 4U, parser);

    _eventTransceiver.removeEventListener(listener);
}

/**
 * Test EventTransceiver event received by two listeners.
 */
TEST_F(EventTransceiverTest, event_received_by_two_listeners)
{
    StrictMock<EventListenerMock> listener1;
    StrictMock<EventListenerMock> listener2;

    _eventTransceiver.addEventListener(listener1);
    _eventTransceiver.addEventListener(listener2);

    uint8_t const buffer[] = {0x01U};
    SomeIpParser parser(buffer);

    EXPECT_CALL(listener1, eventReceived(1U, 2U, 3U, 4U, Ref(parser))).Times(1);
    EXPECT_CALL(listener2, eventReceived(1U, 2U, 3U, 4U, Ref(parser))).Times(1);
    _eventTransceiver.eventReceived(1U, 2U, 3U, 4U, parser);

    _eventTransceiver.removeEventListener(listener1);
    _eventTransceiver.removeEventListener(listener2);
}

/**
 * Make sure no event is received by listener after shutting down EventTransceiver.
 */
TEST_F(EventTransceiverTest, no_event_receiving_after_shutting_down)
{
    StrictMock<EventListenerMock> listener1;
    _eventTransceiver.addEventListener(listener1);

    _eventTransceiver.shutdown();

    EXPECT_CALL(listener1, eventReceived(_, _, _, _, _)).Times(0);
    uint8_t const buffer[] = {0x01U};
    SomeIpParser parser(buffer);
    _eventTransceiver.eventReceived(1U, 2U, 3U, 4U, parser);
}

/**
 * Test EventTransceiver send event.
 */
TEST_F(EventTransceiverTest, send_event)
{
    uint16_t const port      = 20U;
    ::ip::IPAddress const ip = ::ip::make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPEndpoint const endpoint(ip, port);

    ::etl::span<uint8_t const> const buffer;
    SomeIpSerializableBufferMock event(&buffer);

    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(1);
    // EXPECT_CALL(_timeoutManager, set(_, _, _))
    //     .Times(1)
    //     .WillOnce(Return(::common::TimeoutManager2Mock::TIMEOUT_OK));

    _networkChannel = NetworkChannel(_networkResource, endpoint);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    uint16_t const localPort = 5U;
    EXPECT_CALL(_network, getRpcChannel(localPort, endpoint, proto::SD_L4_PROTO_TCP))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    _eventTransceiver.sendEvent(
        1U, 2U, 3U, 4U, &event, localPort, proto::SD_L4_PROTO_TCP, endpoint);
}

/**
 * Test EventTransceiver sending event with several event group IDs.
 */
TEST_F(EventTransceiverTest, send_event_with_several_event_group_IDs)
{
    ::someip::declare::SubscriptionManager<10U> sub_manager;
    EventTransceiver dummy(_eventSender, sub_manager, _subscriptionEndpoints);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(1);
    // EXPECT_CALL(_timeoutManager, set(_, _, _))
    //     .Times(1)
    //     .WillOnce(Return(::common::TimeoutManager2Mock::TIMEOUT_OK));

    uint16_t const port       = 20U;
    ::ip::IPAddress const ip  = ::ip::make_ip4(192U, 0U, 2U, 0U);
    uint16_t const port2      = 21U;
    ::ip::IPAddress const ip2 = ::ip::make_ip4(192U, 0U, 2U, 1U);

    service_id::type serviceid       = 0x1234;
    major_version::type majorversion = 0x1;
    instance_id::type instanceid     = 0x1;
    ttl::type ttl                    = 0x5;

    sub_manager.addSubscription(serviceid, majorversion, instanceid, 0x1, ttl, ip, port);
    sub_manager.addSubscription(serviceid, majorversion, instanceid, 0x2, ttl + 2, ip, port);
    sub_manager.addSubscription(serviceid, majorversion, instanceid, 0x2, ttl + 2, ip2, port2);

    ::etl::span<uint8_t const> const buffer;
    ::etl::vector<uint16_t, 2U> ids;
    ids.push_back(0x1);
    ids.push_back(0x2);
    SomeIpSerializableBufferMock event(&buffer);

    ::ip::IPEndpoint const endpoint(ip, port);
    ::ip::IPEndpoint const endpoint2(ip2, port2);

    _networkChannel  = NetworkChannel(_networkResource, endpoint);
    _networkChannel2 = NetworkChannel(_networkResource, endpoint2);
    EXPECT_CALL(_network, getRpcChannel(6, endpoint, proto::SD_L4_PROTO_UDP))
        .Times(1)
        .WillOnce(Return(_networkChannel));
    EXPECT_CALL(_network, getRpcChannel(6, endpoint2, proto::SD_L4_PROTO_UDP))
        .Times(1)
        .WillOnce(Return(_networkChannel2));
    dummy.sendEvent(serviceid, majorversion, instanceid, 3U, ids, 5U, &event, 6U, 17U);
}

} // anonymous namespace
