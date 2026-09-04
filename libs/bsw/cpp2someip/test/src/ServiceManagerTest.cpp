/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceManager.h"

#include "TestConstants.h"
#include "someip/ProvidedServiceListenerMock.h"
#include "someip/SdConfig.h"
#include "someip/ServiceAnnouncerMock.h"
#include "someip/ServiceAnnouncerTask.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/SomeIpConstants.h"

#include <ip/IPAddress.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;
using namespace ::test;

struct ServiceManagerTest : Test
{
    ServiceManagerTest() { _serviceManager.wire(&_serviceAnnouncer); }

    ::someip::declare::ServiceManager<3U> _serviceManager;
    StrictMock<ServiceAnnouncerMock> _serviceAnnouncer;

    StrictMock<ServiceHandlerMock<1U>> _serviceHandler;
    StrictMock<ServiceHandlerMock<1U>> _serviceHandler2;
    StrictMock<ProvidedServiceListenerMock> _serviceListener;
};

/**
 * Test process of registering and unregistering of ProvidedService to ServiceManager.
 */
TEST_F(ServiceManagerTest, register_and_unregister_ProvidedService)
{
    ProvidedService service1(_serviceHandler, &_serviceListener);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;

    auto serviceDescription1         = ::someip::make<ServiceDescription>();
    serviceDescription1.serviceId    = 1U;
    serviceDescription1.majorVersion = major_version::ANY;
    serviceDescription1.instanceId   = ::someip::instance_id::ANY;
    serviceDescription1.port         = 1U;
    serviceDescription1.proto        = 17U;
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service1.getState());

    EXPECT_TRUE(_serviceManager.registerService(service1));
    EXPECT_TRUE(_serviceManager.hasService(service1));
    EXPECT_TRUE(_serviceManager.hasServiceDescription(serviceDescription1));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service1.getState());

    ProvidedService service2(_serviceHandler, &_serviceListener);
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    service2.description.eventGroup   = 1U;

    auto serviceDescription2         = ::someip::make<ServiceDescription>();
    serviceDescription2.serviceId    = 1U;
    serviceDescription2.majorVersion = major_version::ANY;
    serviceDescription2.instanceId   = ::someip::instance_id::ANY;
    serviceDescription2.port         = 1U;
    serviceDescription2.proto        = 17U;
    serviceDescription2.eventGroup   = 1U;
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service2.getState());

    EXPECT_TRUE(_serviceManager.registerService(service2));
    EXPECT_TRUE(_serviceManager.hasService(service2));
    EXPECT_TRUE(_serviceManager.hasServiceDescription(serviceDescription2));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service2.getState());

    // no double register
    EXPECT_FALSE(_serviceManager.registerService(service1));
    EXPECT_FALSE(_serviceManager.registerService(service2));

    // eventgroup will be remove immediately
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service2))).Times(1);
    EXPECT_TRUE(_serviceManager.unregisterService(service2));
    EXPECT_FALSE(_serviceManager.hasService(service2));
    EXPECT_FALSE(_serviceManager.hasServiceDescription(serviceDescription2));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service2.getState());

    // service will be removed on update
    EXPECT_TRUE(_serviceManager.unregisterService(service1));
    EXPECT_TRUE(_serviceManager.hasService(service1));
    EXPECT_TRUE(_serviceManager.hasServiceDescription(serviceDescription1));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service1.getState());

    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service1))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    _serviceManager.updateServices(1U);
    EXPECT_FALSE(_serviceManager.hasService(service1));
    EXPECT_FALSE(_serviceManager.hasServiceDescription(serviceDescription1));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service1.getState());

    // no effect
    EXPECT_FALSE(_serviceManager.unregisterService(service1));
    EXPECT_FALSE(_serviceManager.unregisterService(service2));
}

/**
 * Make sure registerService() detects if requirements for success are not met.
 */
TEST_F(ServiceManagerTest, test_registerService_invalid)
{
    ProvidedService service1(_serviceHandler);
    EXPECT_FALSE(_serviceManager.registerService(service1));

    ProvidedService service2;
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    service2.description.eventGroup   = 1U;
    EXPECT_FALSE(_serviceManager.registerService(service2));
}

