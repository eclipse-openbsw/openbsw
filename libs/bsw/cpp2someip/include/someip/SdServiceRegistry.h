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

#include "someip/IServiceListener.h"
#include "someip/IServiceRegistry.h"
#include "someip/IServiceTrackerListener.h"
#include "someip/ISubscriptionManager.h"
#include "someip/QueryManager.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceTracker.h"

#include <async/Types.h>
#include <async/util/Call.h>

namespace someip
{
class SdServiceRegistry
: public IServiceRegistry
, public IServiceTrackerListener
{
public:
    SdServiceRegistry(
        ISubscriptionManager& subscriptionManager,
        ::async::ContextType const ethernetContext,
        ServiceManager& serviceManager,
        ServiceTracker& serviceTracker,
        QueryManager& queryManager);

    void init() override;
    void shutdown() override;

    bool registerProvidedService(ProvidedService& service) override;
    void unregisterProvidedService(ProvidedService& service) override;

    bool registerServiceQuery(ServiceQuery& query) override;
    void unregisterServiceQuery(ServiceQuery& query) override;

    QueryManager const* getQueryManager() const override;

    instance_id::type getInstanceId(
        service_id::type serviceId,
        major_version::type majorVersion,
        ::ip::IPAddress const& ipAddress,
        uint16_t port,
        bool remoteProvider = true) const override;

    void offerReceived(
        ServiceDescription const& receivedService, ::ip::IPAddress const& sourceAddress) override;

    SubscriptionResult subscribeReceived(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        eventgroup_id::type eventGroup,
        ttl::type ttl,
        ::ip::IPAddress const& ipAddress,
        uint16_t port,
        uint8_t proto) override;

    void subscribeAckReceived(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventGroup,
        major_version::type majorVersion,
        ::ip::IPEndpoint const& multicastEndpoint,
        ::ip::IPAddress const& sourceAddress) override;

    void subscribeNackReceived(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventGroup,
        major_version::type majorVersion,
        ::ip::IPAddress const& sourceAddress) override;

    void rebootDetected(::ip::IPAddress const& ipAddress) override;

    bool interestedInService(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion) const override;

    bool isEventgroupPort(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        uint16_t port) const override;

    uint16_t getCurrentNumberOfSubscriptions() const override;
    uint16_t getMaximumNumberOfSubscriptions() const override;
    uint16_t getCurrentNumberOfProvidedServices() const override;
    uint16_t getMaximumNumberOfProvidedServices() const override;
    uint16_t getCurrentNumberOfRemoteServices() const override;
    uint16_t getMaximumNumberOfRemoteServices() const override;

    /** \see IServiceTrackerListener::serviceTrackerChanged() */
    void
    serviceTrackerChanged(ServiceDescription const& service, ServiceTrackerStatus status) override;

    void cyclic();

private:
    static uint32_t const CYCLE_TIME_MS = 1000U; // 1 second

    void clearUpdateFlags();

    ::async::ContextType const _ethernetContext;
    ::async::Function _cyclicFunction;
    ::async::TimeoutType _cyclicTimeout;
    ISubscriptionManager& _subscriptionManager;
    ServiceManager& _serviceManager;
    ServiceTracker& _serviceTracker;
    QueryManager& _queryManager;
};

} // namespace someip
