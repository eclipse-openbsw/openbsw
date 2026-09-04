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

#include <ip/IPEndpoint.h>

// include to get typedefs for forward_list
#include "someip/ProvidedService.h"
#include "someip/ServiceQuery.h"

namespace someip
{
class IServiceAnnouncer;
class QueryManager;

class IServiceRegistry
{
public:
    enum class SubscriptionResult : uint8_t
    {
        SUBSCRIBE_OK,
        SUBSCRIBE_OK_MULTICAST,
        SUBSCRIBE_ERROR,
        UNSUBSCRIBE_OK,
        UNSUBSCRIBE_ERROR
    };

    enum class ProvidedServiceResult : uint8_t
    {
        STATUS_OK,
        SERVICE_UNKNOWN,
        WRONG_MAJOR_VERSION
    };

    virtual ~IServiceRegistry() = default;

    virtual void init()     = 0;
    virtual void shutdown() = 0;

    /**
     * Pure virtual function that handles registering provided services.
     */
    virtual bool registerProvidedService(ProvidedService& service) = 0;

    /**
     * Pure virtual function that handles unregistering provided services.
     */
    virtual void unregisterProvidedService(ProvidedService& service) = 0;

    /**
     * Pure virtual function that handles registering service queries.
     */
    virtual bool registerServiceQuery(ServiceQuery& query) = 0;

    /**
     * Pure virtual function that handles unregistering service queries.
     */
    virtual void unregisterServiceQuery(ServiceQuery& query) = 0;

    /**
     * Pure virtual function that returns QueryManager if possible.
     */
    virtual QueryManager const* getQueryManager() const = 0;

    /**
     * Pure virtual function that returns instance ID of given service.
     */
    virtual instance_id::type getInstanceId(
        service_id::type serviceId,
        major_version::type majorVersion,
        ::ip::IPAddress const& ipAddress,
        uint16_t port,
        bool remoteProvider = true) const
        = 0;

    /**
     * Pure virtual function that handles behaviour when receiving an offer.
     */
    virtual void
    offerReceived(ServiceDescription const& receivedService, ::ip::IPAddress const& sourceAddress)
        = 0;

    /**
     * Pure virtual function that handles behaviour when receiving a subscribe.
     */
    virtual SubscriptionResult subscribeReceived(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        eventgroup_id::type eventGroup,
        ttl::type ttl,
        ::ip::IPAddress const& ipAddress,
        uint16_t port,
        uint8_t proto)
        = 0;

    /**
     * Pure virtual function that handles behaviour when receiving a subscribe Ack.
     */
    virtual void subscribeAckReceived(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventGroup,
        major_version::type majorVersion,
        ::ip::IPEndpoint const& multicastEndpoint,
        ::ip::IPAddress const& sourceAddress)
        = 0;

    /**
     * Pure virtual function that handles behaviour when receiving a subscribe Nack.
     */
    virtual void subscribeNackReceived(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventGroup,
        major_version::type majorVersion,
        ::ip::IPAddress const& sourceAddress)
        = 0;

    /**
     * Pure virtual function that removes subscriptions in case of reboot.
     */
    virtual void rebootDetected(::ip::IPAddress const& ipAddress) = 0;

    /**
     * Pure virtual function that states if service of interest is available.
     */
    virtual bool interestedInService(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion) const
        = 0;

    /**
     * Pure virtual function that identifies a possible eventgroup port.
     */
    virtual bool isEventgroupPort(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        uint16_t port) const
        = 0;

    /**
     * Pure virtual function that returns current number of subscriptions.
     */
    virtual uint16_t getCurrentNumberOfSubscriptions() const = 0;

    /**
     * Pure virtual function that returns maximum number of subscriptions.
     */
    virtual uint16_t getMaximumNumberOfSubscriptions() const = 0;

    /**
     * Pure virtual function that returns current number of provided services.
     */
    virtual uint16_t getCurrentNumberOfProvidedServices() const = 0;

    /**
     * Pure virtual function that returns maximum number of provided services.
     */
    virtual uint16_t getMaximumNumberOfProvidedServices() const = 0;

    /**
     * Pure virtual function that returns current number of remote services.
     */
    virtual uint16_t getCurrentNumberOfRemoteServices() const = 0;

    /**
     * Pure virtual function that returns maximum number of remote services.
     */
    virtual uint16_t getMaximumNumberOfRemoteServices() const = 0;
};

} // namespace someip