/**
 * Test ServiceManager lifecycle.
 */
TEST_F(ServiceManagerTest, test_ServiceManager_lifecycle)
{
    ProvidedService service1(_serviceHandler);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service1));

    ProvidedService service2(_serviceHandler);
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    service2.description.eventGroup   = 1U;
    EXPECT_TRUE(_serviceManager.registerService(service2));

    service1.setState(ProvidedService::ProvidedServiceState::MAIN_PHASE);
    service2.setState(ProvidedService::ProvidedServiceState::MAIN_PHASE);

    _serviceManager.start();
    EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service1.getState());
    EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service2.getState());

    _serviceManager.stop();
    EXPECT_EQ(ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE, service1.getState());
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service2.getState());

    _serviceManager.start();
    EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service1.getState());
    EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service2.getState());

    _serviceManager.unregisterService(service1);
    _serviceManager.unregisterService(service2);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    _serviceManager.updateServices(1U);
}

TEST_F(ServiceManagerTest, updateServices)
{
    // provided eventgroup
    ProvidedService service1(_serviceHandler, &_serviceListener);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service1));

    // provided eventgroup
    ProvidedService service2(_serviceHandler, &_serviceListener);
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    service2.description.eventGroup   = 1U;
    EXPECT_TRUE(_serviceManager.registerService(service2));

    ProvidedService* services[] = {&service1, &service2};

    uint64_t time = SD_DEFAULT_INITIAL_DELAY + 1U;
    uint32_t rep  = 1U;
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(1);
    _serviceManager.updateServices(time);
    for (auto& service : services)
    {
        EXPECT_EQ(time, service->getTimestamp());
        EXPECT_EQ(rep, service->getRepetitionCount());
        EXPECT_EQ(ProvidedService::ProvidedServiceState::REPETITION_PHASE, service->getState());
    }

    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service1.description))).Times(4);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(4);

    for (uint8_t i = 0U; i < SD_DEFAULT_REPETITIONS_MAX - 1U; ++i)
    {
        time += ((1U << (rep - 1U)) * SD_DEFAULT_REPETITIONS_BASE_DELAY);
        rep++;
        _serviceManager.updateServices(time);
        for (auto& service : services)
        {
            EXPECT_EQ(rep, service->getRepetitionCount());
            if (rep == SD_DEFAULT_REPETITIONS_MAX)
            {
                EXPECT_EQ((time / 1000U) * 1000U, service->getTimestamp());
                EXPECT_EQ(ProvidedService::ProvidedServiceState::MAIN_PHASE, service->getState());
            }
            else
            {
                EXPECT_EQ(time, service->getTimestamp());
                EXPECT_EQ(
                    ProvidedService::ProvidedServiceState::REPETITION_PHASE, service->getState());
            }
        }
    }
    EXPECT_EQ(SD_DEFAULT_REPETITIONS_MAX, rep);

    time = 1000U;
    _serviceManager.updateServices(time);
    for (auto& service : services)
    {
        EXPECT_EQ(time, service->getTimestamp());
        EXPECT_EQ(rep, service->getRepetitionCount());
        EXPECT_EQ(ProvidedService::ProvidedServiceState::MAIN_PHASE, service->getState());
    }

    service1.setState(ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE);
    service2.setState(ProvidedService::ProvidedServiceState::REMOVAL_PHASE);

    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service2.description))).Times(1);
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service2))).Times(1);
    _serviceManager.updateServices(time);

    EXPECT_TRUE(_serviceManager.hasService(service1));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service1.getState());
    EXPECT_EQ(0U, service1.getTimestamp());

    EXPECT_FALSE(_serviceManager.hasService(service2));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service2.getState());

    _serviceManager.unregisterService(service1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service1))).Times(1);
    _serviceManager.updateServices(1U);
}

