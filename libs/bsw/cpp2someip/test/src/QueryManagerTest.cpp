/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/QueryManager.h"

#include "TestConstants.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcReceiverMock.h"
#include "someip/ServiceAnnouncerMock.h"
#include "someip/ServiceListenerMock.h"
#include "someip/ServiceQuery.h"
#include "someip/SomeIpConstants.h"
#include "someip/TcpClientChannelValidator.h"
#include "someip/init.h"

#include <ip/IPAddress.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;
using namespace ::test;

struct QueryManagerTest : Test
{
    QueryManagerTest()
    : _networkMock()
    , _validator(_networkMock, false)
    , _queryManager(_validator)
    , _serviceAnnouncer()
    , _rpcReceiver()
    {
        _queryManager.wire(&_serviceAnnouncer, &_rpcReceiver);
    }

    static uint8_t const MAX_SERVICES = 2U;

    StrictMock<NetworkMock> _networkMock;
    TcpClientChannelValidator _validator;
    declare::QueryManager<MAX_SERVICES> _queryManager;
    StrictMock<ServiceAnnouncerMock> _serviceAnnouncer;
    StrictMock<RpcReceiverMock> _rpcReceiver;
};

/**
 * Test ServiceQuery registration and unregistration process.
 * Make sure ServiceQuery enters QUERY_INITIAL_WAIT_PHASE on registration,
 * double registration is prevented and no further queries can be added if QueryList already full.
 * On unregistration make sure query enters QUERY_IDLE_PHASE, double unregister is prevented
 * and a subscribed query with event group is unsubscribed.
 */
TEST_F(QueryManagerTest, register_unregister_ServiceQuery)
{
    auto query1        = make<ServiceQuery>();
    auto query2        = make<ServiceQuery>();
    query1.timestamp   = 1U;
    query2.timestamp   = 2U;
    query2.description = query1.description;
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_IDLE_PHASE, query1.state);

    // register
    EXPECT_TRUE(_queryManager.registerQuery(query1));
    EXPECT_TRUE(_queryManager.hasQuery(query1));
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query1.state);

    EXPECT_FALSE(_queryManager.hasQuery(query2));
    EXPECT_TRUE(_queryManager.hasServiceDescription(query2.description));

    // double register
    query1.state = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;
    EXPECT_FALSE(_queryManager.registerQuery(query1)); // no effect
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query1.state);

    // fill to the maximum
    ServiceQuery query3 = query1;
    query3.description.serviceId += 1U;
    EXPECT_FALSE(_queryManager.hasQuery(query3));
    EXPECT_FALSE(_queryManager.hasServiceDescription(query3.description));
    EXPECT_TRUE(_queryManager.registerQuery(query3)); // last entry to be accepted

    ServiceQuery query4 = query1;
    query4.description.serviceId += 2U;
    EXPECT_FALSE(_queryManager.hasQuery(query4));
    EXPECT_FALSE(_queryManager.hasServiceDescription(query4.description));
    EXPECT_FALSE(_queryManager.registerQuery(query4)); // already full, no effect

    // unregister
    EXPECT_TRUE(_queryManager.unregisterQuery(query1));
    EXPECT_FALSE(_queryManager.hasQuery(query1));
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_IDLE_PHASE, query1.state);

    // double unregister
    EXPECT_FALSE(_queryManager.unregisterQuery(query1)); // no effect

    // unsubscribe if query has an event group and has been subscribed
    query1.description.eventGroup = 40000U;
    EXPECT_TRUE(_queryManager.registerQuery(query1));
    EXPECT_TRUE(_queryManager.hasQuery(query1));
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query1.state);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query1.subscriptionState);
    query1.subscriptionState       = ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK;
    auto const sdIpAddress         = make_ip4(224U, 1U, 255U, 255U);
    query1.serviceDiscoveryAddress = sdIpAddress;
    ServiceDescription actualServiceDescription;
    EXPECT_CALL(_serviceAnnouncer, unsubscribe(_, sdIpAddress))
        .Times(1)
        .WillOnce(SaveArg<0>(&actualServiceDescription));
    EXPECT_TRUE(_queryManager.unregisterQuery(query1));
    EXPECT_FALSE(_queryManager.hasQuery(query1));
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_IDLE_PHASE, query1.state);
    EXPECT_TRUE(matches(query1.description, actualServiceDescription));
}

