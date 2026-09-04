/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdServiceRegistry.h"

#include "someip/IServiceAnnouncer.h"
#include "someip/ISubscriptionManager.h"
#include "someip/ServiceHandler.h"
#include "someip/ServiceQuery.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <ip/IPEndpoint.h>
#include <ip/to_str.h>

#include <cstdio>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPAddress;
using ::ip::IPEndpoint;
using ::util::logger::SOMEIP;

SdServiceRegistry::SdServiceRegistry(
    ISubscriptionManager& subscriptionManager,
    ::async::ContextType const ethernetContext,
    ServiceManager& serviceManager,
    ServiceTracker& serviceTracker,
    QueryManager& queryManager)
: _ethernetContext(ethernetContext)
, _cyclicFunction(
      ::async::Function::CallType::create<SdServiceRegistry, &SdServiceRegistry::cyclic>(*this))
, _cyclicTimeout()
, _subscriptionManager(subscriptionManager)
, _serviceManager(serviceManager)
, _serviceTracker(serviceTracker)
, _queryManager(queryManager)
{}

void SdServiceRegistry::init()
{
    _serviceTracker.init(*this);

    async::scheduleAtFixedRate(
        _ethernetContext,
        _cyclicFunction,
        _cyclicTimeout,
        CYCLE_TIME_MS,
        ::async::TimeUnit::MILLISECONDS);
}

void SdServiceRegistry::shutdown() { _cyclicTimeout.cancel(); }

// virtual
bool SdServiceRegistry::registerProvidedService(ProvidedService& service)
{
    return _serviceManager.registerService(service);
}

// virtual
void SdServiceRegistry::unregisterProvidedService(ProvidedService& service)
{
    (void)_serviceManager.unregisterService(service);

    if (containsEventGroup(service.description))
    {
        _subscriptionManager.removeSubscriptions(
            service.description.serviceId,
            service.description.majorVersion,
            service.description.instanceId,
            service.description.eventGroup);
    }
}

// virtual
bool SdServiceRegistry::registerServiceQuery(ServiceQuery& query)
{
    if (_queryManager.registerQuery(query))
    {
        if (!containsEventGroup(query.description))
        {
            _serviceTracker.notifyServices(query);
        }
        return true;
    }

    return false;
}

// virtual
void SdServiceRegistry::unregisterServiceQuery(ServiceQuery& query)
{
    (void)_queryManager.unregisterQuery(query);
}

// virtual
QueryManager const* SdServiceRegistry::getQueryManager() const { return &_queryManager; }

// virtual
instance_id::type SdServiceRegistry::getInstanceId(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    IPAddress const& ipAddress,
    uint16_t const port,
    bool remoteProvider) const
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = serviceId;
    service.majorVersion = majorVersion;
    service.ipAddress    = ipAddress;
    service.port         = port;

    if (remoteProvider)
    {
        return _serviceTracker.getInstanceId(const_cast<ServiceDescription const&>(service));
    }

    return _serviceManager.getInstanceId(const_cast<ServiceDescription const&>(service));
}

// virtual
void SdServiceRegistry::offerReceived(
    ServiceDescription const& receivedService, ::ip::IPAddress const& sourceAddress)
{
    if ((receivedService.ttl != 0U) && (receivedService.ttl != ttl::INVALID))
    {
        (void)_serviceTracker.addService(receivedService);
        _queryManager.offerReceived(receivedService, sourceAddress);
    }
    else
    {
        _serviceTracker.removeService(receivedService);
        _queryManager.stopOfferReceived(receivedService);
    }
}