TEST_F(ServiceManagerTest, updateServicesWithCustomSdConfig)
{
    SdOfferConfig const TEST_SD_CONFIG = {10U, 100U, 2U, 500U};
    uint64_t const TEST_START_TIME     = 5U;

    // provided service
    ProvidedService service1(_serviceHandler, &_serviceListener);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;
    service1.setSdConfig(TEST_SD_CONFIG);
    EXPECT_TRUE(_serviceManager.registerService(service1));

    // provided eventgroup
    ProvidedService service2(_serviceHandler, &_serviceListener);
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    service2.description.eventGroup   = 1U;
    service2.setSdConfig(TEST_SD_CONFIG);
    EXPECT_TRUE(_serviceManager.registerService(service2));

    ProvidedService* services[] = {&service1, &service2};

    _serviceManager.updateServices(TEST_START_TIME);
    for (auto& service : services)
    {
        EXPECT_EQ(TEST_START_TIME, service->getTimestamp());
        EXPECT_EQ(0U, service->getRepetitionCount());
        EXPECT_EQ(ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE, service->getState());
    }

    uint64_t time = TEST_START_TIME + TEST_SD_CONFIG._initialDelay + 1U;
    uint32_t rep  = 1U;
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(1);
    _serviceManager.updateServices(time);
    for (auto& service : services)
    {
        EXPECT_EQ(time, service->getTimestamp());
        EXPECT_EQ(rep, service->getRepetitionCount());
        EXPECT_EQ(ProvidedService::ProvidedServiceState::REPETITION_PHASE, service->getState());
    }

    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service1.description))).Times(2);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(2);

    for (uint8_t i = 0U; i < TEST_SD_CONFIG._repetitionsMax - 1U; ++i)
    {
        time += ((1U << (rep - 1U)) * TEST_SD_CONFIG._repetitionsBaseDelay);
        rep++;
        _serviceManager.updateServices(time);
        for (auto& service : services)
        {
            EXPECT_EQ(rep, service->getRepetitionCount());
            if (rep == TEST_SD_CONFIG._repetitionsMax)
            {
                EXPECT_EQ((time / 1000U) * 1000U, service->getTimestamp());
                EXPECT_EQ(ProvidedService::ProvidedServiceState::MAIN_PHASE, service->getState());
            }
            else
            {
                EXPECT_EQ(time, service->getTimestamp());
                EXPECT_EQ(
                    ProvidedService::ProvidedServiceState::REPETITION_PHASE, service->getState());
            }
        }
    }
    EXPECT_EQ(TEST_SD_CONFIG._repetitionsMax, rep);

    time = 1000U;
    _serviceManager.updateServices(time);
    for (auto& service : services)
    {
        EXPECT_EQ(time, service->getTimestamp());
        EXPECT_EQ(rep, service->getRepetitionCount());
        EXPECT_EQ(ProvidedService::ProvidedServiceState::MAIN_PHASE, service->getState());
    }

    service1.setState(ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE);
    service2.setState(ProvidedService::ProvidedServiceState::REMOVAL_PHASE);

    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service2.description))).Times(1);
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service2))).Times(1);
    _serviceManager.updateServices(time);

    EXPECT_TRUE(_serviceManager.hasService(service1));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service1.getState());
    EXPECT_EQ(0U, service1.getTimestamp());

    EXPECT_FALSE(_serviceManager.hasService(service2));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service2.getState());

    _serviceManager.unregisterService(service1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service1))).Times(1);
    _serviceManager.updateServices(1U);
}

/**
 * Make sure getService() finds a ProvidedService identified by ServiceID, MajorVersion and
 * InstanceID.
 */
TEST_F(ServiceManagerTest, test_getService)
{
    ProvidedService service(_serviceHandler);
    service.description.serviceId    = 1U;
    service.description.majorVersion = 1U;
    service.description.instanceId   = 1U;
    service.description.port         = 1U;
    service.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service));

    // valid
    {
        ServiceDescription temp(service.description);
        EXPECT_EQ(&service, _serviceManager.getService(temp));
    }
    {
        ServiceDescription temp(service.description);
        temp.eventGroup = 1U; // don't care
        EXPECT_EQ(&service, _serviceManager.getService(temp));
    }
    // invalid
    {
        ServiceDescription temp(service.description);
        temp.serviceId = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getService(temp));
    }
    {
        ServiceDescription temp(service.description);
        temp.majorVersion = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getService(temp));
    }
    {
        ServiceDescription temp(service.description);
        temp.instanceId = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getService(temp));
    }

    _serviceManager.unregisterService(service);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service.description))).Times(1);
    _serviceManager.updateServices(1U);
}

