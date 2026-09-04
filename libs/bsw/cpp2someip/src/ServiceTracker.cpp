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

#include "bsp/timer/SystemTimer.h"
#include "someip/IServiceListener.h"
#include "someip/IServiceTrackerListener.h"
#include "someip/ServiceQuery.h"
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
ServiceTracker::ServiceTracker(ServiceList& serviceList)
: _services(serviceList), _pListener(nullptr), _reliabilityConfig{0, 0}
{}

ServiceTracker::ServiceTracker(
    ServiceList& serviceList, ServiceReliabilityConfig const& reliabilityConfig)
: _services(serviceList), _pListener(nullptr), _reliabilityConfig(reliabilityConfig)
{}

void ServiceTracker::init(IServiceTrackerListener& listener)
{
    _services.clear();
    _pListener = &listener;
}

bool ServiceTracker::addService(ServiceDescription const& service)
{
    ttl::type const ttl = service.ttl;
    if ((ttl == 0U) || (ttl == ttl::INVALID))
    {
        WARN_LOG(SOMEIP, "ServiceTracker:addService() invalid ttl");
        return false;
    }

    internal::FindServiceCondition const condition(service);
    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "ServiceTracker:addService() invalid condition");
        return false;
    }

    ServiceList::const_iterator const itr
        = ::etl::find_if(_services.begin(), _services.end(), condition);

    if (itr != _services.end()) // already contained
    {
        DEBUG_LOG(
            SOMEIP,
            "ServiceTracker::updateService(service: %d, version: %d, instance: %d, ttl: %d)",
            service.serviceId,
            service.majorVersion,
            service.instanceId,
            service.ttl);

        internal::TrackedService& entry = const_cast<internal::TrackedService&>(*itr);
        entry.serviceDescription.ttl    = service.ttl;

        if ((entry.serviceDescription.ipAddress != service.ipAddress)
            || (entry.serviceDescription.port != service.port)
            || (entry.serviceDescription.proto != service.proto))
        {
            entry.serviceDescription       = service;
            entry.lastOfferTimestamp       = getSystemTimeMs32Bit();
            entry.sequentialOffersReceived = 1U;

            if (_pListener != nullptr)
            {
                _pListener->serviceTrackerChanged(
                    service, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_CHANGED);
            }
        }
        else
        {
            if ((_reliabilityConfig.expectedOfferPeriodMs > 0)
                && (_reliabilityConfig.offerCountThreshold > 0))
            {
                uint64_t const now      = getSystemTimeMs32Bit();
                uint64_t const timeDiff = static_cast<uint64_t>(now - entry.lastOfferTimestamp);
                uint64_t const timeAccuracyMs = 999U;
                if (timeDiff <= (_reliabilityConfig.expectedOfferPeriodMs + timeAccuracyMs))
                {
                    if (entry.sequentialOffersReceived < _reliabilityConfig.offerCountThreshold)
                    {
                        ++(entry.sequentialOffersReceived);
                        if (entry.sequentialOffersReceived
                            == _reliabilityConfig.offerCountThreshold)
                        {
                            if (_pListener != nullptr)
                            {
                                _pListener->serviceTrackerChanged(
                                    service,
                                    IServiceTrackerListener::ServiceTrackerStatus::
                                        SERVICE_RELIABLE);
                            }
                            else
                            {
                                // no listener
                            }
                        }
                        else
                        {
                            // keep counting
                        }
                    }
                    else
                    {
                        // already reliable
                    }
                }
                else
                {
                    // if once reliable don't go back to unreliable, that transition is done when
                    // ttl is exceeded.
                    if (entry.sequentialOffersReceived < _reliabilityConfig.offerCountThreshold)
                    {
                        entry.sequentialOffersReceived = 1U;
                    }
                    else
                    {
                        // already reliable
                    }
                }
                entry.lastOfferTimestamp = now;
            }
            else
            {
                // reliable service feature is disabled
            }
        }
    }
    else if (_services.full()) // list full
    {
        WARN_LOG(SOMEIP, "ServiceTracker:addService() list full");
        return false;
    }
    else // add as new
    {
        INFO_LOG(
            SOMEIP,
            "ServiceTracker::addService(service: %d, version: %d, instance: %d, ttl: %d)",
            service.serviceId,
            service.majorVersion,
            service.instanceId,
            service.ttl);

        (void)_services.insert({service, 1U, getSystemTimeMs32Bit()});

        if (_pListener != nullptr)
        {
            _pListener->serviceTrackerChanged(
                service, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED);
        }
    }

    return true;
}

void ServiceTracker::removeService(ServiceDescription const& service)
{
    internal::FindServiceCondition const condition(service);
    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "ServiceTracker:removeService() invalid condition");
        return;
    }

    ServiceList::size_type const before = _services.size();

    auto it = _services.begin();
    while (it != _services.end())
    {
        if (condition(*it))
        {
            it = _services.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (_services.size() < before)
    {
        INFO_LOG(
            SOMEIP,
            "ServiceTracker::removeService(service: %d, version: %d, instance: %d)",
            service.serviceId,
            service.majorVersion,
            service.instanceId);

        if (_pListener != nullptr)
        {
            _pListener->serviceTrackerChanged(
                service, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED);
        }
    }
}

void ServiceTracker::stop() { _services.clear(); }

bool ServiceTracker::getService(ServiceDescription& service) const
{
    internal::FindServiceCondition const condition(service);
    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "ServiceTracker:getService() invalid condition");
        return false;
    }

    ServiceList::const_iterator const itr
        = ::etl::find_if(_services.begin(), _services.end(), condition);

    if (itr != _services.end())
    {
        service = itr->serviceDescription;
        return true;
    }

    return false;
}

