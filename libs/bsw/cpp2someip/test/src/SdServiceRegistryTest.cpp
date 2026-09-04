/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdServiceRegistry.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/NetworkMock.h"
#include "someip/QueryManager.h"
#include "someip/SdEndpoint.h"
#include "someip/ServiceAnnouncerMock.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/ServiceListenerMock.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceQuery.h"
#include "someip/ServiceTracker.h"
#include "someip/SomeIpConstants.h"
#include "someip/SubscriptionManagerMock.h"
#include "someip/TcpClientChannelValidator.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <ip/IPAddress.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::ip;
using namespace ::someip;
using namespace ::testing;

struct SdServiceRegistryTest : Test
{
    SdServiceRegistryTest() : _asyncMock(), _testContext(_ethernetContext)
    {
        _serviceManager.wire(&_pServiceAnnouncer);
        EXPECT_CALL(_asyncMock, scheduleAtFixedRate(_ethernetContext, _, _, _, _));
        _pServiceRegistry.init();
        _queryManager.wire(&_pServiceAnnouncer, nullptr);
        _testContext.handleAll();
    }

    ~SdServiceRegistryTest() override { _pServiceRegistry.shutdown(); }

    static uint16_t const ENDPOINT_PORT       = 30501U;
    static uint16_t const NUM_REMOTE_SERVICES = 16U;
    static uint16_t const NUM_LOCAL_QUERIES   = 17U;

    async::ContextType _ethernetContext{0U};
    ::someip::declare::ServiceManager<1U> _serviceManager;
    ::someip::declare::ServiceTracker<NUM_REMOTE_SERVICES> _serviceTracker;
    StrictMock<NetworkMock> _network;
    TcpClientChannelValidator _validator{_network, false};
    ::someip::declare::QueryManager<NUM_LOCAL_QUERIES> _queryManager{_validator};

    ServiceAnnouncerMock _pServiceAnnouncer;
    SubscriptionManagerMock _pSubscriptionManager;
    SdServiceRegistry _pServiceRegistry{
        _pSubscriptionManager, _ethernetContext, _serviceManager, _serviceTracker, _queryManager};
    ServiceListenerMock _pServiceListener;
    StrictMock<SystemTimerMock> _stm;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

/**
 * Make sure offerReceived() adds and offers service successfully in case ttl is valid and != 0. If
 * ttl does not meet these requirements, the service is removed and offer stopped.
 */
TEST_F(SdServiceRegistryTest, test_offerReceived)
{
    SdEndpoint const endpoint(
        ::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    auto receivedService         = make<ServiceDescription>();
    receivedService.serviceId    = 0x1234U;
    receivedService.instanceId   = 0x1U;
    receivedService.majorVersion = 0x1U;
    receivedService.minorVersion = 0x00U;
    receivedService.ttl          = 0x03U;
    receivedService.ipAddress    = endpoint.getAddress();
    receivedService.port         = endpoint.getPort();
    receivedService.proto        = endpoint.getProto();

    auto serviceQuery                     = ::someip::make<ServiceQuery>();
    serviceQuery.description.serviceId    = receivedService.serviceId;
    serviceQuery.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery.description.majorVersion = receivedService.majorVersion;
    serviceQuery.description.minorVersion = receivedService.minorVersion;
    serviceQuery.listener                 = &_pServiceListener;

    _pServiceRegistry.registerServiceQuery(serviceQuery);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    auto const sourceAddress = ::ip::make_ip4(20U);
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);

    // stop offer
    receivedService.ttl = 0x00U;
    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE))
        .Times(1);
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);
    _pServiceRegistry.unregisterServiceQuery(serviceQuery);
}

