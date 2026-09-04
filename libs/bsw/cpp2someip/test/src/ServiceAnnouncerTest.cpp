/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceAnnouncer.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/NetworkMock.h"
#include "someip/QueryManager.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/SessionManager.h"
#include "someip/SomeIpConstants.h"
#include "someip/TcpClientChannelValidator.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <ip/IPAddress.h>

#include <gmock/gmock.h>

namespace
{
// SOME 225: cleanup
using namespace ::testing;
using namespace ::common;
using namespace ::ip;
using namespace ::someip;

struct ServiceAnnouncerTest
: ::testing::Test
, ServiceAnnouncer
{
    ServiceAnnouncerTest()
    : ServiceAnnouncer(
        _network,
        _serviceManager,
        _serviceRegistry,
        _ethernetContext,
        _queryManager,
        _sessionManager)
    , _asyncMock()
    , _testContext(_ethernetContext)
    {
        _queryManager.wire(this, nullptr);
        _testContext.handleAll();

        init();
        EXPECT_CALL(_asyncMock, scheduleAtFixedRate(_ethernetContext, _, _, _, _));
        start();
    }

    static inline async::ContextType _ethernetContext{0U};
    StrictMock<NetworkMock> _network;
    TcpClientChannelValidator _validator{_network, false};
    ::someip::declare::SessionManager<16U> _sessionManager;
    ::someip::declare::QueryManager<16U> _queryManager{_validator};
    ::someip::declare::ServiceManager<1U> _serviceManager;
    StrictMock<ServiceRegistryMock> _serviceRegistry;
    StrictMock<SystemTimerMock> _stm;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;

    void verifyPendingMessages(size_t numBrowse, size_t numTx) const;
    void expectOkTimeout(uint32_t numTimes);
};

void ServiceAnnouncerTest::verifyPendingMessages(size_t numBrowse, size_t numTx) const
{
    EXPECT_EQ(numBrowse, getNumPendingBrowseRequests());
    EXPECT_EQ(numTx, getNumPendingTxMessages());
}

void ServiceAnnouncerTest::expectOkTimeout(uint32_t numTimes)
{
    if (numTimes == 1U)
    {
        // EXPECT_CALL(_timeoutManager, set(_, _, _))
        //     .Times(1)
        //     .WillOnce(Return(TimeoutManager2Mock::TIMEOUT_OK));
    }
    else
    {
        // EXPECT_CALL(_timeoutManager, set(_, _, _))
        //     .Times(numTimes)
        //     .WillRepeatedly(Return(TimeoutManager2Mock::TIMEOUT_OK));
    }
    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(numTimes);
}

/**
 * Make sure respondToSubscribe() is successful if SUBSCRIBE_OK_MULTICAST == result and
 * providedService != nullptr.
 */
TEST_F(ServiceAnnouncerTest, test_respondToSubscribe_SUBSCRIBE_OK_MULTICAST_success)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 2U;
    major_version::type majorVersion = 3U;
    eventgroup_id::type eventGroup   = 4U;
    ttl::type ttl                    = 5U;
    IPAddress source                 = make_ip4(192U, 0U, 2U, 1U);
    IPAddress endpoint               = make_ip4(192U, 0U, 2U, 2U);
    port::type const port            = 6U;
    proto::type const proto          = proto::SD_L4_PROTO_UDP;

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.majorVersion = majorVersion;
    service.description.eventGroup   = eventGroup;
    service.description.ttl          = ttl;
    service.description.port         = port;
    service.description.proto        = proto;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_TRUE(_serviceManager.registerService(service));
    service.setState(ProvidedService::ProvidedServiceState::MAIN_PHASE);

    IServiceRegistry::SubscriptionResult result
        = IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK_MULTICAST;
    EXPECT_CALL(
        _serviceRegistry,
        subscribeReceived(
            serviceId, instanceId, majorVersion, eventGroup, ttl, endpoint, port, proto))
        .Times(1)
        .WillOnce(Return(result));
    expectOkTimeout(1);
    respondToSubscribe(
        serviceId, instanceId, majorVersion, 0U, eventGroup, ttl, source, endpoint, port, proto);
    _serviceManager.unregisterService(service);
}

/**
 * Make sure respondToSubscribe() is not successful if SUBSCRIBE_OK_MULTICAST == result but
 * providedService == nullptr
 */
