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

#include "someip/BufferedEventSender.h"
#include "someip/EventTransceiver.h"
#include "someip/Network.h"
#include "someip/NetworkConfig.h"
#include "someip/RpcHandler.h"
#include "someip/RpcReceiver.h"
#include "someip/RpcServiceRegistry.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceTracker.h"
#include "someip/SomeIpStack.h"
#include "someip/SubscriptionManager.h"
#include "someip/TpTransceiver.h"

#include <async/Types.h>

#include <ip/IPAddress.h>

namespace someip
{
/**
 * The SOME/IP Stack with RPC only.
 */
class RpcSomeIpStack : public SomeIpStack
{
public:
    /* CLIENT */

    bool addRemoteService(ServiceDescription const& service);
    void removeRemoteService(ServiceDescription const& service);

    /* SERVER */

    bool addSubscription(ServiceDescription const& service);
    void removeSubscription(ServiceDescription const& service);

private:
    SubscriptionManager& _subscriptionManager;
    ServiceManager& _serviceManager;
    ServiceTracker& _serviceTracker;
    TpTransceiver& _tpTransceiver;
    BufferedEventSender& _eventSender;
    RpcReceiver& _rpcReceiver;

protected:
    RpcServiceRegistry _serviceRegistry;
    RpcSomeIpStack(
        async::ContextType& ethernetContext,
        NetworkConfig& networkConfig,
        SubscriptionManager& subscriptionManager,
        ServiceManager& serviceManager,
        ServiceTracker& serviceTracker,
        TpTransceiver& tpTransceiver,
        BufferedEventSender& eventSender,
        RpcReceiver& rpcReceiver,
        EventTransceiver& eventTransceiver,
        IDiagnosticListener const* diagnosticListener = nullptr);

    bool doInit() override;
    bool doStart() override;
    void doStop() override;
    void doShutdown() override;

    Network _network;
    EventTransceiver& _eventTransceiver;
    RpcHandler _rpcHandler;
};

namespace declare
{

/**
 * Declares a SOME/IP Stack with RPC only.
 */
template<
    uint16_t NumRemoteServices,
    uint16_t NumRemoteSubscriptions,
    uint16_t NumLocalServices,
    uint8_t NumSubscriptionEndpoints,
    uint8_t NumEventBuffers        = 0,
    uint8_t NumMulticastReceptions = 0>
class RpcSomeIpStack : public ::someip::RpcSomeIpStack
{
public:
    RpcSomeIpStack(async::ContextType& ethernetContext, NetworkConfig& networkConfig);

private:
    ::someip::declare::SubscriptionManager<NumRemoteSubscriptions> _subscriptionManager;
    ::someip::declare::ServiceManager<NumLocalServices> _serviceManager;
    ::someip::declare::ServiceTracker<NumRemoteServices> _serviceTracker;
    TpTransceiver _tpTransceiver;
    ::someip::declare::BufferedEventSender<NumEventBuffers> _eventSender;
    ::someip::declare::RpcReceiver<NumMulticastReceptions> _rpcReceiver;
    ::someip::declare::EventTransceiver<NumSubscriptionEndpoints> _eventTransceiver;
};

template<
    uint16_t NumRemoteServices,
    uint16_t NumRemoteSubscriptions,
    uint16_t NumLocalServices,
    uint8_t NumSubscriptionEndpoints,
    uint8_t NumEventBuffers,
    uint8_t NumMulticastReceptions>
inline RpcSomeIpStack<
    NumRemoteServices,
    NumRemoteSubscriptions,
    NumLocalServices,
    NumSubscriptionEndpoints,
    NumEventBuffers,
    NumMulticastReceptions>::
    RpcSomeIpStack(async::ContextType& ethernetContext, NetworkConfig& networkConfig)
: ::someip::RpcSomeIpStack(
    ethernetContext,
    networkConfig,
    _subscriptionManager,
    _serviceManager,
    _serviceTracker,
    _tpTransceiver,
    _eventSender,
    _rpcReceiver,
    _eventTransceiver)
, _subscriptionManager()
, _serviceManager()
, _serviceTracker()
, _tpTransceiver(
      ethernetContext, networkConfig._tpConfig.tpSenders, networkConfig._tpConfig.tpReceivers)
, _eventSender(_network, ethernetContext, _tpTransceiver)
, _rpcReceiver(_network, _tpTransceiver, _eventTransceiver, _serviceRegistry, _rpcHandler, nullptr)
, _eventTransceiver(_eventSender, _subscriptionManager)
{}

} // namespace declare
} // namespace someip