TEST_F(SdServiceRegistryTest, testServiceListenerUpdateAfterEndpointChange)
{
    SdEndpoint const endpoint(
        ::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);
    auto receivedService         = make<ServiceDescription>();
    receivedService.serviceId    = 0x1234U;
    receivedService.instanceId   = 0x1U;
    receivedService.majorVersion = 0x1U;
    receivedService.minorVersion = 0x00U;
    receivedService.ttl          = 0x03U;
    receivedService.ipAddress    = endpoint.getAddress();
    receivedService.port         = endpoint.getPort();
    receivedService.proto        = endpoint.getProto();

    auto serviceQuery = ::someip::make<ServiceQuery>();
    ::someip::init(serviceQuery);
    serviceQuery.description.serviceId    = receivedService.serviceId;
    serviceQuery.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery.description.majorVersion = receivedService.majorVersion;
    serviceQuery.description.minorVersion = receivedService.minorVersion;
    serviceQuery.listener                 = &_pServiceListener;
    _pServiceRegistry.registerServiceQuery(serviceQuery);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    auto const sourceAddress = ::ip::make_ip4(192U, 0U, 2U, 20U);
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);

    receivedService.ipAddress = ::ip::make_ip4(192U, 0U, 2U, 187U);
    receivedService.port      = ENDPOINT_PORT + 1U;
    receivedService.proto     = proto::SD_L4_PROTO_UDP + 1U;

    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE))
        .Times(1);
    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);

    _pServiceRegistry.unregisterServiceQuery(serviceQuery);
}

TEST_F(SdServiceRegistryTest, InterestedInService)
{
    EXPECT_FALSE(_pServiceRegistry.interestedInService(1U, 2U, 3U));
}

TEST_F(SdServiceRegistryTest, IsEventGroupPort)
{
    EXPECT_FALSE(_pServiceRegistry.isEventgroupPort(1U, 2U, 3U, 4U));
}

/**
 * Make sure maximum number of subscriptions, provided services, and remote services is returned
 * correctly.
 */
TEST_F(SdServiceRegistryTest, test_getting_max_number_of_subscriptions_provided_and_remote_services)
{
    EXPECT_CALL(_pSubscriptionManager, getMaximumNumberOfSubscriptions()).WillOnce(Return(13U));
    EXPECT_EQ(1U, _pServiceRegistry.getMaximumNumberOfProvidedServices());
    EXPECT_EQ(16U, _pServiceRegistry.getMaximumNumberOfRemoteServices());
    EXPECT_EQ(13U, _pServiceRegistry.getMaximumNumberOfSubscriptions());
}

/**
 * Make sure current number of subscriptions, provided services, and remote services is returned
 * correctly.
 */
TEST_F(
    SdServiceRegistryTest,
    test_getting_current_number_of_subscriptions_provided_and_remote_services)
{
    EXPECT_CALL(_pSubscriptionManager, getCurrentNumberOfSubscriptions()).WillOnce(Return(0U));
    EXPECT_EQ(0U, _pServiceRegistry.getCurrentNumberOfProvidedServices());
    EXPECT_EQ(0U, _pServiceRegistry.getCurrentNumberOfRemoteServices());
    EXPECT_EQ(0U, _pServiceRegistry.getCurrentNumberOfSubscriptions());
}

TEST_F(SdServiceRegistryTest, testSubscriptionAfterOffer)
{
    SdEndpoint const endpoint(
        ::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);
    auto receivedService         = make<ServiceDescription>();
    receivedService.serviceId    = 0x1234U;
    receivedService.instanceId   = 0x1U;
    receivedService.majorVersion = 0x1U;
    receivedService.minorVersion = 0x00U;
    receivedService.ttl          = 0x03U;
    receivedService.ipAddress    = endpoint.getAddress();
    receivedService.port         = endpoint.getPort();
    receivedService.proto        = endpoint.getProto();

    auto serviceQuery = ::someip::make<ServiceQuery>();
    ::someip::init(serviceQuery);
    serviceQuery.description.serviceId    = receivedService.serviceId;
    serviceQuery.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery.description.eventGroup   = 1U;
    serviceQuery.description.majorVersion = receivedService.majorVersion;
    serviceQuery.description.minorVersion = receivedService.minorVersion;
    serviceQuery.listener                 = &_pServiceListener;
    _pServiceRegistry.registerServiceQuery(serviceQuery);
    serviceQuery.state
        = ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE; // this is normally done by the
                                                                   // ServiceAnnouncer
    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    auto const sourceAddress = ::ip::make_ip4(20U);
    EXPECT_CALL(_pServiceAnnouncer, subscribe(Ref(serviceQuery.description), sourceAddress))
        .Times(1);

    _pServiceRegistry.offerReceived(receivedService, sourceAddress);

    EXPECT_CALL(_pServiceAnnouncer, unsubscribe(Ref(serviceQuery.description), sourceAddress))
        .Times(1);
    _pServiceRegistry.unregisterServiceQuery(serviceQuery);
}