TEST_F(ServiceAnnouncerTest, test_respondToSubscribe_SUBSCRIBE_OK_MULTICAST_failure)
{
    IServiceRegistry::SubscriptionResult result
        = IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK_MULTICAST;

    EXPECT_CALL(
        _serviceRegistry,
        subscribeReceived(1U, 2U, 4U, 3U, 5U, make_ip4(192U, 0U, 2U, 2U), 10U, 11U))
        .Times(1)
        .WillOnce(Return(result));

    respondToSubscribe(
        1U, 2U, 4U, 0U, 3U, 5U, make_ip4(192U, 0U, 2U, 1U), make_ip4(192U, 0U, 2U, 2U), 10U, 11U);
    verifyPendingMessages(0U, 0U);
}

/**
 * Make sure respondToSubscribe() triggers Nack if result == SUBSCRIBE_ERROR.
 */
TEST_F(ServiceAnnouncerTest, test_respondToSubscribe_SUBSCRIBE_ERROR)
{
    IServiceRegistry::SubscriptionResult result
        = IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(
        _serviceRegistry,
        subscribeReceived(1U, 2U, 4U, 3U, 5U, make_ip4(192U, 0U, 2U, 2U), 10U, 11U))
        .Times(1)
        .WillOnce(Return(result));
    expectOkTimeout(1);

    respondToSubscribe(
        1U, 2U, 4U, 0U, 3U, 5U, make_ip4(192U, 0U, 2U, 1U), make_ip4(192U, 0U, 2U, 2U), 10U, 11U);
    verifyPendingMessages(0U, 1U);
}

/**
 * Make sure nothing happens if respondToSubscribe() is invoked and result is an unkown error.
 */
TEST_F(ServiceAnnouncerTest, test_respondToSubscribe_unknown_error)
{
    IServiceRegistry::SubscriptionResult result = (IServiceRegistry::SubscriptionResult)50;

    EXPECT_CALL(
        _serviceRegistry,
        subscribeReceived(1U, 2U, 4U, 3U, 5U, make_ip4(192U, 0U, 2U, 2U), 10U, 11U))
        .Times(1)
        .WillOnce(Return(result));

    respondToSubscribe(
        1U, 2U, 4U, 0U, 3U, 5U, make_ip4(192U, 0U, 2U, 1U), make_ip4(192U, 0U, 2U, 2U), 10U, 11U);
    verifyPendingMessages(0U, 0U);
}

/**
 * Test sendSubscribeAck() in case of success.
 */
TEST_F(ServiceAnnouncerTest, test_sendSubscribeAck_success)
{
    IPAddress const& sourceIp = make_ip4(192U, 0U, 2U, 13U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(_network, getMulticastIp()).WillRepeatedly(ReturnRef(sourceIp));
    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(1);
    // EXPECT_CALL(_timeoutManager, set(_, _, _))
    //     .Times(1)
    //     .WillOnce(Return(TimeoutManager2Mock::TIMEOUT_OK));
    sendSubscribeAck(1U, 2U, 3U, 4U, 5U, 6U, make_ip4(192U, 0U, 2U, 20U));
}

TEST_F(ServiceAnnouncerTest, ProcessTaskSubscribeAckMulticast_EventGroup)
{
    uint64_t const timestamp = 1U;
    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).WillOnce(Return(timestamp));

    expectOkTimeout(1);
    sendSubscribeAckMulticast(
        1U, 2U, 3U, 4U, 5U, 6U, make_ip4(192U, 0U, 2U, 20U), 7U, make_ip4(192U, 0U, 2U, 30U));

    verifyPendingMessages(0U, 1U);

    IPAddress multiCastIp = make_ip4(192U, 0U, 2U, 40U);

    EXPECT_CALL(_network, getMulticastIp()).Times(1).WillOnce(ReturnRef(multiCastIp));
    EXPECT_CALL(_network, getSdPort(true)).Times(1).WillOnce(Return(6U));

    // this will trigger a network send, so let's avoid that for now
    ::etl::optional<NetworkChannel> channel;
    EXPECT_CALL(_network, getSdChannel(6U, _)).Times(1).WillOnce(Return(channel));

    checkPendingTasks(timestamp + 15U);
    verifyPendingMessages(0U, 0U);
}