/**
 * Make sure getService() finds an EventGroup identified by ServiceID, MajorVersion, InstanceID and
 * EventGroupId.
 */
TEST_F(ServiceManagerTest, test_getEventGroup)
{
    ProvidedService service(_serviceHandler);
    service.description.serviceId    = 1U;
    service.description.majorVersion = 1U;
    service.description.instanceId   = 1U;
    service.description.port         = 1U;
    service.description.proto        = 17U;
    service.description.eventGroup   = 1U;
    EXPECT_TRUE(_serviceManager.registerService(service));

    // valid
    {
        ServiceDescription temp(service.description);
        EXPECT_EQ(&service, _serviceManager.getEventGroup(temp));
    }
    // invalid
    {
        ServiceDescription temp(service.description);
        temp.serviceId = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getEventGroup(temp));
    }
    {
        ServiceDescription temp(service.description);
        temp.majorVersion = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getEventGroup(temp));
    }
    {
        ServiceDescription temp(service.description);
        temp.instanceId = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getEventGroup(temp));
    }
    {
        ServiceDescription temp(service.description);
        temp.eventGroup = 2U;
        EXPECT_EQ(nullptr, _serviceManager.getEventGroup(temp));
    }

    _serviceManager.unregisterService(service);
}

/**
 * Make sure getHandler() finds a ServiceHandler identified by ServiceID, MajorVersion, Port and
 * Proto.
 */
TEST_F(ServiceManagerTest, test_getHandler)
{
    ProvidedService service(_serviceHandler);
    service.description.serviceId    = 1U;
    service.description.majorVersion = 1U;
    service.description.instanceId   = 1U;
    service.description.port         = 1U;
    service.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service));

    // valid
    {
        ServiceDescription temp(service.description);
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(&_serviceHandler, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_OK, result);
    }
    // invalid
    {
        auto temp = ::someip::make<ServiceDescription>();
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(nullptr, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_UNKNOWN, result);
    }
    {
        ServiceDescription temp(service.description);
        temp.serviceId = 2U;
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(nullptr, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_UNKNOWN, result);
    }
    {
        ServiceDescription temp(service.description);
        temp.port = 2U;
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(nullptr, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_UNKNOWN, result);
    }
    {
        ServiceDescription temp(service.description);
        temp.majorVersion = 2U;
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(nullptr, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_WRONG_MAJOR_VERSION, result);
    }

    _serviceManager.unregisterService(service);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service.description))).Times(1);
    _serviceManager.updateServices(1U);
}

/**
 * Make sure getHandler() finds a ServiceHandler correctly even if two handler have the same port
 * but differ in proto.
 */
TEST_F(ServiceManagerTest, test_getHandler_same_port_but_different_proto)
{
    ProvidedService service1(_serviceHandler);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service1));

    ProvidedService service2(_serviceHandler2);
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 6U;
    EXPECT_TRUE(_serviceManager.registerService(service2));

    {
        ServiceDescription temp(service1.description);
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(&_serviceHandler, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_OK, result);
    }

    {
        ServiceDescription temp(service2.description);
        ServiceManager::FindServiceResult result;
        EXPECT_EQ(&_serviceHandler2, _serviceManager.getHandler(temp, result));
        EXPECT_EQ(ServiceManager::FindServiceResult::FIND_SERVICE_OK, result);
    }

    _serviceManager.unregisterService(service1);
    _serviceManager.unregisterService(service2);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service2.description))).Times(1);
    _serviceManager.updateServices(1U);
}

/**
 * Make sure triggerOffers() triggers offers for ServiceAnnouncerTask correctly.
 */