TEST_F(SdServiceRegistryTest, testMultipleStopOfferReceived)
{
    SdEndpoint const endpoint(
        ::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);
    auto receivedService         = make<ServiceDescription>();
    receivedService.serviceId    = 0x1234U;
    receivedService.instanceId   = 0x1U;
    receivedService.majorVersion = 0x1U;
    receivedService.minorVersion = 0x00U;
    receivedService.ttl          = 0x03U;
    receivedService.ipAddress    = endpoint.getAddress();
    receivedService.port         = endpoint.getPort();
    receivedService.proto        = endpoint.getProto();

    auto serviceQuery = ::someip::make<ServiceQuery>();
    ::someip::init(serviceQuery);
    serviceQuery.description.serviceId    = receivedService.serviceId;
    serviceQuery.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery.description.majorVersion = receivedService.majorVersion;
    serviceQuery.description.minorVersion = receivedService.minorVersion;
    serviceQuery.listener                 = &_pServiceListener;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);

    auto const sourceAddress = ::ip::make_ip4(20U);
    _pServiceRegistry.registerServiceQuery(serviceQuery);
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);

    // stop offer
    receivedService.ttl = 0x00U;
    EXPECT_CALL(
        _pServiceListener,
        serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE))
        .Times(1);
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);

    // second stop offer
    receivedService.ttl = 0x00U;
    _pServiceRegistry.offerReceived(receivedService, sourceAddress);
    // not be called again
    _pServiceRegistry.unregisterServiceQuery(serviceQuery);
}

/**
 * Test subscribeReceived() in case of success.
 */
TEST_F(SdServiceRegistryTest, test_subscribeReceived_in_case_of_success)
{
    service_id::type serviceId       = 0x1234U;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroup   = 0x01;
    major_version::type majorVersion = 0x01;
    ttl::type ttl                    = 0x03;
    uint8_t proto                    = 0x11;

    SdEndpoint endpoint(::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.eventGroup   = eventgroup;
    service.description.majorVersion = majorVersion;
    service.description.ttl          = ttl;
    service.description.port         = endpoint.getPort();
    service.description.proto        = proto;
    _pServiceRegistry.registerProvidedService(service);

    EXPECT_CALL(
        handler,
        notifyInitialEvents(
            service.description.serviceId,
            service.description.instanceId,
            service.description.majorVersion,
            service.description.eventGroup,
            endpoint.getAddress(),
            service.description.port,
            service.description.proto))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(
        _pSubscriptionManager,
        addSubscription(
            serviceId,
            majorVersion,
            instanceId,
            eventgroup,
            ttl,
            endpoint.getAddress(),
            endpoint.getPort()))
        .Times(1)
        .WillOnce(Return(ISubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK));

    IServiceRegistry::SubscriptionResult subscriptionResult;
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());

    EXPECT_EQ(IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager, removeSubscriptions(serviceId, majorVersion, instanceId, eventgroup))
        .Times(1);

    _pServiceRegistry.unregisterProvidedService(service);
}

/**
 * Test subscribeReceived() for multicast in case of success.
 */
