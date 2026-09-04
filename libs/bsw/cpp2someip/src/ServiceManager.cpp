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

#include "someip/IServiceAnnouncer.h"
#include "someip/ServiceAnnouncerTask.h"
#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <etl/algorithm.h>
#include <algorithm>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

ServiceManager::ServiceManager(ServiceList& serviceList)
: _services(serviceList), _pServiceAnnouncer(nullptr), _stopCalled(false)
{}

void ServiceManager::wire(IServiceAnnouncer* const serviceAnnouncer)
{
    _pServiceAnnouncer = serviceAnnouncer;
}

void ServiceManager::start()
{
    for (ProvidedService* const service : _services)
    {
        if (service != nullptr)
        {
            service->init();
        }
    }

    _stopCalled = false;
}

void ServiceManager::stop()
{
    for (ProvidedService* const service : _services)
    {
        if (service != nullptr)
        {
            if (containsEventGroup(service->description))
            {
                service->setState(ProvidedService::ProvidedServiceState::IDLE_PHASE);
            }
            else if (ProvidedService::ProvidedServiceState::REMOVAL_PHASE != service->getState())
            {
                service->setState(ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE);
            }
            else
            {
                // do nothing
            }
        }
    }

    _stopCalled = true;
}

bool ServiceManager::registerService(ProvidedService& service)
{
    if (hasService(service))
    {
        return false;
    }

    if ((service.description.serviceId == service_id::INVALID)
        || (service.description.majorVersion == major_version::INVALID)
        || (service.description.instanceId == instance_id::ANY)
        || (service.description.port == port::INVALID)
        || ((service.description.eventGroup == eventgroup_id::ALL)
            && (service.description.proto == SomeIpConstants::INVALID_PROTO))
        || (service.getHandler() == nullptr))
    {
        ERROR_LOG(
            SOMEIP,
            "ServiceManager::registerService(service: %d, eventgroup: %d) invalid service",
            service.description.serviceId,
            service.description.eventGroup);

        return false;
    }

    INFO_LOG(
        SOMEIP,
        "ServiceManager::registerService(service: %d, version: %d, instance: %d, eventgroup: %d)",
        service.description.serviceId,
        service.description.majorVersion,
        service.description.instanceId,
        service.description.eventGroup);

    (void)_services.insert(&service);

    service.init();

    return true;
}

bool ServiceManager::unregisterService(ProvidedService& service)
{
    if (!hasService(service))
    {
        return false;
    }

    INFO_LOG(
        SOMEIP,
        "ServiceManager::unregisterService(service: %d, version: %d, instance: %d, eventgroup: %d)",
        service.description.serviceId,
        service.description.majorVersion,
        service.description.instanceId,
        service.description.eventGroup);

    ServiceList::const_iterator itr = etl::find_if(
        _services.cbegin(),
        _services.cend(),
        [serviceId = service.description.serviceId](ProvidedService const* s)
        { return s && s->description.serviceId >= serviceId; });
    for (; itr != _services.end(); ++itr)
    {
        ProvidedService& entry = *(*itr);

        if (entry.description.serviceId != service.description.serviceId)
        {
            break;
        }

        if (&entry == &service)
        {
            service.setState(ProvidedService::ProvidedServiceState::REMOVAL_PHASE);

            if ((_pServiceAnnouncer == nullptr) ||         // no SD
                containsEventGroup(service.description) || // event-group
                _stopCalled                                // already stopped
            )
            {
                auto it = _services.find(&service);
                if (it != _services.end())
                {
                    _services.erase(it);
                }
                service.unregisterDone();
            }

            break;
        }
    }

    return true;
}

bool ServiceManager::hasService(ProvidedService& service) const
{
    return _services.contains(&service);
}

