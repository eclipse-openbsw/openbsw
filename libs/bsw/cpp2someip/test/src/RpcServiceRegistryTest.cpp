/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcServiceRegistry.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/ServiceAnnouncerMock.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceTracker.h"
#include "someip/SomeIpConstants.h"

#include <gtest/gtest.h>

namespace
{

using namespace ::testing;
using namespace ::someip;
using namespace ::ip;

struct RpcServiceRegistryTest : ::testing::Test
{
    RpcServiceRegistryTest() : _registry(_serviceManager, _serviceTracker) { _registry.init(); }

    ~RpcServiceRegistryTest() override = default;

    ::someip::declare::ServiceManager<1U> _serviceManager;
    ::someip::declare::ServiceTracker<1U> _serviceTracker;

    RpcServiceRegistry _registry;
    StrictMock<SystemTimerMock> _stm;
};

TEST_F(RpcServiceRegistryTest, VirtualMethods)
{
    uint16_t const serviceId   = 1U;
    uint16_t const instanceId  = 1U;
    uint16_t const port        = 0U;
    uint8_t const majorVersion = 1U;

    EXPECT_FALSE(_registry.interestedInService(serviceId, instanceId, majorVersion));
    EXPECT_TRUE(_registry.isEventgroupPort(serviceId, instanceId, majorVersion, port));
    EXPECT_EQ(0U, _registry.getCurrentNumberOfSubscriptions());
    EXPECT_EQ(0U, _registry.getMaximumNumberOfSubscriptions());
}

/**
 * Make sure process of registering and unregistering ProvidedService affects
 * current number of ProvidedServices correctly.
 */
TEST_F(RpcServiceRegistryTest, test_getCurrentNumberOfProvidedServices)
{
    EXPECT_EQ(1U, _registry.getMaximumNumberOfProvidedServices());

    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = 1U;
    service.description.majorVersion = 1U;
    service.description.instanceId   = 1U;
    service.description.port         = 1U;
    service.description.proto        = proto::SD_L4_PROTO_UDP;
    EXPECT_EQ(0U, _registry.getCurrentNumberOfProvidedServices());

    _registry.registerProvidedService(service);
    EXPECT_EQ(1U, _registry.getCurrentNumberOfProvidedServices());

    _registry.unregisterProvidedService(service);
    EXPECT_EQ(0U, _registry.getCurrentNumberOfProvidedServices());
}

/**
 * Make sure process of adding and removing remote service affects
 * current number of remote services correctly.
 */
TEST_F(RpcServiceRegistryTest, test_getCurrentNumberOfRemoteServices)
{
    EXPECT_EQ(1U, _registry.getMaximumNumberOfRemoteServices());

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    auto serviceDescription         = ::someip::make<ServiceDescription>();
    serviceDescription.serviceId    = 1U;
    serviceDescription.majorVersion = 1U;
    serviceDescription.instanceId   = 1U;
    serviceDescription.ipAddress    = ::ip::make_ip4(1U);
    serviceDescription.port         = 1U;
    serviceDescription.ttl          = 1U;
    EXPECT_EQ(0U, _registry.getCurrentNumberOfRemoteServices());

    _serviceTracker.addService(serviceDescription);
    EXPECT_EQ(1U, _registry.getCurrentNumberOfRemoteServices());

    _serviceTracker.removeService(serviceDescription);
}

/**
 * Make sure hasService() reflects registering and unregistering of service correctly.
 */
TEST_F(RpcServiceRegistryTest, register_unregister_ProvidedService)
{
    StrictMock<ServiceHandlerMock<1U>> handler;
    ProvidedService service(handler);
    service.description.serviceId    = 1U;
    service.description.majorVersion = 1U;
    service.description.instanceId   = 1U;
    service.description.port         = 1U;
    service.description.proto        = proto::SD_L4_PROTO_UDP;

    _registry.registerProvidedService(service);
    EXPECT_TRUE(_serviceManager.hasService(service));

    _registry.unregisterProvidedService(service); // sync
    EXPECT_FALSE(_serviceManager.hasService(service));
}

TEST_F(RpcServiceRegistryTest, getInstanceId)
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 0U);
    service.port         = 1U;
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_EQ(
        ::someip::instance_id::ANY, _registry.getInstanceId(1U, 1U, service.ipAddress, 1U, true));

    EXPECT_TRUE(_serviceTracker.addService(service));
    EXPECT_EQ(1U, _registry.getInstanceId(1U, 1U, service.ipAddress, 1U, true));

    _serviceTracker.removeService(service);
    EXPECT_EQ(
        ::someip::instance_id::ANY, _registry.getInstanceId(1U, 1U, service.ipAddress, 1U, true));
}

TEST_F(RpcServiceRegistryTest, DummyFunctions)
{
    // if anything is called on this mock, the test will fail
    StrictMock<::someip::ServiceAnnouncerMock> announcer;

    // shouldn't register any queries.
    auto query = make<ServiceQuery>();
    _registry.registerServiceQuery(query);
    _registry.unregisterServiceQuery(query);

    auto receivedService = ::someip::make<ServiceDescription>();
    ::ip::IPAddress sourceAddress;

    _registry.offerReceived(receivedService, sourceAddress);

    auto remoteIp = make_ip4(192U, 0U, 2U, 0U);
    EXPECT_EQ(
        IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK,
        _registry.subscribeReceived(1U, 2U, 3U, 4U, 5U, remoteIp, 6U, 7U));
    _registry.subscribeAckReceived(1U, 2U, 3U, 4U, ::ip::IPEndpoint(), make_ip4(192U, 0U, 2U, 1U));

    _registry.rebootDetected(remoteIp);
}

} // anonymous namespace