/**
 * Make sure registered ServiceQuery is initialized when QueryManager starts.
 */
TEST_F(QueryManagerTest, test_ServiceQuery_lifecycle)
{
    auto query = make<ServiceQuery>();
    EXPECT_TRUE(_queryManager.registerQuery(query));
    query.state = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;

    _queryManager.start();
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query.state);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, updateServiceQuery)
{
    StrictMock<ServiceListenerMock> listener;

    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = ::someip::instance_id::ANY;
    query.listener                 = &listener;

    EXPECT_TRUE(_queryManager.registerQuery(query));

    ServiceDescription service(query.description);
    service.instanceId = 1U;

    EXPECT_CALL(
        listener,
        serviceStatusChanged(Ref(service), IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_AVAILABLE);
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query.state);

    EXPECT_CALL(
        listener,
        serviceStatusChanged(Ref(service), IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE))
        .Times(1);
    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query.state);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, updateEventGroupQuery)
{
    StrictMock<ServiceListenerMock> listener;

    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = 1U;
    query.description.eventGroup   = 1U;
    query.listener                 = &listener;

    EXPECT_TRUE(_queryManager.registerQuery(query));

    ServiceDescription service(query.description);
    service.instanceId = 1U;
    service.eventGroup = ::someip::eventgroup_id::ALL;

    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_AVAILABLE);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query.subscriptionState);

    query.subscriptionState = ServiceQuery::SubscriptionState::STATE_ACK_RECEIVED;

    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query.subscriptionState);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, updateTime)
{
    auto query1 = make<ServiceQuery>(); // service query
    EXPECT_TRUE(_queryManager.registerQuery(query1));

    auto query2                   = make<ServiceQuery>(); // eventgroup query
    query2.description.eventGroup = 1U;
    EXPECT_TRUE(_queryManager.registerQuery(query2));

    ServiceQuery* queries[] = {&query1, &query2};

    uint64_t time = SD_DEFAULT_INITIAL_DELAY + 1U;
    uint32_t rep  = 1U;
    EXPECT_CALL(_serviceAnnouncer, find(Ref(query1.description)))
        .Times(1); // only find service-query
    _queryManager.updateQueries(time);
    for (auto& query : queries)
    {
        EXPECT_EQ(time, query->timestamp);
        EXPECT_EQ(rep, query->repetitionCount);
        EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE, query->state);
    }

    for (uint8_t i = 0U; i < SD_DEFAULT_REPETITIONS_MAX - 1U; ++i)
    {
        time += ((1U << (rep - 1U)) * SD_DEFAULT_REPETITIONS_BASE_DELAY);
        rep++;
        EXPECT_CALL(_serviceAnnouncer, find(Ref(query1.description)))
            .Times(1); // only find service-query
        _queryManager.updateQueries(time);
        for (auto& query : queries)
        {
            EXPECT_EQ(time, query->timestamp);
            EXPECT_EQ(rep, query->repetitionCount);
            if (rep == SD_DEFAULT_REPETITIONS_MAX)
            {
                EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query->state);
            }
            else
            {
                EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE, query->state);
            }
        }
    }
    EXPECT_EQ(SD_DEFAULT_REPETITIONS_MAX, rep);

    _queryManager.updateQueries(1000U); // no effect
    for (auto& query : queries)
    {
        EXPECT_EQ(time, query->timestamp);
        EXPECT_EQ(rep, query->repetitionCount);
        EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query->state);
    }
    _queryManager.unregisterQuery(query1);
    _queryManager.unregisterQuery(query2);
}