bool ServiceManager::hasServiceDescription(ServiceDescription const& service) const
{
    ServiceList::const_iterator itr = etl::find_if(
        _services.cbegin(),
        _services.cend(),
        [serviceId = service.serviceId](ProvidedService const* s)
        { return s && s->description.serviceId >= serviceId; });
    for (; itr != _services.cend(); ++itr)
    {
        if ((*itr)->description.serviceId != service.serviceId)
        {
            break;
        }

        if (isInstanceOf(service, (*itr)->description))
        {
            return true;
        }
    }
    return false;
}

void ServiceManager::updateServices(uint64_t const time)
{
    bool dirty = false;

    for (ProvidedService* const servicePtr : _services)
    {
        ProvidedService& service = *servicePtr;

        if (ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE == service.getState())
        {
            if (service.getTimestamp() == 0U)
            {
                service.setTimestamp(time); // start waiting
            }

            uint32_t const timePassed = static_cast<uint32_t>(time - service.getTimestamp());
            if (timePassed >= service.getSdConfig()._initialDelay)
            {
                if (_pServiceAnnouncer != nullptr)
                {
                    _pServiceAnnouncer->offer(service.description);
                }

                service.setState(ProvidedService::ProvidedServiceState::REPETITION_PHASE);
                service.setTimestamp(time);
                service.setRepetitionCount(1U);
            }
        }
        else if (ProvidedService::ProvidedServiceState::REPETITION_PHASE == service.getState())
        {
            uint32_t const timePassed = static_cast<uint32_t>(time - service.getTimestamp());
            uint32_t const repetitionDelay
                = (static_cast<uint32_t>(1U)
                   << (service.getRepetitionCount() - static_cast<uint32_t>(1U)))
                  * service.getSdConfig()._repetitionsBaseDelay;
            if (timePassed >= repetitionDelay)
            {
                if (_pServiceAnnouncer != nullptr)
                {
                    _pServiceAnnouncer->offer(service.description);
                }

                service.setTimestamp(time);
                service.setRepetitionCount(service.getRepetitionCount() + 1U);

                if (service.getRepetitionCount() >= service.getSdConfig()._repetitionsMax)
                {
                    // round to second which will make sure that the SOME/IP SD requirement
                    // of accumulating SD entries in as little frames as possible
                    service.setTimestamp((time / 1000) * 1000);
                    service.setState(ProvidedService::ProvidedServiceState::MAIN_PHASE);
                }
            }
        }
        else if (ProvidedService::ProvidedServiceState::MAIN_PHASE == service.getState())
        {
            uint32_t const timePassed = static_cast<uint32_t>(time - service.getTimestamp());
            if (timePassed >= service.getSdConfig()._cyclicOfferDelay)
            {
                if (_pServiceAnnouncer != nullptr)
                {
                    _pServiceAnnouncer->offer(service.description);
                }

                service.setTimestamp(time);
            }
        }
        else if (ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE == service.getState())
        {
            if (_pServiceAnnouncer != nullptr)
            {
                _pServiceAnnouncer->stopOffer(service.description);
            }

            service.setState(ProvidedService::ProvidedServiceState::IDLE_PHASE);
            service.setTimestamp(0U);
        }
        else if (ProvidedService::ProvidedServiceState::REMOVAL_PHASE == service.getState())
        {
            if (_pServiceAnnouncer != nullptr)
            {
                _pServiceAnnouncer->stopOffer(service.description);
            }

            dirty = true;
        }
    }

    if (dirty)
    {
        removeServices();
    }
}

// private
void ServiceManager::removeServices()
{
    ServiceList::const_iterator current = _services.begin();

    while (current != _services.end())
    {
        if (ProvidedService::ProvidedServiceState::REMOVAL_PHASE == (*current)->getState())
        {
            ProvidedService& tmp = *(*current);
            current              = _services.erase(current);
            tmp.unregisterDone();
        }
        else
        {
            ++current;
        }
    }
}