TEST_F(ServiceManagerTest, test_triggerOffers)
{
    // provided service
    ProvidedService service1(_serviceHandler);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.minorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service1));

    // provided eventgroup
    ProvidedService service2(_serviceHandler);
    service2.description.serviceId    = 1U;
    service2.description.majorVersion = 1U;
    service2.description.minorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    service2.description.eventGroup   = 1U;
    EXPECT_TRUE(_serviceManager.registerService(service2));

    ServiceAnnouncerTask task1;
    task1.init(
        service1.description.serviceId,
        service1.description.instanceId,
        ::someip::eventgroup_id::ALL,
        0U,
        service1.description.majorVersion,
        service1.description.minorVersion);

    EXPECT_CALL(_serviceAnnouncer, offer(_)).Times(1);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(1);
    _serviceManager.triggerOffers(task1);

    ServiceAnnouncerTask task2;
    task2.init(
        service1.description.serviceId,
        service1.description.instanceId,
        ::someip::eventgroup_id::ALL,
        0U,
        major_version::ANY,
        service1.description.minorVersion);

    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(1);
    _serviceManager.triggerOffers(task2);

    ServiceAnnouncerTask task3;
    task3.init(
        service1.description.serviceId,
        ::someip::instance_id::ANY,
        ::someip::eventgroup_id::ALL,
        0U,
        major_version::ANY,
        service1.description.minorVersion);

    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, offer(Ref(service2.description))).Times(1);
    _serviceManager.triggerOffers(task3);

    ServiceAnnouncerTask task4;
    task4.init(
        service1.description.serviceId + 1U,
        ::someip::instance_id::ANY,
        ::someip::eventgroup_id::ALL,
        0U,
        major_version::ANY,
        service1.description.minorVersion);

    _serviceManager.triggerOffers(task4); // no effect

    _serviceManager.unregisterService(service1);
    _serviceManager.unregisterService(service2);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    _serviceManager.updateServices(1U);
}

/**
 * Make sure triggerStopOffer() calls stopOffer() implicitly if not called after stop.
 */
TEST_F(ServiceManagerTest, test_triggerStopOffers_implicit_stop)
{
    // provided service
    ProvidedService service1(_serviceHandler, &_serviceListener);
    service1.description.serviceId    = 1U;
    service1.description.majorVersion = 1U;
    service1.description.instanceId   = 1U;
    service1.description.port         = 1U;
    service1.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service1));

    // provided service
    ProvidedService service2(_serviceHandler, &_serviceListener);
    service2.description.serviceId    = 2U;
    service2.description.majorVersion = 1U;
    service2.description.instanceId   = 1U;
    service2.description.port         = 1U;
    service2.description.proto        = 17U;
    EXPECT_TRUE(_serviceManager.registerService(service2));

    // provided eventgroup
    ProvidedService service3(_serviceHandler, &_serviceListener);
    service3.description.serviceId    = 2U;
    service3.description.majorVersion = 1U;
    service3.description.instanceId   = 1U;
    service3.description.port         = 1U;
    service3.description.proto        = 17U;
    service3.description.eventGroup   = 1U;
    EXPECT_TRUE(_serviceManager.registerService(service3));

    EXPECT_TRUE(_serviceManager.unregisterService(service2));
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service2.getState());

    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service1.description))).Times(1);
    EXPECT_CALL(_serviceAnnouncer, stopOffer(Ref(service2.description))).Times(1);
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service2))).Times(1);

    _serviceManager.triggerStopOffers(); // implicit stop
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service1.getState());
    EXPECT_EQ(ProvidedService::ProvidedServiceState::REMOVAL_PHASE, service2.getState());
    EXPECT_EQ(ProvidedService::ProvidedServiceState::IDLE_PHASE, service3.getState());

    _serviceManager.triggerStopOffers(); // no effect

    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service1))).Times(1);
    _serviceManager.unregisterService(service1);
    EXPECT_CALL(_serviceListener, unregisterDone(Ref(service3))).Times(1);
    _serviceManager.unregisterService(service3);
}

} // anonymous namespace
