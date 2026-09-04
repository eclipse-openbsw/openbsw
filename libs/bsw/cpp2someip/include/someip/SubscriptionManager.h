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

#include "someip/ISubscriptionManager.h"

#include <etl/flat_set.h>
#include <etl/ipool.h>

namespace someip
{
namespace internal
{

using EndpointPool   = ::etl::ipool;
using EventGroupPool = ::etl::ipool;

class FindEventGroupCondition
{
public:
    explicit FindEventGroupCondition(SubscribedEventGroup const& eventgroup);

    bool isValid() const;
    bool operator()(SubscribedEventGroup const* eventgroup) const;

private:
    SubscribedEventGroup const& _eventgroup;
};

class RemoveExpiredTtlCondition
{
public:
    RemoveExpiredTtlCondition(
        EndpointPool& endpoints, uint32_t ticks, internal::EventGroupPool& groupPool);

    bool operator()(SubscribedEventGroup* eventgroup) const;

private:
    EndpointPool& _endpoints;
    uint32_t const _ticks;
    internal::EventGroupPool& _groupPool;
};

} // namespace internal

class SubscriptionManager : public ISubscriptionManager
{
public:
    void stop();

    InternalSubscribeResult addSubscription(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroupId,
        ttl::type ttl,
        ::ip::IPAddress const& ipAddress,
        uint16_t port) override;

    void removeSubscription(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroupId,
        ::ip::IPAddress const& ipAddress,
        uint16_t port) override;

    void removeSubscriptions(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroupId) override;

    void removeSubscriptions(::ip::IPAddress const& ipAddress) override;

    void updateTTLs(uint32_t expiredSeconds) override;

    SubscriptionEndpointList*
    getSubscriptions(SubscribedEventGroup const& eventgroup) const override;

    uint16_t getCurrentNumberOfSubscriptions() const override;
    uint16_t getMaximumNumberOfSubscriptions() const override;

protected:
    struct LessThanComparator
    {
        bool operator()(SubscribedEventGroup const* lhs, SubscribedEventGroup const* rhs) const;
    };

    class RemoveIpCondition
    {
    public:
        RemoveIpCondition(
            internal::EndpointPool& endpoints,
            ::ip::IPAddress const& ipAddr,
            internal::EventGroupPool& groupPool);

        bool operator()(SubscribedEventGroup* eventgroup) const;

    private:
        internal::EndpointPool& _endpoints;
        ::ip::IPAddress const& _ip;
        internal::EventGroupPool& _groupPool;
    };

    using EventGroupList = ::etl::iflat_set<SubscribedEventGroup*, LessThanComparator>;

    SubscriptionManager(
        EventGroupList& groupList,
        internal::EventGroupPool& groupPool,
        internal::EndpointPool& endpointPool);

private:
    void clear();

    EventGroupList& _eventgroups;
    internal::EventGroupPool& _groupPool;
    internal::EndpointPool& _endpoints;
};

namespace declare
{
template<uint16_t NUM_SUBSCRIPTIONS>
class SubscriptionManager : public ::someip::SubscriptionManager
{
public:
    SubscriptionManager();

private:
    ::etl::flat_set<SubscribedEventGroup*, NUM_SUBSCRIPTIONS, LessThanComparator> _eventgroupList;

    ::etl::pool<SubscribedEventGroup, NUM_SUBSCRIPTIONS> _eventGroupPool;
    ::etl::pool<SubscriptionEndpoint, NUM_SUBSCRIPTIONS> _endpointPool;
};

template<uint16_t NUM_SUBSCRIPTIONS>
inline SubscriptionManager<NUM_SUBSCRIPTIONS>::SubscriptionManager()
: ::someip::SubscriptionManager(_eventgroupList, _eventGroupPool, _endpointPool)
{}

} // namespace declare
} // namespace someip
