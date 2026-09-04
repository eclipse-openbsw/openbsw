/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SubscriptionManager.h"

#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <etl/algorithm.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPAddress;
using ::util::logger::SOMEIP;

// protected
SubscriptionManager::SubscriptionManager(
    EventGroupList& groupList,
    internal::EventGroupPool& groupPool,
    internal::EndpointPool& endpointPool)
: _eventgroups(groupList), _groupPool(groupPool), _endpoints(endpointPool)
{}

void SubscriptionManager::stop() { clear(); }

void SubscriptionManager::clear()
{
    for (auto eg = _eventgroups.rbegin(); eg != _eventgroups.rend(); ++eg)
    {
        SubscriptionEndpointList& endpoints
            = const_cast<SubscribedEventGroup&>(**eg).getEndpoints();

        SubscriptionEndpointList::iterator endpoint   = endpoints.begin();
        SubscriptionEndpointList::iterator const prev = endpoints.before_begin();

        while (endpoint != endpoints.end())
        {
            endpoint->clear();
            SubscriptionEndpointList::iterator const current = endpoint;
            endpoint                                         = endpoints.erase_after(prev);
            _endpoints.release(&(*current));
        }

        _groupPool.release(&(**eg));
        auto it = _eventgroups.find(*eg);
        if (it != _eventgroups.end())
        {
            _eventgroups.erase(it);
        }
    }
}

// virtual
SubscriptionManager::InternalSubscribeResult SubscriptionManager::addSubscription(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroupId,
    ttl::type const ttl,
    IPAddress const& ipAddress,
    uint16_t const port)
{
    SubscribedEventGroup const eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpoint const endpoint(ipAddress, port);

    internal::FindEventGroupCondition const condition(eventgroup);

    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "SubscriptionManager::addSubscription() invalid condition");
        return InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR;
    }

    SubscribedEventGroup* eg = nullptr;
    bool isNew               = false;

    EventGroupList::const_iterator const findIter
        = _eventgroups.find(const_cast<SubscribedEventGroup*>(&eventgroup));
    if (findIter != _eventgroups.end()) // already contained
    {
        eg = *findIter;
    }
    else if (_eventgroups.full()) // list full
    {
        WARN_LOG(SOMEIP, "SubscriptionManager::addSubscription() eventgroup list full");
        return InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR;
    }
    else // add as new
    {
        DEBUG_LOG(
            SOMEIP,
            "SubscriptionManager::addSubscription(service: %d, version: %d, instance: %d, "
            "eventgroup: %d, ttl: %d)",
            serviceId,
            majorVersion,
            instanceId,
            eventgroupId,
            ttl);
        SubscribedEventGroup* seg = _groupPool.create<SubscribedEventGroup>(
            serviceId, majorVersion, instanceId, eventgroupId);
        eg = seg;
        (void)_eventgroups.insert(eg);
        isNew = true;
    }

    SubscriptionEndpointList& endpoints = eg->getEndpoints();
    for (auto& ep : endpoints)
    {
        if (ep == endpoint)
        {
            ep.ttl = ttl;
            return InternalSubscribeResult::INTERNAL_ALREADY_SUBSCRIBED;
        }
    }

    if (_endpoints.full())
    {
        WARN_LOG(SOMEIP, "SubscriptionManager::addSubscription() endpoint pool full");

        if (isNew)
        {
            _groupPool.release(eg);
            auto it = _eventgroups.find(eg);
            if (it != _eventgroups.end())
            {
                _eventgroups.erase(it);
            }
        }

        return InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR;
    }

    SubscriptionEndpoint& ep = *_endpoints.create<SubscriptionEndpoint>();
    endpoints.push_front(ep);
    ep.setAddress(ipAddress);
    ep.setPort(port);
    ep.ttl = ttl;

    return InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK;
}