instance_id::type ServiceTracker::getInstanceId(ServiceDescription const& service) const
{
    internal::FindInstanceCondition const condition(service);
    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "ServiceTracker:getInstanceId() invalid condition");
        return instance_id::ANY;
    }

    ServiceList::const_iterator const itr
        = ::etl::find_if(_services.begin(), _services.end(), condition);

    if (itr == _services.end())
    {
        return instance_id::ANY;
    }

    return itr->serviceDescription.instanceId;
}

void ServiceTracker::rebootDetected(IPAddress const& ipAddr)
{
    internal::RemoveIpCondition const condition(ipAddr, _pListener);
    auto it = _services.begin();
    while (it != _services.end())
    {
        if (condition(*it))
        {
            it = _services.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ServiceTracker::updateTTLs(uint32_t const ticks)
{
    internal::UpdateTTLCondition const condition(ticks, _pListener);
    auto it = _services.begin();
    while (it != _services.end())
    {
        if (condition(*it))
        {
            it = _services.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ServiceTracker::notifyServices(ServiceQuery& query) const
{
    if (query.listener == nullptr)
    {
        return;
    }

    internal::FindServiceCondition const condition(
        query.description, (query.description.instanceId == instance_id::ANY));
    if (!condition.isValid())
    {
        WARN_LOG(SOMEIP, "ServiceTracker:notifyServices() invalid condition");
        return;
    }

    ServiceList::const_iterator itr = ::etl::find_if(_services.begin(), _services.end(), condition);

    while ((itr != _services.end()) && (condition(*itr)))
    {
        query.listener->serviceStatusChanged(
            itr->serviceDescription, IServiceListener::ServiceStatus::SERVICE_AVAILABLE);
        ++itr;
    }
}

uint16_t ServiceTracker::getCurrentNumberOfServices() const
{
    return static_cast<uint16_t>(_services.size());
}

uint16_t ServiceTracker::getMaximumNumberOfServices() const
{
    return static_cast<uint16_t>(_services.max_size());
}

namespace internal
{
bool LessThanComparator::operator()(TrackedService const& lhs, TrackedService const& rhs) const
{
    return (
        (lhs.serviceDescription.serviceId < rhs.serviceDescription.serviceId)
        || (lhs.serviceDescription.majorVersion < rhs.serviceDescription.majorVersion)
        || (lhs.serviceDescription.instanceId < rhs.serviceDescription.instanceId));
}

FindServiceCondition::FindServiceCondition(
    ServiceDescription const& service, bool const instanceAny)
: _service(service), _instanceAny(instanceAny)
{}

bool FindServiceCondition::isValid() const
{
    return (
        (_service.serviceId != service_id::INVALID)
        && (_service.majorVersion != major_version::INVALID)
        && (_instanceAny || (_service.instanceId != instance_id::ANY)));
}

bool FindServiceCondition::operator()(TrackedService const& service) const
{
    return (
        (_service.serviceId == service.serviceDescription.serviceId)
        && (_service.majorVersion == service.serviceDescription.majorVersion)
        && (_instanceAny || (_service.instanceId == service.serviceDescription.instanceId)));
}

FindInstanceCondition::FindInstanceCondition(ServiceDescription const& service) : _service(service)
{}

bool FindInstanceCondition::isValid() const
{
    return (
        (_service.serviceId != service_id::INVALID)
        && (_service.majorVersion != major_version::INVALID)
        && (!::ip::isUnspecified(_service.ipAddress)) && (_service.port != port::INVALID));
}

bool FindInstanceCondition::operator()(TrackedService const& service) const
{
    return (
        (_service.serviceId == service.serviceDescription.serviceId)
        && (_service.majorVersion == service.serviceDescription.majorVersion)
        && (_service.ipAddress == service.serviceDescription.ipAddress));
}

RemoveIpCondition::RemoveIpCondition(
    IPAddress const& ipAddr, IServiceTrackerListener* const listener)
: _ip(ipAddr), _pListener(listener)
{}

bool RemoveIpCondition::operator()(TrackedService const& service) const
{
    if (service.serviceDescription.ipAddress != _ip)
    {
        return false;
    }

    if (_pListener != nullptr)
    {
        ServiceDescription temp(service.serviceDescription);
        temp.ttl = 0U;

        _pListener->serviceTrackerChanged(
            temp, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED);
    }

    return true;
}

UpdateTTLCondition::UpdateTTLCondition(
    uint32_t const ticks, IServiceTrackerListener* const listener)
: _ticks(ticks), _pListener(listener)
{}

bool UpdateTTLCondition::operator()(TrackedService const& service) const
{
    uint32_t const ticks = service.serviceDescription.ttl;
    if (ticks >= _ticks)
    {
        const_cast<ServiceDescription&>(service.serviceDescription).ttl = ticks - _ticks;
        return false;
    }

    INFO_LOG(
        SOMEIP,
        "ServiceTracker::serviceExpired(service: %d, version: %d, instance: %d)",
        service.serviceDescription.serviceId,
        service.serviceDescription.majorVersion,
        service.serviceDescription.instanceId);

    ServiceDescription temp(service.serviceDescription);
    temp.ttl = 0U;

    if (_pListener != nullptr)
    {
        _pListener->serviceTrackerChanged(
            temp, IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED);
    }

    return true;
}

} // namespace internal
} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