TEST_F(SdServiceRegistryTest, test_subscribeReceived_for_multicast_in_case_of_success)
{
    service_id::type serviceId       = 0x1234U;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroup   = 0x01;
    major_version::type majorVersion = 0x01;
    ttl::type ttl                    = 0x03;
    uint8_t proto                    = 0x11;

    SdEndpoint endpoint(
        ::ip::make_ip4(224U, 1U, 255U, 255U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.eventGroup   = eventgroup;
    service.description.majorVersion = majorVersion;
    service.description.ttl          = ttl;
    service.description.port         = endpoint.getPort();
    service.description.proto        = proto;
    service.description.ipAddress    = endpoint.getAddress();
    _pServiceRegistry.registerProvidedService(service);

    EXPECT_CALL(
        handler,
        notifyInitialEvents(
            service.description.serviceId,
            service.description.instanceId,
            service.description.majorVersion,
            service.description.eventGroup,
            endpoint.getAddress(),
            service.description.port,
            service.description.proto))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(
        _pSubscriptionManager,
        addSubscription(
            serviceId,
            majorVersion,
            instanceId,
            eventgroup,
            ttl,
            endpoint.getAddress(),
            endpoint.getPort()))
        .Times(1)
        .WillOnce(Return(ISubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK));

    IServiceRegistry::SubscriptionResult subscriptionResult;
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());

    EXPECT_EQ(IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK_MULTICAST, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager, removeSubscriptions(serviceId, majorVersion, instanceId, eventgroup))
        .Times(1);

    _pServiceRegistry.unregisterProvidedService(service);
}

/**
 * Make sure subscribeReceived() in case of invalid majorVersion is not successful.
 */
TEST_F(SdServiceRegistryTest, subscribeReceived_not_successful_in_case_of_invalid_majorVersion)
{
    service_id::type serviceId     = 0x1234U;
    instance_id::type instanceId   = 0x01;
    eventgroup_id::type eventgroup = 0x01;
    uint8_t providedMajorVersion   = 0x01;
    uint8_t subscribeMajorVersion  = 0x02;
    ttl::type ttl                  = 0x03;

    SdEndpoint endpoint(::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.eventGroup   = eventgroup;
    service.description.majorVersion = providedMajorVersion;
    service.description.ttl          = ttl;
    service.description.port         = endpoint.getPort();
    _pServiceRegistry.registerProvidedService(service);

    IServiceRegistry::SubscriptionResult subscriptionResult;
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        subscribeMajorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());

    EXPECT_EQ(IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager,
        removeSubscriptions(serviceId, providedMajorVersion, instanceId, eventgroup))
        .Times(1);

    _pServiceRegistry.unregisterProvidedService(service);
}

/**
 * Make sure subscribeReceived() in case of invalid endpoint protocol is not successful.
 */
TEST_F(SdServiceRegistryTest, subscribeReceived_not_successful_in_case_of_invalid_endpoint_protocol)
{
    service_id::type serviceId       = 0x1234U;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroup   = 0x01;
    major_version::type majorVersion = 0x01;
    ttl::type ttl                    = 0x03;

    SdEndpoint endpoint(::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT,
                        0x8D); // invalid protocol

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.eventGroup   = eventgroup;
    service.description.majorVersion = majorVersion;
    service.description.ttl          = ttl;
    service.description.port         = endpoint.getPort();
    _pServiceRegistry.registerProvidedService(service);

    IServiceRegistry::SubscriptionResult subscriptionResult;
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());

    EXPECT_EQ(IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager, removeSubscriptions(serviceId, majorVersion, instanceId, eventgroup))
        .Times(1);

    _pServiceRegistry.unregisterProvidedService(service);
}

/**
 * Test unsubscribe() in case of success.
 */
TEST_F(SdServiceRegistryTest, test_unsubscribe_in_case_of_success)
{
    service_id::type serviceId       = 0x1234U;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroup   = 0x01;
    major_version::type majorVersion = 0x01;
    ttl::type ttl                    = 0x03;
    uint8_t proto                    = 0x11;

    SdEndpoint endpoint(::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.eventGroup   = eventgroup;
    service.description.majorVersion = majorVersion;
    service.description.ttl          = ttl;
    service.description.port         = endpoint.getPort();
    service.description.proto        = proto;
    _pServiceRegistry.registerProvidedService(service);

    EXPECT_CALL(
        _pSubscriptionManager,
        addSubscription(
            serviceId,
            majorVersion,
            instanceId,
            eventgroup,
            ttl,
            endpoint.getAddress(),
            endpoint.getPort()))
        .Times(1)
        .WillOnce(Return(ISubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK));

    SubscriptionEndpoint subscriptionEndpoint;
    SubscriptionEndpointList subscriptionList;
    subscriptionList.push_front(subscriptionEndpoint);

    EXPECT_CALL(_pSubscriptionManager, getSubscriptions(_))
        .Times(1)
        .WillOnce(Return(&subscriptionList));

    EXPECT_CALL(handler, onEventGroupSubscriptionStateChanged(service.description.eventGroup, true))
        .Times(1);

    EXPECT_CALL(
        handler,
        notifyInitialEvents(
            service.description.serviceId,
            service.description.instanceId,
            service.description.majorVersion,
            service.description.eventGroup,
            endpoint.getAddress(),
            service.description.port,
            service.description.proto))
        .Times(1)
        .WillOnce(Return(true));

    IServiceRegistry::SubscriptionResult subscriptionResult;
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());
    EXPECT_EQ(IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager,
        removeSubscription(
            serviceId,
            majorVersion,
            instanceId,
            eventgroup,
            endpoint.getAddress(),
            endpoint.getPort()))
        .Times(1);

    subscriptionList.clear();
    EXPECT_CALL(_pSubscriptionManager, getSubscriptions(_))
        .Times(1)
        .WillOnce(Return(&subscriptionList));

    EXPECT_CALL(
        handler, onEventGroupSubscriptionStateChanged(service.description.eventGroup, false))
        .Times(1);

    ttl                = 0U; // unsubscribe
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());
    EXPECT_EQ(IServiceRegistry::SubscriptionResult::UNSUBSCRIBE_OK, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager, removeSubscriptions(serviceId, majorVersion, instanceId, eventgroup))
        .Times(1);

    _pServiceRegistry.unregisterProvidedService(service);
}