// virtual
void SubscriptionManager::removeSubscription(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroupId,
    IPAddress const& ipAddress,
    uint16_t const port)
{
    SubscribedEventGroup const eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpoint const endpoint(ipAddress, port);

    internal::FindEventGroupCondition const condition(eventgroup);

    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "SubscriptionManager::removeSubscription() invalid condition");
        return;
    }

    EventGroupList::const_iterator const removeIter
        = _eventgroups.find(const_cast<SubscribedEventGroup*>(&eventgroup));
    if (removeIter == _eventgroups.end())
    {
        return;
    }

    DEBUG_LOG(
        SOMEIP,
        "SubscriptionManager::removeSubscription(service: %d, version: %d, instance: %d, "
        "eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventgroupId);
    SubscribedEventGroup& eg = (**removeIter);

    SubscriptionEndpointList& endpoints = eg.getEndpoints();
    for (auto& ep : endpoints)
    {
        if (ep == endpoint)
        {
            ep.clear();
            endpoints.remove_if([&ep](SubscriptionEndpoint const& e) { return &e == &ep; });
            _endpoints.release(&ep);

            if (endpoints.empty())
            {
                _groupPool.release(&eg);
                auto it = _eventgroups.find(&eg);
                if (it != _eventgroups.end())
                {
                    _eventgroups.erase(it);
                }
            }

            break;
        }
    }
}

// virtual
void SubscriptionManager::removeSubscriptions(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroupId)
{
    SubscribedEventGroup const eventgroup(serviceId, majorVersion, instanceId, eventgroupId);

    internal::FindEventGroupCondition const condition(eventgroup);

    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "SubscriptionManager::removeSubscriptions() invalid condition");
        return;
    }

    EventGroupList::const_iterator const removeIter
        = _eventgroups.find(const_cast<SubscribedEventGroup*>(&eventgroup));
    if (removeIter == _eventgroups.end())
    {
        return;
    }

    DEBUG_LOG(
        SOMEIP,
        "SubscriptionManager::removeSubscriptions(service: %d, version: %d, instance: %d, "
        "eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventgroupId);
    SubscribedEventGroup& eg = (**removeIter);

    SubscriptionEndpointList& endpoints = eg.getEndpoints();
    // Release all endpoints back to pool
    while (!endpoints.empty())
    {
        auto& ep = endpoints.front();
        ep.clear();
        endpoints.pop_front();
        _endpoints.release(&ep);
    }
    _groupPool.release(&eg);
    auto it = _eventgroups.find(&eg);
    if (it != _eventgroups.end())
    {
        _eventgroups.erase(it);
    }
}

