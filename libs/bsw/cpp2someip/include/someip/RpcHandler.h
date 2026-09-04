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

#include "someip/IRpcChannel.h"
#include "someip/IRpcHandler.h"
#include "someip/IRpcSender.h"
#include "someip/ServiceManager.h"
#include "someip/SomeIpMessage.h"

#include <async/Types.h>

#include <ip/IPAddress.h>

#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>
#include <cstdint>

namespace common
{
class ITimeoutManager2;
}

namespace someip
{
class ITpTransceiver;
class IServiceRegistry;
class RequestContext;
class INetwork;
class NetworkChannel;

class RpcHandler
: public IRpcHandler
, public IRpcSender
{
public:
    RpcHandler(
        INetwork& network,
        ::async::ContextType const ethernetContext,
        ITpTransceiver& tpTransceiver,
        ServiceManager& serviceManager,
        IServiceRegistry& serviceRegistry);

    /** \see IRpcHandler::handleMessage() */
    IRpcHandler::ErrorCode
    handleMessage(NetworkChannel const& channel, SomeIpMessage const& message) override;

    /** \see IRpcHandler::handleRequest() */
    IRpcHandler::ErrorCode handleRequest(
        SomeIpMessage const& message,
        ::ip::IPEndpoint const& sourceAddress,
        uint16_t localPort,
        uint8_t proto) override;

    /** \see IRpcHandler::handleResponse() */
    IRpcHandler::ErrorCode handleResponse(
        SomeIpMessage const& message,
        ::ip::IPEndpoint const& sourceAddress,
        uint16_t localPort) override;

    /** \see IRpcHandler::handleNotification() */
    void handleNotification(
        SomeIpMessage const& message,
        ::ip::IPEndpoint const& sourceAddress,
        uint16_t localPort) override;

    /** \see IRpcHandler::handleError() */
    IRpcHandler::ErrorCode
    handleError(SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress) override;

    /** \see IRpcHandler::setEventReceiver() */
    void setEventReceiver(IEventReceiver& eventReceiver) override;

    /** \see IRpcHandler::removeEventReceiver() */
    void removeEventReceiver() override;

    /** \see IRpcSender::sendRequest() */
    ServiceResultCode sendRequest(
        ISomeIpSerializable const* pRequest,
        service_id::type serviceId,
        uint16_t methodId,
        uint8_t interfaceVersion,
        bool isResponseExpected,
        IRpcChannel& channel,
        uint32_t timeout) override;

    /** \see IRpcSender::requestExpired() */
    void requestExpired(IRpcChannel& channel) override;

    size_t getNumRegisteredChannels() const;

private:
    ServiceResultCode requestDone(RequestContext& context, ServiceResultCode result);

    ServiceResultCode sendError(
        uint32_t requestId,
        service_id::type serviceId,
        uint16_t methodId,
        uint8_t interfaceVersion,
        uint8_t returnCode,
        uint16_t localPort,
        uint8_t proto,
        ::ip::IPEndpoint const& remoteIp) const;

    void registerChannel(IRpcChannel& channel);
    void unregisterChannel(IRpcChannel& channel);
    IRpcChannel* findChannel(
        service_id::type serviceId,
        uint16_t clientId,
        ::ip::IPEndpoint const& remoteIp,
        uint16_t sessionId);

    static ServiceResultCode getRpcErrorCode(SomeIpMessage::ReturnCode error);
    static SomeIpMessage::ReturnCode getSomeipErrorCode(ServiceResultCode error);

    using RpcChannelList = ::etl::intrusive_forward_list<IRpcChannel, ::etl::forward_link<0>>;
    RpcChannelList _rpcChannelList;

    INetwork& _network;
    ::async::ContextType const _ethernetContext;
    ITpTransceiver& _tpTransceiver;
    ServiceManager& _serviceManager;
    IServiceRegistry& _serviceRegistry;

    IEventReceiver* _pEventReceiver;
};

} // namespace someip
