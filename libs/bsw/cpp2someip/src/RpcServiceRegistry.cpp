/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcServiceRegistry.h"

#include "someip/ServiceQuery.h"

namespace someip
{
RpcServiceRegistry::RpcServiceRegistry(
    ServiceManager& serviceManager, ServiceTracker& serviceTracker)
: _serviceManager(serviceManager), _serviceTracker(serviceTracker)
{}

void RpcServiceRegistry::init() { _serviceTracker.init(*this); }

void RpcServiceRegistry::shutdown() {}

// virtual
bool RpcServiceRegistry::registerProvidedService(ProvidedService& service)
{
    return _serviceManager.registerService(service);
}

// virtual
void RpcServiceRegistry::unregisterProvidedService(ProvidedService& service)
{
    (void)_serviceManager.unregisterService(service);
}

// virtual
bool RpcServiceRegistry::registerServiceQuery(ServiceQuery& /* query */)
{
    return false; // not used for rpc-mode
}

// virtual
void RpcServiceRegistry::unregisterServiceQuery(ServiceQuery& /* query */) {}

// virtual
QueryManager const* RpcServiceRegistry::getQueryManager() const { return nullptr; }

// virtual
instance_id::type RpcServiceRegistry::getInstanceId(
    uint16_t const serviceId,
    uint8_t const majorVersion,
    ::ip::IPAddress const& ipAddress,
    uint16_t const port,
    bool /*remoteProvider*/) const
{
    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = serviceId;
    service.majorVersion = majorVersion;
    service.ipAddress    = ipAddress;
    service.port         = port;

    return _serviceTracker.getInstanceId(service);
}

// virtual
void RpcServiceRegistry::offerReceived(
    ServiceDescription const& /*receivedService*/, ::ip::IPAddress const& /*sourceAddress*/)
{}

// virtual
IServiceRegistry::SubscriptionResult RpcServiceRegistry::subscribeReceived(
    uint16_t const /* serviceId */,
    uint16_t const /* instanceId */,
    uint8_t const /* majorVersion */,
    uint16_t const /* eventGroup */,
    uint32_t const /* ttl */,
    ::ip::IPAddress const& /* ipAddress */,
    uint16_t const /* port */,
    uint8_t const /* proto */)
{
    return IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK;
}

// virtual
void RpcServiceRegistry::subscribeAckReceived(
    uint16_t const /* serviceId */,
    uint16_t const /* instanceId */,
    uint16_t const /* eventGroup */,
    uint8_t const /* majorVersion */,
    ::ip::IPEndpoint const& /* multicastEndpoint */,
    ::ip::IPAddress const& /* sourceAddress */)
{}

// virtual
void RpcServiceRegistry::subscribeNackReceived(
    uint16_t const /* serviceId */,
    uint16_t const /* instanceId */,
    uint16_t const /* eventGroup */,
    uint8_t const /* majorVersion */,
    ::ip::IPAddress const& /* sourceAddress */)
{}

// virtual
void RpcServiceRegistry::rebootDetected(::ip::IPAddress const& /* ipAddress */) {}

// virtual
bool RpcServiceRegistry::interestedInService(
    service_id::type /* serviceId */,
    instance_id::type /* instanceId */,
    major_version::type /* majorVersion */) const
{
    return false;
}

// virtual
bool RpcServiceRegistry::isEventgroupPort(
    service_id::type /*serviceId*/,
    instance_id::type /*instanceId*/,
    major_version::type /*majorVersion*/,
    uint16_t /*port*/) const
{
    return true;
}

// virtual
uint16_t RpcServiceRegistry::getCurrentNumberOfSubscriptions() const { return 0; }

// virtual
uint16_t RpcServiceRegistry::getMaximumNumberOfSubscriptions() const { return 0; }

// virtual
uint16_t RpcServiceRegistry::getCurrentNumberOfProvidedServices() const
{
    return _serviceManager.getNumberOfServices();
}

// virtual
uint16_t RpcServiceRegistry::getMaximumNumberOfProvidedServices() const
{
    return _serviceManager.getMaxNumberOfServices();
}

// virtual
uint16_t RpcServiceRegistry::getCurrentNumberOfRemoteServices() const
{
    return _serviceTracker.getCurrentNumberOfServices();
}

// virtual
uint16_t RpcServiceRegistry::getMaximumNumberOfRemoteServices() const
{
    return _serviceTracker.getMaximumNumberOfServices();
}

} // namespace someip