// virtual
void SubscriptionManager::removeSubscriptions(IPAddress const& ipAddress)
{
    RemoveIpCondition const condition(_endpoints, ipAddress, _groupPool);
    auto it = _eventgroups.begin();
    while (it != _eventgroups.end())
    {
        if (condition(*it))
        {
            it = _eventgroups.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// virtual
void SubscriptionManager::updateTTLs(uint32_t const expiredSeconds)
{
    internal::RemoveExpiredTtlCondition const condition(_endpoints, expiredSeconds, _groupPool);
    auto it = _eventgroups.begin();
    while (it != _eventgroups.end())
    {
        if (condition(*it))
        {
            it = _eventgroups.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// virtual
SubscriptionEndpointList*
SubscriptionManager::getSubscriptions(SubscribedEventGroup const& eventgroup) const
{
    internal::FindEventGroupCondition const condition(eventgroup);

    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "SubscriptionManager::getSubscriptions() invalid condition");
        return nullptr;
    }

    EventGroupList::const_iterator const itr
        = _eventgroups.find(const_cast<SubscribedEventGroup*>(&eventgroup));
    if (itr != _eventgroups.end())
    {
        if ((*itr) != nullptr)
        {
            return &((*itr)->getEndpoints());
        }
    }

    return nullptr;
}

// virtual
uint16_t SubscriptionManager::getCurrentNumberOfSubscriptions() const
{
    return static_cast<uint16_t>(_endpoints.size());
}

// virtual
uint16_t SubscriptionManager::getMaximumNumberOfSubscriptions() const
{
    return static_cast<uint16_t>(_endpoints.max_size());
}

bool SubscriptionManager::LessThanComparator::operator()(
    SubscribedEventGroup const* const lhs, SubscribedEventGroup const* const rhs) const
{
    if (lhs->getServiceId() != rhs->getServiceId())
    {
        return lhs->getServiceId() < rhs->getServiceId();
    }

    if (lhs->getInstanceId() != rhs->getInstanceId())
    {
        return lhs->getInstanceId() < rhs->getInstanceId();
    }

    if (lhs->getEventgroupId() != rhs->getEventgroupId())
    {
        return lhs->getEventgroupId() < rhs->getEventgroupId();
    }

    return lhs->getMajorVersion() < rhs->getMajorVersion();
}

SubscriptionManager::RemoveIpCondition::RemoveIpCondition(
    internal::EndpointPool& endpoints, IPAddress const& ipAddr, internal::EventGroupPool& groupPool)
: _endpoints(endpoints), _ip(ipAddr), _groupPool(groupPool)
{}

bool SubscriptionManager::RemoveIpCondition::operator()(
    SubscribedEventGroup* const eventgroup) const
{
    SubscriptionEndpointList& endpoints = eventgroup->getEndpoints();

    SubscriptionEndpointList::iterator endpoint = endpoints.begin();
    SubscriptionEndpointList::iterator prev     = endpoints.before_begin();

    while (endpoint != endpoints.end())
    {
        if (endpoint->getAddress() == _ip)
        {
            endpoint->clear();
            SubscriptionEndpointList::iterator const current = endpoint;
            endpoint                                         = endpoints.erase_after(prev);
            _endpoints.release(&(*current));
        }
        else
        {
            prev = endpoint;
            ++endpoint;
        }
    }
    if (endpoints.empty())
    {
        _groupPool.release(eventgroup);
        return true;
    }
    return false;
}

namespace internal
{
FindEventGroupCondition::FindEventGroupCondition(SubscribedEventGroup const& eventgroup)
: _eventgroup(eventgroup)
{}

bool FindEventGroupCondition::isValid() const
{
    return (
        (_eventgroup.getServiceId() != service_id::INVALID)
        && (_eventgroup.getMajorVersion() != major_version::INVALID)
        && (_eventgroup.getInstanceId() != instance_id::ANY)
        && (_eventgroup.getEventgroupId() != eventgroup_id::ALL));
}

bool FindEventGroupCondition::operator()(SubscribedEventGroup const* const eventgroup) const
{
    return (
        (_eventgroup.getServiceId() == eventgroup->getServiceId())
        && (_eventgroup.getMajorVersion() == eventgroup->getMajorVersion())
        && (_eventgroup.getInstanceId() == eventgroup->getInstanceId())
        && (_eventgroup.getEventgroupId() == eventgroup->getEventgroupId()));
}

RemoveExpiredTtlCondition::RemoveExpiredTtlCondition(
    EndpointPool& endpoints, uint32_t const ticks, internal::EventGroupPool& groupPool)
: _endpoints(endpoints), _ticks(ticks), _groupPool(groupPool)
{}

bool RemoveExpiredTtlCondition::operator()(SubscribedEventGroup* const eventgroup) const
{
    SubscriptionEndpointList& endpoints = eventgroup->getEndpoints();

    SubscriptionEndpointList::iterator endpoint = endpoints.begin();
    SubscriptionEndpointList::iterator prev     = endpoints.before_begin();

    while (endpoint != endpoints.end())
    {
        uint32_t const ticks = endpoint->ttl;

        if (ticks >= _ticks)
        {
            endpoint->ttl = ticks - _ticks;
            prev          = endpoint;
            ++endpoint;
        }
        else // expired
        {
            INFO_LOG(
                SOMEIP,
                "SubscriptionManager::subscriptionExpired(service: %d, version: %d, instance: %d, "
                "eventgroup: %d)",
                eventgroup->getServiceId(),
                eventgroup->getMajorVersion(),
                eventgroup->getInstanceId(),
                eventgroup->getEventgroupId());

            endpoint->clear();
            SubscriptionEndpointList::iterator const current = endpoint;
            endpoint                                         = endpoints.erase_after(prev);
            _endpoints.release(&(*current));
        }
    }

    if (endpoints.empty())
    {
        _groupPool.release(eventgroup);
        return true;
    }
    return false;
}

} // namespace internal
} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