TEST_F(ServiceAnnouncerTest, ProcessTaskSubscribeAckMulticast_NoEventGroup)
{
    uint64_t const timestamp = 1U;
    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).WillOnce(Return(timestamp));

    expectOkTimeout(1);
    sendSubscribeAckMulticast(
        1U,
        2U,
        ::someip::eventgroup_id::ALL,
        4U,
        5U,
        6U,
        make_ip4(192U, 0U, 2U, 20U),
        7U,
        make_ip4(192U, 0U, 2U, 30U));

    verifyPendingMessages(0U, 1U);

    checkPendingTasks(timestamp + 15U);
    verifyPendingMessages(0U, 0U);
}

TEST_F(ServiceAnnouncerTest, ProcessTaskUnsubscribe_EventGroup)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.instanceId   = 2U;
    service.eventGroup   = 3U;
    service.majorVersion = 6U;
    service.minorVersion = 7U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 20U);

    auto const sdIpAddress = make_ip4(192U, 0U, 2U, 30U);

    expectOkTimeout(1);

    IPAddress localIp     = make_ip4(192U, 0U, 2U, 20U);
    IPAddress multiCastIp = make_ip4(192U, 0U, 2U, 40U);

    uint64_t const timestamp = 1U;
    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).WillOnce(Return(timestamp));

    EXPECT_CALL(_network, getLocalIp()).Times(2).WillRepeatedly(ReturnRef(localIp));
    EXPECT_CALL(_network, getMulticastIp()).Times(1).WillOnce(ReturnRef(multiCastIp));
    EXPECT_CALL(_network, getSdPort(true)).Times(1).WillOnce(Return(6U));

    unsubscribe(service, sdIpAddress);

    verifyPendingMessages(0U, 1U);

    // this will trigger a network send, so let's avoid that for now
    ::etl::optional<NetworkChannel> channel;
    EXPECT_CALL(_network, getSdChannel(6U, _)).Times(1).WillOnce(Return(channel));

    checkPendingTasks(timestamp + 15U);
    verifyPendingMessages(0U, 0U);
}

TEST_F(ServiceAnnouncerTest, SendSubscribeNack)
{
    IPAddress nack = make_ip4(192U, 0U, 2U, 20U);

    expectOkTimeout(1);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    sendSubscribeNack(1U, 2U, 3U, 4U, 5U, nack);
    verifyPendingMessages(0U, 1U);
}

TEST_F(ServiceAnnouncerTest, Subscribe_EventGroup)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.instanceId   = 2U;
    service.eventGroup   = 3U;
    service.majorVersion = 6U;
    service.minorVersion = 7U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 20U);
    service.ttl          = 2000U;

    auto const sdIpAddress = make_ip4(192U, 0U, 2U, 30U);

    expectOkTimeout(1);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    IPAddress localIp = make_ip4(192U, 0U, 2U, 30U);
    EXPECT_CALL(_network, getLocalIp()).Times(1).WillOnce(ReturnRef(localIp));

    subscribe(service, sdIpAddress);
    verifyPendingMessages(0U, 1U);
}

TEST_F(ServiceAnnouncerTest, Subscribe_NotEventGroup)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.instanceId   = 2U;
    service.eventGroup   = ::someip::eventgroup_id::ALL;
    service.majorVersion = 6U;
    service.minorVersion = 7U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 20U);
    service.ttl          = 2000U;

    auto const sdIpAddress = make_ip4(192U, 0U, 2U, 30U);

    subscribe(service, sdIpAddress);
    // shouldn't get enqueued
    verifyPendingMessages(0U, 0U);
}

/**
 * Make sure nothing happens on calling start() if ServiceAnnouncer is already started.
 */
TEST_F(ServiceAnnouncerTest, test_starting_ServiceAnnouncer_twice)
{
    // nothing else should be called.
    start();
}

/**
 * Make sure shutting down ServiceAnnouncer is executed correctly.
 */