// virtual
IServiceRegistry::SubscriptionResult SdServiceRegistry::subscribeReceived(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    eventgroup_id::type const eventGroup,
    ttl::type const ttl,
    IPAddress const& ipAddress,
    uint16_t const port,
    uint8_t const proto)
{
    if ((proto != proto::SD_L4_PROTO_UDP) && (proto != proto::SD_L4_PROTO_TCP))
    {
        return IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR;
    }

    auto subscribe         = ::someip::make<ServiceDescription>();
    subscribe.serviceId    = serviceId;
    subscribe.instanceId   = instanceId;
    subscribe.majorVersion = majorVersion;
    subscribe.eventGroup   = eventGroup;
    subscribe.ttl          = ttl;
    subscribe.ipAddress    = ipAddress;
    subscribe.port         = port;
    subscribe.proto        = proto;

    ProvidedService const* const service = _serviceManager.getEventGroup(subscribe);
    if (service == nullptr)
    {
        return (ttl != 0U) ? SdServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR
                           : IServiceRegistry::SubscriptionResult::UNSUBSCRIBE_ERROR;
    }

    if (ttl != 0U)
    {
        auto const result = _subscriptionManager.addSubscription(
            serviceId, majorVersion, instanceId, eventGroup, ttl, ipAddress, port);

        if (ISubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK == result)
        {
            ServiceHandler* const handler = service->getHandler();

            if (handler == nullptr)
            {
                WARN_LOG(
                    SOMEIP,
                    "ServiceRegistry::subscribeReceived(service: %d, version: %d, instance: %d, "
                    "eventgroup: %d) no handler",
                    serviceId,
                    majorVersion,
                    instanceId,
                    eventGroup);

                return SdServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR;
            }

            SubscriptionEndpointList* const subscriptionsList
                = _subscriptionManager.getSubscriptions(
                    SubscribedEventGroup(serviceId, majorVersion, instanceId, eventGroup));
            if ((subscriptionsList != nullptr) && (!subscriptionsList->empty())
                && (++subscriptionsList->begin() == subscriptionsList->end()))
            {
                handler->onEventGroupSubscriptionStateChanged(eventGroup, true);
            }

            bool const initialEventsNotified = handler->notifyInitialEvents(
                subscribe.serviceId,
                subscribe.instanceId,
                subscribe.majorVersion,
                subscribe.eventGroup,
                subscribe.ipAddress,
                subscribe.port,
                subscribe.proto);

            if (!initialEventsNotified)
            {
                // Initial event request could not be queued; respond with NACK
                _subscriptionManager.removeSubscription(
                    serviceId, majorVersion, instanceId, eventGroup, ipAddress, port);

                return IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR;
            }
        }

        if ((ISubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK == result)
            || (ISubscriptionManager::InternalSubscribeResult::INTERNAL_ALREADY_SUBSCRIBED
                == result))
        {
            ::ip::IPAddress const& ipAddr = service->description.ipAddress;

            if (::ip::isMulticastAddress(ipAddr) == true)
            {
                return IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK_MULTICAST;
            }

            return IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK;
        }

        return IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR;
    }

    _subscriptionManager.removeSubscription(
        serviceId, majorVersion, instanceId, eventGroup, ipAddress, port);

    ServiceHandler* const handler = service->getHandler();
    if (handler != nullptr)
    {
        SubscriptionEndpointList* const subscriptionsList = _subscriptionManager.getSubscriptions(
            SubscribedEventGroup(serviceId, majorVersion, instanceId, eventGroup));
        if ((subscriptionsList == nullptr) || (subscriptionsList->empty()))
        {
            handler->onEventGroupSubscriptionStateChanged(eventGroup, false);
        }
    }

    return IServiceRegistry::SubscriptionResult::UNSUBSCRIBE_OK;
}

// virtual
void SdServiceRegistry::subscribeAckReceived(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    IPEndpoint const& multicastEndpoint,
    ::ip::IPAddress const& sourceAddress)
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = serviceId;
    service.instanceId   = instanceId;
    service.eventGroup   = eventGroup;
    service.majorVersion = majorVersion;

    _queryManager.subscribeAckReceived(service, multicastEndpoint, sourceAddress);
}