/**
 * Make sure unsubscribe() is not successful in case of invalid service.
 */
TEST_F(SdServiceRegistryTest, unsubscribe_unsuccessful_in_case_of_invalid_service)
{
    service_id::type serviceId       = 0x1234U;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroup   = 0x01;
    major_version::type majorVersion = 0x01;
    ttl::type ttl                    = 0x03;

    SdEndpoint endpoint(::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.eventGroup   = eventgroup;
    service.description.majorVersion = majorVersion;
    service.description.ttl          = ttl;
    service.description.port         = endpoint.getPort();
    _pServiceRegistry.registerProvidedService(service);

    ttl = 0U;           // unsubscribe
    majorVersion += 1U; // change major version to unsubscribe incompatible service
    IServiceRegistry::SubscriptionResult subscriptionResult;
    subscriptionResult = _pServiceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpoint.getAddress(),
        endpoint.getPort(),
        endpoint.getProto());
    EXPECT_EQ(IServiceRegistry::SubscriptionResult::UNSUBSCRIBE_ERROR, subscriptionResult);

    EXPECT_CALL(
        _pSubscriptionManager,
        removeSubscriptions(serviceId, majorVersion - 1U, instanceId, eventgroup))
        .Times(1);

    _pServiceRegistry.unregisterProvidedService(service);
}