TEST_F(QueryManagerTest, updateTimeWithCustomSdConfig)
{
    SdConfig const TEST_SD_CONFIG{10U, 100U, 2U};
    uint64_t const TEST_START_TIME = 5U;

    auto query1 = make<ServiceQuery>(); // service query
    EXPECT_TRUE(_queryManager.registerQuery(query1));
    query1.sdConfig = TEST_SD_CONFIG;

    auto query2                   = make<ServiceQuery>(); // eventgroup query
    query2.description.eventGroup = 1U;
    query2.sdConfig               = TEST_SD_CONFIG;
    EXPECT_TRUE(_queryManager.registerQuery(query2));

    ServiceQuery* queries[] = {&query1, &query2};

    _queryManager.updateQueries(TEST_START_TIME);
    for (auto& query : queries)
    {
        EXPECT_EQ(TEST_START_TIME, query->timestamp);
        EXPECT_EQ(0U, query->repetitionCount);
        EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query->state);
    }

    uint64_t time = TEST_START_TIME + TEST_SD_CONFIG._initialDelay + 1U;
    uint32_t rep  = 1U;
    EXPECT_CALL(_serviceAnnouncer, find(Ref(query1.description)))
        .Times(1); // only find service-query
    _queryManager.updateQueries(time);
    for (auto& query : queries)
    {
        EXPECT_EQ(time, query->timestamp);
        EXPECT_EQ(rep, query->repetitionCount);
        EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE, query->state);
    }

    for (uint8_t i = 0U; i < TEST_SD_CONFIG._repetitionsMax - 1U; ++i)
    {
        time += ((1U << (rep - 1U)) * TEST_SD_CONFIG._repetitionsBaseDelay);
        rep++;
        EXPECT_CALL(_serviceAnnouncer, find(Ref(query1.description)))
            .Times(1); // only find service-query
        _queryManager.updateQueries(time);
        for (auto& query : queries)
        {
            EXPECT_EQ(time, query->timestamp);
            EXPECT_EQ(rep, query->repetitionCount);
            if (rep == TEST_SD_CONFIG._repetitionsMax)
            {
                EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query->state);
            }
            else
            {
                EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE, query->state);
            }
        }
    }
    EXPECT_EQ(TEST_SD_CONFIG._repetitionsMax, rep);

    _queryManager.updateQueries(1000U); // no effect
    for (auto& query : queries)
    {
        EXPECT_EQ(time, query->timestamp);
        EXPECT_EQ(rep, query->repetitionCount);
        EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query->state);
    }
    _queryManager.unregisterQuery(query1);
    _queryManager.unregisterQuery(query2);
}

