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

#include "someip/IDiagnosticListener.h"
#include "someip/INetwork.h"
#include "someip/INetworkListener.h"
#include "someip/IRpcHandler.h"
#include "someip/IRpcReceiver.h"
#include "someip/ITpListener.h"
#include "someip/ITpTransceiver.h"
#include "someip/SomeIpMessage.h"

#include <ip/IPAddress.h>
#include <ip/IPEndpoint.h>

#include <etl/flat_set.h>
#include <etl/vector.h>
#include <cstdint>

namespace common
{
class ITimeoutManager2;
}

namespace someip
{

namespace internal
{
struct NetworkChannelComparator
{
    bool operator()(
        ::etl::optional<NetworkChannel> const& lhs,
        ::etl::optional<NetworkChannel> const& rhs) const;
};

class FindNetworkChannelCondition
{
public:
    FindNetworkChannelCondition(::ip::IPAddress const& ipAddr, uint16_t port);
    bool operator()(::etl::optional<NetworkChannel> const& channel) const;

private:
    ::ip::IPAddress const& _ip;
    uint16_t const _port;
};

} // namespace internal

class ITpTransceiver;
class IServiceRegistry;

class RpcReceiver
: public INetworkListener
, public ITpListener
, public IRpcReceiver
{
public:
    using MulticastReceptionList
        = ::etl::iflat_set<::etl::optional<NetworkChannel>, internal::NetworkChannelComparator>;

    RpcReceiver(
        INetwork& network,
        ITpTransceiver& tpTransceiver,
        IEventReceiver& eventReceiver,
        IServiceRegistry& serviceRegistry,
        IRpcHandler& rpcHandler,
        MulticastReceptionList& multicastReceptionList,
        IDiagnosticListener* diagnosticListener);

    void init();
    void shutdown();

    /** \see INetworkListener::received */
    void received(NetworkChannel& channel, uint32_t length) override;

    /** \see ITpListener::receivedTpMessage */
    void receivedTpMessage(NetworkChannel& channel, SomeIpMessage const& message) override;

    /** \see IRpcReceiver::setPriorityRpcHandler() */
    void setPriorityRpcHandler(IRpcHandler& priorityRpcHandler) override;

    /** \see IRpcReceiver::removePriorityRpcHandler() */
    void removePriorityRpcHandler() override;

    /** \see IRpcReceiver::requestMulticastReception() */
    bool requestMulticastReception(::ip::IPEndpoint const& multicastEndpoint) override;

    /** \see IRpcReceiver::cancelMulticastReception() */
    void cancelMulticastReception(::ip::IPEndpoint const& multicastEndpoint) override;

private:
    static bool isMagicCookie(SomeIpMessage const& message);

    void handleMessage(NetworkChannel& channel, SomeIpMessage const& message);

    void sendError(
        NetworkChannel& channel,
        SomeIpMessage const& message,
        SomeIpMessage::ReturnCode returnCode);

    static SomeIpMessage::ReturnCode getSomeipErrorCode(IRpcHandler::ErrorCode error);

    INetwork& _network;
    ITpTransceiver& _tpTransceiver;
    IEventReceiver& _eventReceiver;
    IDiagnosticListener* _pDiagnosticListener;
    IServiceRegistry& _serviceRegistry;
    IRpcHandler& _rpcHandler;
    IRpcHandler* _pPriorityRpcHandler;

    MulticastReceptionList& _multicastReceptions;
};

namespace declare
{

template<uint8_t NUM_MULTICAST_RECEPTIONS>
class RpcReceiver : public ::someip::RpcReceiver
{
public:
    RpcReceiver(
        INetwork& network,
        ITpTransceiver& tpTransceiver,
        IEventReceiver& eventReceiver,
        IServiceRegistry& serviceRegistry,
        IRpcHandler& rpcHandler,
        IDiagnosticListener* diagnosticListener);

private:
    ::etl::flat_set<
        ::etl::optional<NetworkChannel>,
        (NUM_MULTICAST_RECEPTIONS > 0U ? NUM_MULTICAST_RECEPTIONS : 1U),
        internal::NetworkChannelComparator>
        _multicastReceptionList;
};

template<uint8_t NUM_MULTICAST_RECEPTIONS>
inline RpcReceiver<NUM_MULTICAST_RECEPTIONS>::RpcReceiver(
    INetwork& network,
    ITpTransceiver& tpTransceiver,
    IEventReceiver& eventReceiver,
    IServiceRegistry& serviceRegistry,
    IRpcHandler& rpcHandler,
    IDiagnosticListener* const diagnosticListener)
: ::someip::RpcReceiver(
    network,
    tpTransceiver,
    eventReceiver,
    serviceRegistry,
    rpcHandler,
    _multicastReceptionList,
    diagnosticListener)
, _multicastReceptionList()
{}

} // namespace declare

} // namespace someip
