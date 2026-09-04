/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/IServiceRegistry.h"
#include "someip/ProvidedService.h"

#include <gmock/gmock.h>

namespace someip
{
class ServiceRegistryMock : public IServiceRegistry
{
public:
    MOCK_METHOD(void, init, ());
    MOCK_METHOD(void, shutdown, ());
    MOCK_METHOD(bool, registerProvidedService, (ProvidedService & service));
    MOCK_METHOD(void, unregisterProvidedService, (ProvidedService & service));

    MOCK_METHOD(bool, registerServiceQuery, (ServiceQuery & query));
    MOCK_METHOD(void, unregisterServiceQuery, (ServiceQuery & query));

    MOCK_METHOD(QueryManager const*, getQueryManager, (), (const));

    MOCK_METHOD(
        uint16_t,
        getInstanceId,
        (service_id::type serviceId,
         major_version::type majorVersion,
         ::ip::IPAddress const& ipAddress,
         uint16_t port,
         bool remoteProvider),
        (const));

    MOCK_METHOD(
        void,
        offerReceived,
        (ServiceDescription const& receivedService, ::ip::IPAddress const& sourceAddress));

    MOCK_METHOD(
        SubscriptionResult,
        subscribeReceived,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         eventgroup_id::type eventGroup,
         ttl::type ttl,
         ::ip::IPAddress const& ipAddress,
         uint16_t port,
         uint8_t proto));

    MOCK_METHOD(
        void,
        subscribeAckReceived,
        (service_id::type serviceId,
         instance_id::type instanceId,
         eventgroup_id::type eventGroup,
         major_version::type majorVersion,
         ::ip::IPEndpoint const& multicastEndpoint,
         ::ip::IPAddress const& sourceAddress));

    MOCK_METHOD(
        void,
        subscribeNackReceived,
        (service_id::type serviceId,
         instance_id::type instanceId,
         eventgroup_id::type eventGroup,
         major_version::type majorVersion,
         ::ip::IPAddress const& sourceAddress));

    MOCK_METHOD(void, rebootDetected, (::ip::IPAddress const& ipAddress));

    MOCK_METHOD(
        bool,
        interestedInService,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion),
        (const));

    MOCK_METHOD(
        bool,
        isEventgroupPort,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         uint16_t port),
        (const));

    MOCK_METHOD(uint16_t, getCurrentNumberOfSubscriptions, (), (const));
    MOCK_METHOD(uint16_t, getMaximumNumberOfSubscriptions, (), (const));
    MOCK_METHOD(uint16_t, getCurrentNumberOfProvidedServices, (), (const));
    MOCK_METHOD(uint16_t, getMaximumNumberOfProvidedServices, (), (const));
    MOCK_METHOD(uint16_t, getCurrentNumberOfRemoteServices, (), (const));
    MOCK_METHOD(uint16_t, getMaximumNumberOfRemoteServices, (), (const));
};

} // namespace someip
