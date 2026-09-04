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

#include "someip/SomeIpConstants.h"
#include "someip/SubscribedEventGroup.h"
#include "someip/SubscriptionEndpoint.h"

#include <ip/IPAddress.h>

namespace someip
{
class ISubscriptionManager
{
public:
    enum class InternalSubscribeResult : uint8_t
    {
        INTERNAL_SUBSCRIBE_OK,
        INTERNAL_SUBSCRIBE_ERROR,
        INTERNAL_ALREADY_SUBSCRIBED
    };

    virtual ~ISubscriptionManager() = default;

    /**
     * Pure virtual function that adds a subscription associated with:
     *  - ip and port
     *
     * for an eventgroup identified by:
     *
     *  - serviceId
     *  - majorVersion
     *  - instanceId
     *  - eventgroupId
     *
     *  \return result of subscription
     */
    virtual InternalSubscribeResult addSubscription(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroupId,
        ttl::type ttl,
        ::ip::IPAddress const& ipAddress,
        uint16_t port)
        = 0;

    /**
     * Pure virtual function that remove a subscription associated with:
     *  - ip and port
     *
     * for an eventgroup identified by:
     *
     *  - serviceId
     *  - majorVersion
     *  - instanceId
     *  - eventgroupId
     */
    virtual void removeSubscription(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroupId,
        ::ip::IPAddress const& ipAddress,
        uint16_t port)
        = 0;

    /**
     * Overloaded pure virtual function that remove all subscriptions
     * for an eventgroup identified by:
     *
     *  - serviceId
     *  - majorVersion
     *  - instanceId
     *  - eventgroup
     */
    virtual void removeSubscriptions(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroupId)
        = 0;

    /**
     * Overloaded pure virtual function that removes all subscriptions for
     * all subscriptions associated with given ip.
     */
    virtual void removeSubscriptions(::ip::IPAddress const& ipAddress) = 0;

    /**
     * Pure virtual function that is responsible for updating **TTL** of all subscriptions.
     *
     * \post subscriptions with ttl == 0 will be removed
     */
    virtual void updateTTLs(uint32_t expiredSeconds) = 0;

    /**
     * Pure virtual function that returns subscriptions of an eventgroup.
     *
     * \return subscriptions or 0L if none found.
     */
    virtual SubscriptionEndpointList* getSubscriptions(SubscribedEventGroup const& eventgroup) const
        = 0;

    /**
     * Pure virtual function that returns current number of subscriptions.
     */
    virtual uint16_t getCurrentNumberOfSubscriptions() const = 0;

    /**
     * Pure virtual function that returns maximum number of subscriptions.
     */
    virtual uint16_t getMaximumNumberOfSubscriptions() const = 0;
};
} // namespace someip