TEST_F(QueryManagerTest, subscribeAckReceived)
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;

    auto query        = make<ServiceQuery>();
    query.description = service;
    EXPECT_TRUE(_queryManager.registerQuery(query));

    auto const sourceAddress      = make_ip4(192U, 0U, 2U, 0U);
    query.serviceDiscoveryAddress = sourceAddress;
    IPEndpoint endpoint(sourceAddress, 1U);

    // invalid state
    _queryManager.subscribeAckReceived(service, endpoint, make_ip4(192U, 0U, 2U, 1U));
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query.subscriptionState);

    query.state = ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE;

    _queryManager.subscribeAckReceived(service, endpoint, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_ACK_RECEIVED, query.subscriptionState);

    endpoint.setAddress(IPV4_MULTICAST_IP);
    EXPECT_CALL(_rpcReceiver, requestMulticastReception(Ref(endpoint))).Times(1);
    _queryManager.subscribeAckReceived(service, endpoint, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_ACK_RECEIVED, query.subscriptionState);
    EXPECT_TRUE(query.multicastAddress.isSet());

    // ip already set
    _queryManager.subscribeAckReceived(service, endpoint, sourceAddress);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, offerReceivedUdp)
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;
    service.eventGroup   = 1U;

    auto query        = make<ServiceQuery>();
    query.description = service;
    EXPECT_TRUE(_queryManager.registerQuery(query));

    service.eventGroup = ::someip::eventgroup_id::ALL;
    service.ipAddress  = make_ip4(192U, 0U, 2U, 0U);
    service.ttl        = 1U;
    service.proto      = static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_UDP);

    auto const sourceAddress = make_ip4(192U, 0U, 2U, 1U);
    // invalid state
    _queryManager.offerReceived(service, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query.subscriptionState);

    query.state = ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE;

    EXPECT_CALL(_serviceAnnouncer, subscribe(Ref(query.description), sourceAddress)).Times(1);
    _queryManager.offerReceived(service, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK, query.subscriptionState);
    EXPECT_EQ(service.ipAddress, query.description.ipAddress);
    EXPECT_EQ(service.ttl, query.description.ttl);

    query.state = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;

    EXPECT_CALL(_serviceAnnouncer, unsubscribe(Ref(query.description), sourceAddress)).Times(1);
    EXPECT_CALL(_serviceAnnouncer, subscribe(Ref(query.description), sourceAddress)).Times(1);
    _queryManager.offerReceived(service, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK, query.subscriptionState);
    EXPECT_CALL(_serviceAnnouncer, unsubscribe(Ref(query.description), sourceAddress)).Times(1);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, offerReceivedTcp)
{
    port::type localPort           = 11U;
    port::type remotePort          = 10U;
    auto query                     = make<ServiceQuery>();
    query.description              = ::someip::make<ServiceDescription>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = 1U;
    query.description.eventGroup   = 1U;
    query.description.port         = localPort;

    EXPECT_TRUE(_queryManager.registerQuery(query));

    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;
    service.eventGroup   = ::someip::eventgroup_id::ALL;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 0U);
    service.port         = remotePort;
    service.ttl          = 1U;
    service.proto        = proto::SD_L4_PROTO_TCP;

    ::ip::IPEndpoint expectedEp(service.ipAddress, service.port);

    query.state = ServiceQuery::ServiceQueryState::QUERY_IDLE_PHASE;

    auto const sourceAddress = make_ip4(192U, 0U, 2U, 1U);

    StrictMock<NetworkResourceMock> proxy;
    proxy.incRefCounter();
    ::etl::optional<NetworkChannel> networkChannel;
    networkChannel = NetworkChannel(proxy, expectedEp);

    // Query in INITIAL_WAIT_PHASE -> No TCP check
    EXPECT_CALL(proxy, isConnected()).Times(0);

    _queryManager.offerReceived(service, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query.subscriptionState);

    // Query in repetition phase and TCP connection NOT established
    query.state = ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE;
    EXPECT_CALL(
        _networkMock,
        getRpcChannel(
            localPort, expectedEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
        .WillRepeatedly(Return(networkChannel));
    EXPECT_CALL(proxy, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(_serviceAnnouncer, subscribe(Ref(query.description), sourceAddress)).Times(0);

    _queryManager.offerReceived(service, sourceAddress);
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED, query.subscriptionState);

    // Query in repetition phase and TCP connection established
    query.state = ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE;

    EXPECT_CALL(proxy, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(_serviceAnnouncer, subscribe(Ref(query.description), sourceAddress)).Times(1);

    _queryManager.offerReceived(service, sourceAddress);

    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK, query.subscriptionState);
    EXPECT_EQ(service.ipAddress, query.description.ipAddress);
    EXPECT_EQ(service.ttl, query.description.ttl);

    // Query in waiting for ack state -> no tcp checks
    query.state = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;
    // Next offer without SubscribeAck
    EXPECT_CALL(_serviceAnnouncer, unsubscribe(Ref(query.description), sourceAddress)).Times(1);
    EXPECT_CALL(_serviceAnnouncer, subscribe(Ref(query.description), sourceAddress)).Times(1);
    EXPECT_CALL(proxy, isConnected()).Times(0);

    _queryManager.offerReceived(service, sourceAddress);

    // Remove query
    EXPECT_EQ(ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK, query.subscriptionState);
    EXPECT_CALL(_serviceAnnouncer, unsubscribe(Ref(query.description), sourceAddress)).Times(1);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, stopOfferReceived)
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;

    auto query        = make<ServiceQuery>();
    query.description = service;
    EXPECT_TRUE(_queryManager.registerQuery(query));

    _queryManager.stopOfferReceived(service);
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query.state);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, updateServiceQuery_NoListener)
{
    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = ::someip::instance_id::ANY;
    query.listener                 = nullptr;

    EXPECT_TRUE(_queryManager.registerQuery(query));

    ServiceDescription service(query.description);
    service.instanceId = 1U;

    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_AVAILABLE);
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query.state);

    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE, query.state);
    _queryManager.unregisterQuery(query);
}

/**
 * Make sure query.timestamp does not change on update if new timestamp is smaller.
 */
