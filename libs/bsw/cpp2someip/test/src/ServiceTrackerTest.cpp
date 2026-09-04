/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceTracker.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/ServiceListenerMock.h"
#include "someip/ServiceQuery.h"
#include "someip/ServiceTrackerListenerMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/init.h"

#include <ip/IPAddress.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

static constexpr uint8_t MAX_SERVICES = 3U;

class ServiceTrackerTest : public Test
{
public:
    ServiceTrackerTest() { _tracker.init(_listener); }

protected:
    declare::ServiceTracker<MAX_SERVICES> _tracker;
    StrictMock<ServiceTrackerListenerMock> _listener;
    StrictMock<SystemTimerMock> _stm;
};

/**
 * Make sure LessThanComparator reflects relation between two TrackedServices correctly.
 */
TEST_F(ServiceTrackerTest, test_LessThanComparator_for_TrackedService)
{
    ::someip::internal::LessThanComparator less;

    ::someip::internal::TrackedService service1{};
    ::someip::internal::TrackedService service2{};
    service1.serviceDescription = make<ServiceDescription>();
    service2.serviceDescription = make<ServiceDescription>();
    EXPECT_FALSE(less(service1, service2));

    service1.serviceDescription.serviceId = 1U;
    service2.serviceDescription.serviceId = 2U;
    EXPECT_TRUE(less(service1, service2));

    service1.serviceDescription.serviceId = 2U;
    service2.serviceDescription.serviceId = 1U;
    EXPECT_FALSE(less(service1, service2));

    service1.serviceDescription.serviceId = 1U;
    service2.serviceDescription.serviceId = 1U;
    EXPECT_FALSE(less(service1, service2));

    service1.serviceDescription.majorVersion = 1U;
    service2.serviceDescription.majorVersion = 2U;
    EXPECT_TRUE(less(service1, service2));

    service1.serviceDescription.majorVersion = 2U;
    service2.serviceDescription.majorVersion = 1U;
    EXPECT_FALSE(less(service1, service2));

    service1.serviceDescription.majorVersion = 1U;
    service2.serviceDescription.majorVersion = 1U;
    EXPECT_FALSE(less(service1, service2));

    service1.serviceDescription.instanceId = 1U;
    service2.serviceDescription.instanceId = 2U;
    EXPECT_TRUE(less(service1, service2));

    service1.serviceDescription.instanceId = 2U;
    service2.serviceDescription.instanceId = 1U;
    EXPECT_FALSE(less(service1, service2));

    service1.serviceDescription.instanceId = 1U;
    service2.serviceDescription.instanceId = 1U;
    EXPECT_FALSE(less(service1, service2));
}

/**
 * Make sure invalid services cannot be added with addService() to ServiceTracker successfully.
 */
TEST_F(ServiceTrackerTest, test_addService_with_invalid_service)
{
    auto service = someip::make<ServiceDescription>();
    EXPECT_FALSE(_tracker.addService(service));

    service.serviceId = 1U;
    EXPECT_FALSE(_tracker.addService(service));

    service.majorVersion = 1U;
    EXPECT_FALSE(_tracker.addService(service));

    service.instanceId = 1U;
    EXPECT_FALSE(_tracker.addService(service));

    service.ttl = 0U;
    EXPECT_FALSE(_tracker.addService(service));
}

/**
 * Test process of adding and removing services to ServiceTracker.
 */
TEST_F(ServiceTrackerTest, test_adding_and_removing_services_to_ServiceTracker)
{
    auto service         = someip::make<ServiceDescription>();
    service.majorVersion = 1U;
    service.instanceId   = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    // add
    for (uint8_t i = 0U; i < MAX_SERVICES; ++i)
    {
        service.serviceId = i + 1U;
        auto temp(service);

        // insert
        service.ttl = 1U;
        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
            .Times(1);
        EXPECT_TRUE(_tracker.addService(service));
        EXPECT_TRUE(_tracker.getService(temp));

        EXPECT_TRUE(matches(service, temp));
        EXPECT_EQ(service.ttl, temp.ttl);

        // update
        service.ttl = 2U;
        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_CHANGED))
            .Times(0);
        EXPECT_TRUE(_tracker.addService(service));
        EXPECT_TRUE(_tracker.getService(temp));

        EXPECT_TRUE(matches(service, temp));
        EXPECT_EQ(service.ttl, temp.ttl);
    }

    // full
    service.serviceId = MAX_SERVICES + 1U;
    EXPECT_FALSE(_tracker.addService(service));
    EXPECT_FALSE(_tracker.getService(service));

    // remove
    for (uint8_t i = 0U; i < MAX_SERVICES; ++i)
    {
        service.serviceId = i + 1U;
        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
            .Times(1);
        _tracker.removeService(service);
        EXPECT_FALSE(_tracker.getService(service));
        _tracker.removeService(service); // no effect
    }
}

/**
 * Make sure serviceTrackerChanged() is triggered correctly in case of adding a new service to a
 * ServiceTracker or changing one which is already added.
 */
TEST_F(ServiceTrackerTest, test_serviceTrackerChanged)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
        .Times(1);
    EXPECT_TRUE(_tracker.addService(service));

    service.ipAddress = make_ip4(192U, 0U, 2U, 1U);
    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_CHANGED))
        .Times(1);
    EXPECT_TRUE(_tracker.addService(service));

    service.port = 1U;
    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_CHANGED))
        .Times(1);
    EXPECT_TRUE(_tracker.addService(service));

    service.proto = 1U;
    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_CHANGED))
        .Times(1);
    EXPECT_TRUE(_tracker.addService(service));
}

/**
 * Make sure serviceTrackerChanged() is triggered correctly in case of service becoming
 * SERVICE_RELIABLE.
 */
