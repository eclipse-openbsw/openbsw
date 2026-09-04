/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdSomeIpStack.h"

#include "someip/EventTransceiver.h"
#include "someip/IServiceRegistry.h"
#include "someip/RpcHandler.h"
#include "someip/TcpClientChannelValidator.h"

namespace someip
{
using ref = detail::SdRefs;

SdSomeIpStack::SdSomeIpStack(
    async::ContextType& ethernetContext,
    Network& network,
    TcpClientChannelValidator& validator,
    SubscriptionManager& subscriptionManager,
    ServiceTracker& serviceTracker,
    QueryManager& queryManager,
    ServiceManager& serviceManager,
    SdServiceRegistry& serviceRegistry,
    SessionManager& sessionManager,
    RebootTracker& rebootTracker,
    TpTransceiver& tpTransceiver,
    BufferedEventSender& eventSender,
    RpcHandler& rpcHandler,
    EventTransceiver& eventTransceiver,
    RpcReceiver& rpcReceiver,
    ISdMessageParser::AdditionalSDCheck const additionalSDCheck)
: ref(
    network,
    validator,
    subscriptionManager,
    serviceTracker,
    queryManager,
    serviceManager,
    serviceRegistry,
    sessionManager,
    rebootTracker,
    tpTransceiver,
    eventSender,
    rpcHandler,
    eventTransceiver,
    rpcReceiver)
, SomeIpStack(
      ref::_network,
      ref::_rpcHandler,
      ref::_eventTransceiver,
      ref::_eventTransceiver,
      ref::_serviceRegistry)
, _serviceAnnouncer(
      ref::_network,
      ref::_serviceManager,
      ref::_serviceRegistry,
      ethernetContext,
      ref::_queryManager,
      ref::_sessionManager)
, _sdParser(
      ref::_serviceRegistry,
      _serviceAnnouncer,
      ref::_rebootTracker,
      ref::_network.getSubnetId(),
      ref::_network.getLocalIp(),
      additionalSDCheck)
, _sdReceiver(_sdParser)
{
    ref::_network.setSdListener(_sdReceiver);
}

// virtual
bool SdSomeIpStack::doInit()
{
    _queryManager.wire(&_serviceAnnouncer, &_rpcReceiver);
    _serviceManager.wire(&_serviceAnnouncer);

    ref::_serviceRegistry.init();
    _rpcReceiver.init();
    ref::_eventSender.init();
    _serviceAnnouncer.init();
    _sdParser.init();

    return true;
}

// virtual
bool SdSomeIpStack::doStart()
{
    if (!ref::_network.start())
    {
        return false;
    }

    ref::_queryManager.start();
    ref::_serviceManager.start();
    _serviceAnnouncer.start();

    return true;
}

// virtual
void SdSomeIpStack::doStop()
{
    ref::_serviceTracker.stop();
    ref::_queryManager.stop();
    ref::_serviceManager.stop();
    _serviceAnnouncer.stop();
    ref::_subscriptionManager.stop();
    ref::_tpTransceiver.stop();
    ref::_network.stop();
}

// virtual
void SdSomeIpStack::doShutdown()
{
    _serviceAnnouncer.shutdown();
    ref::_eventSender.shutdown();
    ref::_eventTransceiver.shutdown();
    ref::_rpcReceiver.shutdown();
    ref::_serviceRegistry.shutdown();
}

INetwork const& SdSomeIpStack::getNetwork() const { return ref::_network; }

INetwork& SdSomeIpStack::getNetwork() { return ref::_network; }

EventTransceiver const& SdSomeIpStack::getEventTransceiver() const
{
    return ref::_eventTransceiver;
}

EventTransceiver& SdSomeIpStack::getEventTransceiver() { return ref::_eventTransceiver; }

RpcHandler const& SdSomeIpStack::getRpcHandler() const { return ref::_rpcHandler; }

RpcHandler& SdSomeIpStack::getRpcHandler() { return ref::_rpcHandler; }

} // namespace someip
