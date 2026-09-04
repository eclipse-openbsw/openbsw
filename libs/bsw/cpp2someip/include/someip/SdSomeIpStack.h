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
#include "someip/ISdMessageParser.h"
#include "someip/IServiceRegistry.h"
#include "someip/Network.h"
#include "someip/NetworkConfig.h"
#include "someip/QueryManager.h"
#include "someip/RebootTracker.h"
#include "someip/RpcHandler.h"
#include "someip/RpcReceiver.h"
#include "someip/SdMessageParser.h"
#include "someip/SdReceiver.h"
#include "someip/SdServiceRegistry.h"
#include "someip/ServiceAnnouncer.h"
#include "someip/ServiceManager.h"
#include "someip/ServiceTracker.h"
#include "someip/SessionManager.h"
#include "someip/SomeIpStack.h"
#include "someip/SubscriptionManager.h"
#include "someip/TcpClientChannelValidator.h"
#include "someip/TpTransceiver.h"

#include <ip/IPAddress.h>

#include <cstddef>

namespace someip
{

namespace detail
{

class SdRefs
{
protected:
    Network& _network;
    TcpClientChannelValidator& _validator;
    SubscriptionManager& _subscriptionManager;
    ServiceTracker& _serviceTracker;
    QueryManager& _queryManager;
    ServiceManager& _serviceManager;
    SdServiceRegistry& _serviceRegistry;
    SessionManager& _sessionManager;
    RebootTracker& _rebootTracker;
    TpTransceiver& _tpTransceiver;
    BufferedEventSender& _eventSender;
    RpcHandler& _rpcHandler;
    EventTransceiver& _eventTransceiver;
    RpcReceiver& _rpcReceiver;

    SdRefs(
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
        RpcReceiver& rpcReceiver)
    : _network(network)
    , _validator(validator)
    , _subscriptionManager(subscriptionManager)
    , _serviceTracker(serviceTracker)
    , _queryManager(queryManager)
    , _serviceManager(serviceManager)
    , _serviceRegistry(serviceRegistry)
    , _sessionManager(sessionManager)
    , _rebootTracker(rebootTracker)
    , _tpTransceiver(tpTransceiver)
    , _eventSender(eventSender)
    , _rpcHandler(rpcHandler)
    , _eventTransceiver(eventTransceiver)
    , _rpcReceiver(rpcReceiver)
    {}
};

} // namespace detail

/**
 * The SOME/IP Stack with SD.
 */

class SdSomeIpStack
: protected detail::SdRefs
, public SomeIpStack
{
public:
    SdSomeIpStack(
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
        ISdMessageParser::AdditionalSDCheck additionalSDCheck = {});

    bool doInit() override;
    bool doStart() override;
    void doStop() override;
    void doShutdown() override;

    INetwork const& getNetwork() const;
    INetwork& getNetwork();

    EventTransceiver const& getEventTransceiver() const;
    EventTransceiver& getEventTransceiver();

    RpcHandler const& getRpcHandler() const;
    RpcHandler& getRpcHandler();

private:
    ServiceAnnouncer _serviceAnnouncer;
    SdMessageParser _sdParser;
    SdReceiver _sdReceiver;
};

namespace declare
{

namespace detail
{

/*
 * Base-from-member idiom.
 */
template<
    typename UdpSocketType,
    uint8_t NumUdpRpcSockets,
    typename TcpServerSocketType,
    uint8_t NumTcpServerSockets,
    typename TcpSocketType,
    uint8_t NumTcpRpcSockets,
    size_t BufferSize,
    uint8_t NumTpStreams,
    bool SdEndpointOption,
    size_t InternalTcpReassembleBufferSize,
    uint16_t NumRemoteEndpoints,
    uint16_t NumRemoteServices,
    uint16_t NumRemoteSubscriptions,
    uint16_t NumLocalServices,
    uint16_t NumLocalQueries,
    uint8_t NumSubscriptionEndpoints,
    uint8_t NumEventBuffers,
    uint8_t NumMulticastReceptions>
class SdMembers
{
protected:
    SdMembers(
        ::ip::IPAddress const& multicastIp,
        ::ip::IPAddress const& localIp,
        uint8_t const subnet,
        async::ContextType& ethernetContext)
    : _rebootTracker()
    , _sessionManager()
    , _networkConfig(multicastIp, localIp, subnet)
    , _tpTransceiver(
          ethernetContext, _networkConfig._tpConfig.tpSenders, _networkConfig._tpConfig.tpReceivers)
    , _network(_networkConfig)
    , _validator(_network, _networkConfig._useMagicCookie)
    , _queryManager(_validator)
    , _eventSender(_network, ethernetContext, _tpTransceiver)
    , _subscriptionManager()
    , _eventTransceiver(_eventSender, _subscriptionManager)
    , _serviceTracker()
    , _serviceRegistry(
          _subscriptionManager, ethernetContext, _serviceManager, _serviceTracker, _queryManager)
    , _serviceManager()
    , _rpcHandler(_network, ethernetContext, _tpTransceiver, _serviceManager, _serviceRegistry)
    , _rpcReceiver(
          _network, _tpTransceiver, _eventTransceiver, _serviceRegistry, _rpcHandler, nullptr)
    {}