TEST_F(QueryManagerTest, updateQueries_with_smaller_timestamp)
{
    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = ::someip::instance_id::ANY;
    query.listener                 = nullptr;

    EXPECT_TRUE(_queryManager.registerQuery(query));
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query.state);

    query.timestamp = 20U;

    // nothing should happen
    _queryManager.updateQueries(19U);
    _queryManager.unregisterQuery(query);
}

TEST_F(QueryManagerTest, updateQueriesWithTime_NoServiceAnnouncer)
{
    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = ::someip::instance_id::ANY;
    query.listener                 = nullptr;

    // disable service announcer
    _queryManager.wire(nullptr, &_rpcReceiver);
    EXPECT_TRUE(_queryManager.registerQuery(query));
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query.state);

    _queryManager.start();

    _queryManager.updateQueries(20U);
    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE, query.state);
    EXPECT_EQ(1U, query.repetitionCount);
    EXPECT_EQ(20U, query.timestamp);
    _queryManager.unregisterQuery(query);
}

/**
 * \refs: SMD_someip_QueryManager
 * \desc: Test that "serviceStatusChanged" of the listener for a registered query
 *        is called if a subscribeNack was received for this service.
 */
TEST_F(QueryManagerTest, subscribeNackReceived)
{
    TcpClientChannelValidator validator(_networkMock, true);
    declare::QueryManager<MAX_SERVICES> queryManager(validator);

    uint16_t const localPort  = 10U;
    uint16_t const remotePort = 20U;
    auto const sourceAddress  = make_ip4(0x01020304);
    ::ip::IPEndpoint const remoteEp(sourceAddress, remotePort);

    auto serviceDescription         = make<ServiceDescription>();
    serviceDescription.serviceId    = 1U;
    serviceDescription.majorVersion = 1U;
    serviceDescription.instanceId   = static_cast<uint16_t>(0xFFFF);
    serviceDescription.eventGroup   = 0x01;

    StrictMock<ServiceListenerMock> listener;

    auto query                    = make<ServiceQuery>();
    query.description             = serviceDescription;
    query.serviceDiscoveryAddress = sourceAddress;
    query.listener                = &listener;
    query.description.eventGroup  = 0xFFFF;
    query.description.port        = localPort;

    auto queryEg                    = make<ServiceQuery>();
    queryEg.description             = serviceDescription;
    queryEg.serviceDiscoveryAddress = sourceAddress;
    queryEg.listener                = nullptr;
    query.description.eventGroup    = 0x01;
    queryEg.description.port        = localPort;

    EXPECT_TRUE(queryManager.registerQuery(query));

    query.state             = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;
    query.subscriptionState = ServiceQuery::SubscriptionState::STATE_ACK_RECEIVED;

    EXPECT_CALL(
        listener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_SUBSCRIPTION_NACK))
        .Times(1);

    queryManager.subscribeNackReceived(serviceDescription, sourceAddress);
}

TEST_F(QueryManagerTest, shouldSearchingForServiceWhenTtlTimerExpired)
{
    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = ::someip::instance_id::ANY;
    query.listener                 = nullptr;

    EXPECT_TRUE(_queryManager.registerQuery(query));

    ServiceDescription service(query.description);
    service.instanceId = 1U;
    service.ttl        = 0U; // indication of the expired TTL timer

    _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);

    EXPECT_EQ(ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE, query.state);
}

/**
 * Make sure service announcer unsubscribe from all the services on stop
 */
TEST_F(QueryManagerTest, unsubscribeOnStop)
{
    auto query = make<ServiceQuery>();
    query.description
        = {/*minorVersion=*/1U,
           /*ttl=*/1U,
           /*serviceId=*/1U,
           /*instanceId=*/1U,
           /*eventGroup=*/1U,
           /*ipAddress=*/make_ip4(192U, 0U, 2U, 0U),
           /*port*/ 30501,
           /*proto=*/::someip::proto::SD_L4_PROTO_UDP,
           /*majorVersion=*/1U};

    _queryManager.registerQuery(query);

    EXPECT_CALL(
        _serviceAnnouncer, unsubscribe(Ref(query.description), query.serviceDiscoveryAddress));
    _queryManager.stop();
}

} // anonymous namespace