// virtual
void SdServiceRegistry::subscribeNackReceived(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const /* eventGroup */,
    major_version::type const majorVersion,
    IPAddress const& sourceAddress)
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = serviceId;
    service.instanceId   = instanceId;
    service.majorVersion = majorVersion;

    _queryManager.subscribeNackReceived(service, sourceAddress);
    _serviceTracker.removeService(service);
}

// virtual
void SdServiceRegistry::rebootDetected(IPAddress const& ipAddress)
{
    char ipStr[::ip::MAX_IP_STRING_LENGTH];
    char* const ipInfo = ::ip::to_str(ipAddress, ipStr).data();
    INFO_LOG(SOMEIP, "ServiceRegistry::rebootDetected(ip: %s)", ipInfo);

    _subscriptionManager.removeSubscriptions(ipAddress);
    _serviceTracker.rebootDetected(ipAddress);
}

// virtual
bool SdServiceRegistry::interestedInService(
    service_id::type serviceId,
    instance_id::type instanceId,
    major_version::type majorVersion) const
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = serviceId;
    service.instanceId   = instanceId;
    service.majorVersion = majorVersion;

    return (
        _serviceManager.hasServiceDescription(service)
        || _queryManager.hasServiceDescription(service));
}

// virtual
bool SdServiceRegistry::isEventgroupPort(
    service_id::type serviceId,
    instance_id::type instanceId,
    major_version::type majorVersion,
    uint16_t port) const
{
    return _queryManager.isEventgroupPort(serviceId, instanceId, majorVersion, port);
}

// virtual
uint16_t SdServiceRegistry::getCurrentNumberOfSubscriptions() const
{
    return _subscriptionManager.getCurrentNumberOfSubscriptions();
}

// virtual
uint16_t SdServiceRegistry::getMaximumNumberOfSubscriptions() const
{
    return _subscriptionManager.getMaximumNumberOfSubscriptions();
}

// virtual
uint16_t SdServiceRegistry::getCurrentNumberOfProvidedServices() const
{
    return _serviceManager.getNumberOfServices();
}

uint16_t SdServiceRegistry::getMaximumNumberOfProvidedServices() const
{
    return _serviceManager.getMaxNumberOfServices();
}

// virtual
uint16_t SdServiceRegistry::getCurrentNumberOfRemoteServices() const
{
    return _serviceTracker.getCurrentNumberOfServices();
}

// virtual
uint16_t SdServiceRegistry::getMaximumNumberOfRemoteServices() const
{
    return _serviceTracker.getMaximumNumberOfServices();
}

// virtual
void SdServiceRegistry::cyclic()
{
    uint32_t const seconds = CYCLE_TIME_MS / 1000U;

    _serviceTracker.updateTTLs(seconds);
    _subscriptionManager.updateTTLs(seconds);
}

// virtual
void SdServiceRegistry::serviceTrackerChanged(
    ServiceDescription const& service, ServiceTrackerStatus const status)
{
    DEBUG_LOG(
        SOMEIP,
        "ServiceRegistry::serviceTrackerChanged(serviceId: %d, majorVersion: %d, instanceId: %d, "
        "status: %d)",
        service.serviceId,
        service.majorVersion,
        service.instanceId,
        status);

    if (IServiceTrackerListener::ServiceTrackerStatus::SERVICE_ADDED == status)
    {
        _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_AVAILABLE);
    }
    else if (IServiceTrackerListener::ServiceTrackerStatus::SERVICE_REMOVED == status)
    {
        _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);
    }
    else if (IServiceTrackerListener::ServiceTrackerStatus::SERVICE_CHANGED == status)
    {
        _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);
        _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_AVAILABLE);
    }
    else if (IServiceTrackerListener::ServiceTrackerStatus::SERVICE_RELIABLE == status)
    {
        _queryManager.updateQueries(service, IServiceListener::ServiceStatus::SERVICE_RELIABLE);
    }
    else
    {
        // status enum ends
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