    /*
     * Tracks reboots from remote ECUs by monitoring session IDs.
     * According to spec we need to react to such reboots by re-establishing a new TCP connection
     */
    RebootTracker<NumRemoteEndpoints> _rebootTracker;

    SessionManager<NumRemoteEndpoints> _sessionManager;

    SdNetworkConfig<
        UdpSocketType,
        NumUdpRpcSockets,
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpSocketType,
        NumTcpRpcSockets,
        BufferSize>
        _networkConfig;

    // SOME/IP-TP
    TpTransceiver _tpTransceiver;

    Network _network;

    // To NOT miss initial events after subscription.
    // Validator checks if channel is established before sub/unsub-ing.
    TcpClientChannelValidator _validator;

    // Managing offers on client side
    QueryManager<NumLocalQueries> _queryManager;

    BufferedEventSender<NumEventBuffers> _eventSender;

    SubscriptionManager<NumRemoteSubscriptions> _subscriptionManager;
    EventTransceiver<NumSubscriptionEndpoints> _eventTransceiver;
    ServiceTracker<NumLocalServices> _serviceTracker;

    // holds all services known by their offers
    SdServiceRegistry _serviceRegistry;

    ServiceManager<NumLocalServices> _serviceManager;

    // Handles "fire & forget"s and requests
    RpcHandler _rpcHandler;

    RpcReceiver<NumMulticastReceptions> _rpcReceiver;
};

} // namespace detail

/**
 * Declares a SOME/IP Stack with SD.
 */
template<
    typename UdpSocketType,
    uint8_t NumUdpRpcSockets,
    typename TcpServerSocketType,
    uint8_t NumTcpServerSockets,
    typename TcpSocketType,
    uint8_t NumTcpRpcSockets,
    size_t BufferSize,
    uint8_t NumTpStreams,
    bool SdEndpointOption,
    size_t InternalTcpReassembleBufferSize,
    uint16_t NumRemoteEndpoints,
    uint16_t NumRemoteServices,
    uint16_t NumRemoteSubscriptions,
    uint16_t NumLocalServices,
    uint16_t NumLocalQueries,
    uint8_t NumSubscriptionEndpoints,
    uint8_t NumEventBuffers        = 0,
    uint8_t NumMulticastReceptions = 0>
class SdSomeIpStack
: protected detail::SdMembers<
      UdpSocketType,
      NumUdpRpcSockets,
      TcpServerSocketType,
      NumTcpServerSockets,
      TcpSocketType,
      NumTcpRpcSockets,
      BufferSize,
      NumTpStreams,
      SdEndpointOption,
      InternalTcpReassembleBufferSize,
      NumRemoteEndpoints,
      NumRemoteServices,
      NumRemoteSubscriptions,
      NumLocalServices,
      NumLocalQueries,
      NumSubscriptionEndpoints,
      NumEventBuffers,
      NumMulticastReceptions>
, public ::someip::SdSomeIpStack
{
    using mem = detail::SdMembers<
        UdpSocketType,
        NumUdpRpcSockets,
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpSocketType,
        NumTcpRpcSockets,
        BufferSize,
        NumTpStreams,
        SdEndpointOption,
        InternalTcpReassembleBufferSize,
        NumRemoteEndpoints,
        NumRemoteServices,
        NumRemoteSubscriptions,
        NumLocalServices,
        NumLocalQueries,
        NumSubscriptionEndpoints,
        NumEventBuffers,
        NumMulticastReceptions>;

public:
    explicit SdSomeIpStack(
        ::ip::IPAddress const& multicastIp,
        ::ip::IPAddress const& localIp,
        uint8_t const subnet,
        async::ContextType& ethernetContext)
    : mem(multicastIp, localIp, subnet, ethernetContext)
    , ::someip::SdSomeIpStack(
          ethernetContext,
          mem::_network,
          mem::_validator,
          mem::_subscriptionManager,
          mem::_serviceTracker,
          mem::_queryManager,
          mem::_serviceManager,
          mem::_serviceRegistry,
          mem::_sessionManager,
          mem::_rebootTracker,
          mem::_tpTransceiver,
          mem::_eventSender,
          mem::_rpcHandler,
          mem::_eventTransceiver,
          mem::_rpcReceiver)
    {}
};

} // namespace declare
} // namespace someip