TEST_F(SdServiceRegistryTest, testRebootDetected)
{
    service_id::type serviceId1 = 0x1234U;

    SdEndpoint endpoint1(::ip::make_ip4(192U, 0U, 2U, 1U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    service_id::type serviceId2 = 0x1235U;

    SdEndpoint endpoint2(::ip::make_ip4(192U, 0U, 2U, 2U), ENDPOINT_PORT, proto::SD_L4_PROTO_UDP);

    service_id::type serviceId3 = 0x1236U;

    SdEndpoint endpoint3(
        ::ip::make_ip4(192U, 0U, 2U, 1U), // services 1 and 3 are offered by the same IP
        ENDPOINT_PORT,
        proto::SD_L4_PROTO_UDP);

    instance_id::type instanceId     = 0x01;
    major_version::type majorVersion = 0x01;
    minor_version::type minorVersion = 0x00;
    ttl::type ttl                    = 0x03;

    StrictMock<ServiceListenerMock> listener1;
    auto serviceQuery1 = ::someip::make<ServiceQuery>();
    ::someip::init(serviceQuery1);
    serviceQuery1.description.serviceId    = serviceId1;
    serviceQuery1.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery1.description.majorVersion = majorVersion;
    serviceQuery1.description.minorVersion = minorVersion;
    serviceQuery1.listener                 = &listener1;
    _pServiceRegistry.registerServiceQuery(serviceQuery1);

    StrictMock<ServiceListenerMock> listener2;
    auto serviceQuery2 = ::someip::make<ServiceQuery>();
    ::someip::init(serviceQuery2);
    serviceQuery2.description.serviceId    = serviceId2;
    serviceQuery2.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery2.description.majorVersion = majorVersion;
    serviceQuery2.description.minorVersion = minorVersion;
    serviceQuery2.listener                 = &listener2;
    _pServiceRegistry.registerServiceQuery(serviceQuery2);

    StrictMock<ServiceListenerMock> listener3;
    auto serviceQuery3 = make<ServiceQuery>();
    ::someip::init(serviceQuery3);
    serviceQuery3.description.serviceId    = serviceId3;
    serviceQuery3.description.instanceId   = ::someip::instance_id::ANY;
    serviceQuery3.description.majorVersion = majorVersion;
    serviceQuery3.description.minorVersion = minorVersion;
    serviceQuery3.listener                 = &listener3;
    _pServiceRegistry.registerServiceQuery(serviceQuery3);

    auto receivedService1         = make<ServiceDescription>();
    receivedService1.serviceId    = serviceId1;
    receivedService1.instanceId   = instanceId;
    receivedService1.majorVersion = majorVersion;
    receivedService1.minorVersion = minorVersion;
    receivedService1.ttl          = ttl;
    receivedService1.ipAddress    = endpoint1.getAddress();
    receivedService1.port         = endpoint1.getPort();
    receivedService1.proto        = endpoint1.getProto();

    auto receivedService2      = receivedService1;
    receivedService2.serviceId = serviceId2;
    receivedService2.ipAddress = endpoint2.getAddress();
    receivedService2.port      = endpoint2.getPort();
    receivedService2.proto     = endpoint2.getProto();

    auto receivedService3      = receivedService1;
    receivedService3.serviceId = serviceId3;
    receivedService3.ipAddress = endpoint3.getAddress();
    receivedService3.port      = endpoint3.getPort();
    receivedService3.proto     = endpoint3.getProto();

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    EXPECT_CALL(
        listener1, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    auto const sourceAddress = ::ip::make_ip4(20U);
    _pServiceRegistry.offerReceived(receivedService1, sourceAddress);

    EXPECT_CALL(
        listener2, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    _pServiceRegistry.offerReceived(receivedService2, sourceAddress);

    EXPECT_CALL(
        listener3, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    _pServiceRegistry.offerReceived(receivedService3, sourceAddress);

    EXPECT_CALL(
        listener1, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE))
        .Times(1);
    EXPECT_CALL(
        listener3, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE))
        .Times(1);
    EXPECT_CALL(_pSubscriptionManager, removeSubscriptions(endpoint1.getAddress())).Times(1);
    _pServiceRegistry.rebootDetected(endpoint1.getAddress());
    _pServiceRegistry.unregisterServiceQuery(serviceQuery1);
    _pServiceRegistry.unregisterServiceQuery(serviceQuery2);
    _pServiceRegistry.unregisterServiceQuery(serviceQuery3);
}

/**
 * Test successfully registering and unregistering of ProvidedService.
 */
TEST_F(SdServiceRegistryTest, successfully_registering_and_unregistering_ProvidedService)
{
    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = 1U;
    service.description.majorVersion = 1U;
    service.description.instanceId   = 1U;
    service.description.port         = 1U;
    service.description.proto        = 17U;

    _pServiceRegistry.registerProvidedService(service);
    EXPECT_TRUE(_serviceManager.hasService(service));

    _pServiceRegistry.unregisterProvidedService(service); // async
    EXPECT_TRUE(_serviceManager.hasService(service));

    EXPECT_CALL(_pServiceAnnouncer, stopOffer(Ref(service.description))).Times(1);
    _serviceManager.updateServices(1U);
    EXPECT_FALSE(_serviceManager.hasService(service));
}

/**
 * Make sure instanceId of a service is identified by serviceId, majorVersion, ipAddress and port.
 * If instanceID cannot be found SomeIpMessageConstants::INSTANCE_ID_ANY will be returned.
 */
TEST_F(SdServiceRegistryTest, getInstanceId)
{
    ServiceDescription service{};
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;
    service.ipAddress    = ::ip::make_ip4(192U, 0U, 2U, 1U);
    service.port         = 1U;
    service.proto        = 17U;
    service.ttl          = 1U;

    EXPECT_EQ(
        ::someip::instance_id::ANY,
        _pServiceRegistry.getInstanceId(1U, 1U, ::ip::make_ip4(192U, 0U, 2U, 1U), 1U, true));

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_TRUE(_serviceTracker.addService(service));

    EXPECT_EQ(
        1U, _pServiceRegistry.getInstanceId(1U, 1U, ::ip::make_ip4(192U, 0U, 2U, 1U), 1U, true));
}

} // anonymous namespace