instance_id::type ServiceManager::getInstanceId(ServiceDescription const& service) const
{
    ServiceList::const_iterator itr = etl::find_if(
        _services.cbegin(),
        _services.cend(),
        [serviceId = service.serviceId](ProvidedService const* s)
        { return s && s->description.serviceId >= serviceId; });
    for (; itr != _services.cend(); ++itr)
    {
        ServiceDescription const& entry = (*itr)->description;

        // Once we encounter a different serviceId, we've passed the possible matches
        if (entry.serviceId != service.serviceId)
        {
            WARN_LOG(
                SOMEIP,
                "ServiceManager::getInstanceId() - Breaking loop: encountered different serviceId");
            break;
        }
        // Check if the majorVersion and port match as well
        if (entry.majorVersion == service.majorVersion && entry.port == service.port)
        {
            return entry.instanceId;
        }
    }

    // Return an invalid instance ID if not found
    return instance_id::ANY;
}

ProvidedService const* ServiceManager::getService(ServiceDescription const& service) const
{
    if ((service.serviceId == service_id::INVALID) || (service.instanceId == instance_id::ANY)
        || (service.majorVersion == major_version::INVALID))
    {
        WARN_LOG(SOMEIP, "ServiceManager::getService() invalid key");
        return nullptr;
    }

    ServiceList::const_iterator itr = etl::find_if(
        _services.cbegin(),
        _services.cend(),
        [serviceId = service.serviceId](ProvidedService const* s)
        { return s && s->description.serviceId >= serviceId; });
    for (; itr != _services.cend(); ++itr)
    {
        ServiceDescription const& entry = (*itr)->description;

        if (entry.serviceId != service.serviceId)
        {
            break;
        }

        if (entry.eventGroup != eventgroup_id::ALL)
        {
            continue;
        }
        if ((service.serviceId == entry.serviceId) && (service.instanceId == entry.instanceId)
            && (service.majorVersion == entry.majorVersion))
        {
            return *itr;
        }
    }

    return nullptr;
}

ProvidedService const* ServiceManager::getEventGroup(ServiceDescription const& service) const
{
    if ((service.serviceId == service_id::INVALID) || (service.instanceId == instance_id::ANY)
        || (service.majorVersion == major_version::INVALID)
        || (service.eventGroup == eventgroup_id::ALL))
    {
        WARN_LOG(SOMEIP, "ServiceManager::getEventGroup() invalid key");
        return nullptr;
    }

    ServiceList::const_iterator itr = etl::find_if(
        _services.cbegin(),
        _services.cend(),
        [serviceId = service.serviceId](ProvidedService const* s)
        { return s && s->description.serviceId >= serviceId; });
    for (; itr != _services.cend(); ++itr)
    {
        ServiceDescription const& entry = (*itr)->description;

        if (entry.serviceId != service.serviceId)
        {
            break;
        }

        if (entry.eventGroup == eventgroup_id::ALL)
        {
            continue;
        }
        if ((service.serviceId == entry.serviceId) && (service.instanceId == entry.instanceId)
            && (service.majorVersion == entry.majorVersion)
            && (service.eventGroup == entry.eventGroup))
        {
            return *itr;
        }
    }

    return nullptr;
}

ServiceHandler*
ServiceManager::getHandler(ServiceDescription const& service, FindServiceResult& result) const
{
    if ((service.serviceId == service_id::INVALID)
        || (service.majorVersion == major_version::INVALID) || (service.port == port::INVALID)
        || (service.proto == SomeIpConstants::INVALID_PROTO))
    {
        WARN_LOG(SOMEIP, "ServiceManager::getHandler() invalid key");

        result = FindServiceResult::FIND_SERVICE_UNKNOWN;
        return nullptr;
    }

    ServiceList::const_iterator itr = etl::find_if(
        _services.cbegin(),
        _services.cend(),
        [serviceId = service.serviceId](ProvidedService const* s)
        { return s && s->description.serviceId >= serviceId; });
    for (; itr != _services.cend(); ++itr)
    {
        ServiceDescription const& entry = (*itr)->description;

        if (entry.serviceId != service.serviceId)
        {
            break;
        }

        if (entry.eventGroup != eventgroup_id::ALL)
        {
            continue;
        }
        if ((service.serviceId == entry.serviceId) && (service.port == entry.port)
            && (service.proto == entry.proto))
        {
            if (service.majorVersion == entry.majorVersion)
            {
                result = FindServiceResult::FIND_SERVICE_OK;
                return (*itr)->getHandler();
            }

            result = FindServiceResult::FIND_SERVICE_WRONG_MAJOR_VERSION;
            return nullptr;
        }
    }

    result = FindServiceResult::FIND_SERVICE_UNKNOWN;
    return nullptr;
}

