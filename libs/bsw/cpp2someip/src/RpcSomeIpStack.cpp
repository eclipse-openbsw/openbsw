/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcSomeIpStack.h"

#include "someip/IDiagnosticListener.h"
#include "someip/NetworkConfig.h"

namespace someip
{
// 4127: These are not output parameters. We pass references to member variables
//       for initialization purposes.
RpcSomeIpStack::RpcSomeIpStack(
    async::ContextType& ethernetContext,
    NetworkConfig& networkConfig,
    SubscriptionManager& subscriptionManager,
    ServiceManager& serviceManager,
    ServiceTracker& serviceTracker,
    TpTransceiver& tpTransceiver,
    BufferedEventSender& eventSender,
    RpcReceiver& rpcReceiver,
    EventTransceiver& eventTransceiver,
    IDiagnosticListener const* const /*diagnosticListener*/)
: SomeIpStack(_network, _rpcHandler, eventTransceiver, eventTransceiver, _serviceRegistry)
, _subscriptionManager(subscriptionManager)
, _serviceManager(serviceManager)
, _serviceTracker(serviceTracker)
, _tpTransceiver(tpTransceiver)
, _eventSender(eventSender)
, _rpcReceiver(rpcReceiver)
, _serviceRegistry(_serviceManager, _serviceTracker)
, _network(networkConfig)
, _eventTransceiver(eventTransceiver)
, _rpcHandler(_network, ethernetContext, _tpTransceiver, _serviceManager, _serviceRegistry)
{}

bool RpcSomeIpStack::addRemoteService(ServiceDescription const& service)
{
    return _serviceTracker.addService(service);
}

void RpcSomeIpStack::removeRemoteService(ServiceDescription const& service)
{
    _serviceTracker.removeService(service);
}

bool RpcSomeIpStack::addSubscription(ServiceDescription const& service)
{
    return (
        ISubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR
        != _subscriptionManager.addSubscription(
            service.serviceId,
            service.majorVersion,
            service.instanceId,
            service.eventGroup,
            service.ttl,
            service.ipAddress,
            service.port));
}

void RpcSomeIpStack::removeSubscription(ServiceDescription const& service)
{
    _subscriptionManager.removeSubscription(
        service.serviceId,
        service.majorVersion,
        service.instanceId,
        service.eventGroup,
        service.ipAddress,
        service.port);
}

// virtual
bool RpcSomeIpStack::doInit()
{
    _serviceRegistry.init();
    _rpcReceiver.init();
    _eventSender.init();

    return true;
}

// virtual
void RpcSomeIpStack::doShutdown()
{
    _eventSender.shutdown();
    _eventTransceiver.shutdown();
    _rpcReceiver.shutdown();
}

// virtual
bool RpcSomeIpStack::doStart() { return _network.start(); }

// virtual
void RpcSomeIpStack::doStop()
{
    _subscriptionManager.stop();
    _tpTransceiver.stop();
    _network.stop();
}

} // namespace someip