TEST_F(ServiceTrackerTest, test_serviceTrackerChanged_if_service_is_reliable)
{
    InSequence inSequence;

    uint32_t const offerCount  = 3U;
    uint32_t const offerTimeMs = 1000U;

    declare::ServiceTracker<MAX_SERVICES> tracker(
        ServiceTracker::ServiceReliabilityConfig{offerTimeMs, offerCount});
    tracker.init(_listener);

    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.instanceId   = 1U;
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).WillOnce(Return(1U));

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
        .Times(1U);

    EXPECT_TRUE(tracker.addService(service));

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).WillOnce(Return(2U));

    EXPECT_TRUE(tracker.addService(service));

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).WillOnce(Return(3U));

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_RELIABLE))
        .Times(1);

    EXPECT_TRUE(tracker.addService(service));
}

TEST_F(ServiceTrackerTest, GetInstanceId)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (uint8_t i = 0U; i < MAX_SERVICES; ++i)
    {
        service.instanceId = i + 1U;
        service.ipAddress  = make_ip4(192U, 0U, 2U, i + 1U);
        service.port       = i + 1U;

        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
            .Times(1);
        EXPECT_TRUE(_tracker.addService(service));
    }

    for (uint8_t i = 0U; i < MAX_SERVICES; ++i)
    {
        service.instanceId = ::someip::instance_id::ANY;
        service.ipAddress  = make_ip4(192U, 0U, 2U, i + 1U);
        service.port       = i + 1U;

        EXPECT_EQ((i + 1U), _tracker.getInstanceId(service));
    }

    // not found
    service.instanceId = 1U;
    service.ipAddress  = make_ip4(192U, 0U, 2U, 1U);
    service.port       = 1U;
    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(1);
    _tracker.removeService(service);
    EXPECT_EQ(::someip::instance_id::ANY, _tracker.getInstanceId(service));

    // invalid query
    service.majorVersion = 2U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 2U);
    service.port         = 2U;
    EXPECT_EQ(::someip::instance_id::ANY, _tracker.getInstanceId(service));
}

TEST_F(ServiceTrackerTest, rebootDetected)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 1U);
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (uint8_t i = 0U; i < MAX_SERVICES - 1U; ++i)
    {
        service.instanceId = i + 1U;
        service.port       = i + 1U;

        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
            .Times(1);
        EXPECT_TRUE(_tracker.addService(service));
    }

    service.instanceId = MAX_SERVICES + 1U;
    service.ipAddress  = make_ip4(192U, 0U, 2U, 2U);
    service.port       = 1U;

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
        .Times(1);
    EXPECT_TRUE(_tracker.addService(service));

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(_, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(MAX_SERVICES - 1U);
    _tracker.rebootDetected(make_ip4(192U, 0U, 2U, 1U));

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(_, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(1);
    _tracker.rebootDetected(make_ip4(192U, 0U, 2U, 2U));
}

TEST_F(ServiceTrackerTest, updateTTLs)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.ipAddress    = make_ip4(192U, 0U, 2U, 1U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (uint8_t i = 0U; i < MAX_SERVICES; ++i)
    {
        service.instanceId = i + 1U;
        service.port       = i + 1U;
        service.ttl        = i + 1U;

        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
            .Times(1);
        EXPECT_TRUE(_tracker.addService(service));
    }

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(_, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(0);
    _tracker.updateTTLs(0U);

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(_, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(0);
    _tracker.updateTTLs(1U);

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(_, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(1);
    _tracker.updateTTLs(1U);

    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(_, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED))
        .Times(2);
    _tracker.updateTTLs(2U);
}

TEST_F(ServiceTrackerTest, notifyServices)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (uint8_t i = 0U; i < MAX_SERVICES - 1U; ++i)
    {
        service.instanceId = i + 1U;
        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
            .Times(1);
        EXPECT_TRUE(_tracker.addService(service));
    }
    service.serviceId  = 2U;
    service.instanceId = 1U;
    EXPECT_CALL(
        _listener,
        serviceTrackerChanged(
            Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
        .Times(1);
    EXPECT_TRUE(_tracker.addService(service));

    StrictMock<ServiceListenerMock> listener;
    auto query                     = make<ServiceQuery>();
    query.description.serviceId    = 1U;
    query.description.majorVersion = 1U;
    query.description.instanceId   = ::someip::instance_id::ANY;
    query.listener                 = &listener;

    EXPECT_CALL(
        listener, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(MAX_SERVICES - 1U);
    _tracker.notifyServices(query);

    query.description.serviceId = 2U;
    EXPECT_CALL(
        listener, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    _tracker.notifyServices(query);

    query.description.serviceId  = 1U;
    query.description.instanceId = 1U;
    EXPECT_CALL(
        listener, serviceStatusChanged(_, IServiceListener::ServiceStatus::SERVICE_AVAILABLE))
        .Times(1);
    _tracker.notifyServices(query);
}

/**
 * Make sure service tracker clears the list of services when the stop is called
 */
TEST_F(ServiceTrackerTest, clearOnStop)
{
    auto service         = someip::make<ServiceDescription>();
    service.serviceId    = 1U;
    service.majorVersion = 1U;
    service.ttl          = 1U;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (uint8_t i = 0U; i < MAX_SERVICES; ++i)
    {
        service.instanceId = i + 1U;
        EXPECT_CALL(
            _listener,
            serviceTrackerChanged(
                Ref(service), IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED))
            .Times(1);
        EXPECT_TRUE(_tracker.addService(service));
    }

    EXPECT_EQ(_tracker.getCurrentNumberOfServices(), MAX_SERVICES);
    _tracker.stop();
    EXPECT_EQ(_tracker.getCurrentNumberOfServices(), 0U);
}

} // anonymous namespace