void ServiceManager::triggerOffers(ServiceAnnouncerTask const& task) const
{
    if (_pServiceAnnouncer == nullptr)
    {
        return;
    }

    for (ProvidedService* const service : _services)
    {
        if ((service != nullptr) && (task.isSame(service->description)))
        {
            _pServiceAnnouncer->offer(service->description);
        }
    }
}

void ServiceManager::triggerStopOffers()
{
    if (!_stopCalled)
    {
        WARN_LOG(SOMEIP, "ServiceManager::triggerStopOffers() force stop !");

        stop();
    }

    bool dirty = false;

    for (ProvidedService* const servicePtr : _services)
    {
        if (servicePtr != nullptr)
        {
            ProvidedService& service = *servicePtr;

            if ((ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE == service.getState())
                || (ProvidedService::ProvidedServiceState::REMOVAL_PHASE == service.getState()))
            {
                if (_pServiceAnnouncer != nullptr)
                {
                    _pServiceAnnouncer->stopOffer(service.description);
                }

                if (ProvidedService::ProvidedServiceState::DENOUNCEMENT_PHASE == service.getState())
                {
                    service.setState(ProvidedService::ProvidedServiceState::IDLE_PHASE);
                    service.setTimestamp(0U);
                }
                else
                {
                    dirty = true;
                }
            }
        }
    }

    if (dirty)
    {
        removeServices();
    }
}

uint16_t ServiceManager::getNumberOfServices() const
{
    return static_cast<uint16_t>(_services.size());
}

uint16_t ServiceManager::getMaxNumberOfServices() const
{
    return static_cast<uint16_t>(_services.max_size());
}

bool ServiceManager::LessThanComparator::operator()(
    ProvidedService const* const lhs, ProvidedService const* const rhs) const
{
    uint32_t const lhsServiceId = lhs->description.serviceId;
    uint32_t const rhsServiceId = rhs->description.serviceId;
    if (lhsServiceId != rhsServiceId)
    {
        return lhsServiceId < rhsServiceId;
    }

    uint32_t const lhsInstanceId = lhs->description.instanceId;
    uint32_t const rhsInstanceId = rhs->description.instanceId;
    if (lhsInstanceId != rhsInstanceId)
    {
        return lhsInstanceId < rhsInstanceId;
    }

    uint32_t const lhsEventGroup = lhs->description.eventGroup;
    uint32_t const rhsEventGroup = rhs->description.eventGroup;
    if (lhsEventGroup != rhsEventGroup)
    {
        return lhsEventGroup < rhsEventGroup;
    }

    uint32_t const lhsMajorVersion = lhs->description.majorVersion;
    uint32_t const rhsMajorVersion = rhs->description.majorVersion;
    if (lhsMajorVersion != rhsMajorVersion)
    {
        return lhsMajorVersion < rhsMajorVersion;
    }

    uint32_t const lhsPort = lhs->description.port;
    uint32_t const rhsPort = rhs->description.port;
    if (lhsPort != rhsPort)
    {
        return lhsPort < rhsPort;
    }

    uint32_t const lhsProto = lhs->description.proto;
    uint32_t const rhsProto = rhs->description.proto;
    return lhsProto < rhsProto;
}

bool ServiceManager::ServiceIdComparator::operator()(
    ProvidedService const* const lhs, uint16_t const rhs) const
{
    return lhs->description.serviceId < rhs;
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