TEST_F(ServiceAnnouncerTest, test_shutting_down_ServiceAnnouncer)
{
    IPAddress const& sourceIp = make_ip4(192U, 0U, 2U, 13U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(_network, getMulticastIp()).WillRepeatedly(ReturnRef(sourceIp));
    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(2);
    shutdown();
}

TEST_F(ServiceAnnouncerTest, RespondToFindService_InstanceIdAny)
{
    IPAddress sourceIp = make_ip4(192U, 0U, 2U, 20U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    respondToFindService(1U, ::someip::instance_id::ANY, 3U, 4U, 2000U, sourceIp, true);
    verifyPendingMessages(1U, 0U);
}

/**
 * Make sure respondToFindService() is successful if majorVersion == MAJOR_VERSION_ANY.
 */
TEST_F(ServiceAnnouncerTest, respondToFindService_is_successful_with_MAJOR_VERSION_ANY)
{
    IPAddress sourceIp = make_ip4(192U, 0U, 2U, 20U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    respondToFindService(1U, 2U, major_version::ANY, 4U, 2000U, sourceIp, true);
    verifyPendingMessages(1U, 0U);
}

/**
 * Make sure respondToFindService() is successful if minorVersion == MINOR_VERSION_ANY.
 */
TEST_F(ServiceAnnouncerTest, respondToFindService_is_successful_with_MINOR_VERSION_ANY)
{
    IPAddress sourceIp = make_ip4(192U, 0U, 2U, 20U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    respondToFindService(1U, 2U, 3U, minor_version::ANY, 2000U, sourceIp, true);
    verifyPendingMessages(1U, 0U);
}

/**
 * Make sure respondToFindService() is successful if majorVersion == MAJOR_VERSION_ANY and
 * minorVersion == MINOR_VERSION_ANY.
 */
TEST_F(
    ServiceAnnouncerTest,
    respondToFindService_is_successful_with_MAJOR_VERSION_ANY_and_MINOR_VERSION_ANY)
{
    IPAddress sourceIp = make_ip4(192U, 0U, 2U, 20U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    respondToFindService(1U, 2U, major_version::ANY, minor_version::ANY, 2000U, sourceIp, true);
    verifyPendingMessages(1U, 0U);
}

TEST_F(ServiceAnnouncerTest, Expired)
{
    IPAddress const& sourceIp = make_ip4(13U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    EXPECT_CALL(_network, getMulticastIp()).WillRepeatedly(ReturnRef(sourceIp));
    // AbstractTimeoutMock timeoutMock;
    // AbstractTimeout::TimeoutExpiredActions actions(timeoutMock);
    // expired(actions);
}

TEST_F(ServiceAnnouncerTest, RespondToFindService_MainPhase)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 2U;
    major_version::type majorVersion = 3U;
    minor_version::type minorVersion = 4U;
    ttl::type ttl                    = 5U;
    IPAddress ip                     = make_ip4(192U, 0U, 2U, 20U);
    uint16_t port                    = 6U;
    uint8_t proto                    = 17U;

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.majorVersion = majorVersion;
    service.description.ttl          = ttl;
    service.description.port         = port;
    service.description.proto        = proto;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_TRUE(_serviceManager.registerService(service));
    service.setState(ProvidedService::ProvidedServiceState::MAIN_PHASE);

    expectOkTimeout(1);
    respondToFindService(serviceId, instanceId, majorVersion, minorVersion, ttl, ip, true);
    verifyPendingMessages(0U, 1U);

    EXPECT_TRUE(_serviceManager.unregisterService(service));
}

/**
 * Make sure on calling unsubscribe() nothing happens if service contain no eventGroup.
 */
TEST_F(ServiceAnnouncerTest, unsubscribe_has_no_effect_if_service_has_no_eventGroup)
{
    auto service           = someip::make<ServiceDescription>();
    auto const sdIpAddress = make_ip4(192U, 0U, 2U, 30U);

    // no event group
    unsubscribe(service, sdIpAddress);
    verifyPendingMessages(0U, 0U);
}

/**
 * Make sure unsubscribe() is successful if service has eventGroup.
 */
TEST_F(ServiceAnnouncerTest, unsubscribe_is_successful_for_service_with_eventGroup)
{
    auto service       = someip::make<ServiceDescription>();
    service.instanceId = 1U;
    service.eventGroup = 2U;
    service.port       = 3U;
    service.ipAddress  = make_ip4(192U, 0U, 2U, 20U);

    auto const sdIpAddress = make_ip4(192U, 0U, 2U, 30U);

    IPAddress localIp = make_ip4(192U, 0U, 2U, 30U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(_network, getLocalIp()).Times(1).WillOnce(ReturnRef(localIp));
    expectOkTimeout(1);

    unsubscribe(service, sdIpAddress);
    verifyPendingMessages(0U, 1U);
}
} // anonymous namespace
