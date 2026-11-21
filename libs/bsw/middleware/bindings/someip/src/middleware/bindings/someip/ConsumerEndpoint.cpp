/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/bindings/someip/ConsumerEndpoint.h"

#include "ip/IPEndpoint.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/MemberBinding.h"
#include "middleware/bindings/someip/MemberTable.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/bindings/someip/PayloadTransfer.h"
#include "middleware/bindings/someip/SomeIpBridgeConfig.h"
#include "middleware/core/types.h"
#include "someip/INetwork.h"
#include "someip/IRpcSender.h"
#include "someip/IServiceListener.h"
#include "someip/ISomeIpSerializable.h"
#include "someip/ISomeIpStack.h"
#include "someip/RpcChannel.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceInfo.h"
#include "someip/ServiceQuery.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpParser.h"

#include <etl/vector.h>

#include <cstdint>

namespace middleware::bindings::someip::internal
{

using ::middleware::core::ErrorState;

ConsumerCall::ConsumerCall(
    ConsumerEndpoint& consumer, ::someip::INetwork& network, ::someip::IRpcSender& sender)
: _consumer(consumer), _channel(network, sender)
{}

::someip::ServiceResultCode ConsumerCall::operator()(::someip::ServiceResultCode const result)
{
    // Freed before the proxy is answered, which may already send the next request.
    ProxyRequest const request = _request;
    _isBusy                    = false;
    _consumer.callDone(*this, request, result);
    return result;
}

ConsumerEndpoint::ConsumerEndpoint(
    ISomeIpConnection& connection,
    ::someip::ISomeIpStack& stack,
    ::someip::INetwork& network,
    ::someip::IRpcSender& rpcSender,
    uint16_t const serviceId,
    SomeIpBridgeConfig const& config,
    MemberTable& members,
    ::etl::ivector<ConsumerCall>& calls)
: _connection(connection)
, _stack(stack)
, _network(network)
, _rpcSender(rpcSender)
, _serviceId(serviceId)
, _config(config)
, _members(members)
, _calls(calls)
{}

void ConsumerEndpoint::bindEvent(uint16_t const memberId, IPayloadSlot& payloadSlot)
{
    MemberBinding binding;
    binding.memberId    = memberId;
    binding.kind        = MemberBinding::Kind::Event;
    binding.instanceId  = _config.consumedInstanceId;
    binding.requestSlot = &payloadSlot;
    _members.add(binding);
}

void ConsumerEndpoint::bindMethod(
    uint16_t const memberId,
    MemberBinding::Kind const kind,
    IPayloadSlot* const requestSlot,
    IPayloadSlot* const responseSlot)
{
    MemberBinding binding;
    binding.memberId     = memberId;
    binding.kind         = kind;
    binding.instanceId   = _config.consumedInstanceId;
    binding.requestSlot  = requestSlot;
    binding.responseSlot = responseSlot;
    binding.methodDetail = MemberTable::methodDetailFor(memberId, kind);
    _members.add(binding);
}

bool ConsumerEndpoint::startQuerying()
{
    if (_isQuerying)
    {
        return true;
    }
    describeQuery(_serviceQuery, ::someip::eventgroup_id::ALL);
    describeQuery(_eventGroupQuery, _config.consumedEventGroupId);

    bool const isServiceRegistered    = _stack.registerServiceQuery(_serviceQuery);
    bool const isEventGroupRegistered = _stack.registerServiceQuery(_eventGroupQuery);
    if (!isServiceRegistered || !isEventGroupRegistered)
    {
        if (isServiceRegistered)
        {
            _stack.unregisterServiceQuery(_serviceQuery);
        }
        if (isEventGroupRegistered)
        {
            _stack.unregisterServiceQuery(_eventGroupQuery);
        }
        return false;
    }
    _stack.addEventListener(*this);
    _isQuerying = true;
    return true;
}

void ConsumerEndpoint::stopQuerying()
{
    if (!_isQuerying)
    {
        return;
    }
    _stack.unregisterServiceQuery(_eventGroupQuery);
    _stack.unregisterServiceQuery(_serviceQuery);
    _stack.removeEventListener(*this);
    _isQuerying = false;
}

void ConsumerEndpoint::restartStartupPhase()
{
    if (_isQuerying)
    {
        restartQueryPhase(_serviceQuery);
        restartQueryPhase(_eventGroupQuery);
    }
}

bool ConsumerEndpoint::isInstanceAvailable() const { return _isAvailable; }

bool ConsumerEndpoint::sendRequest(Message const& msg, MemberBinding const& binding)
{
    // Events of the consumed instance are received, never sent.
    if (binding.kind == MemberBinding::Kind::Event)
    {
        return false;
    }
    ConsumerCall* const call = idleCall();
    if (call == nullptr)
    {
        return false;
    }
    ::someip::RpcChannel& channel = call->channel();
    advanceSession(channel);

    Message::Header const& header                = msg.getHeader();
    ::someip::ISomeIpSerializable const* request = nullptr;
    if (binding.requestSlot != nullptr)
    {
        // The stack serializes before the call returns, as for a response.
        serializeInPlace(msg, *binding.requestSlot);
        request = &binding.requestSlot->serializable();
    }

    bool isSent = false;
    if (msg.isFireAndForgetRequest())
    {
        isSent
            = (channel.callFireAndForgetMethod(header.memberId, request, _config.majorVersion)
               == ::someip::RPC_SENT_SUCCESSFULLY_NO_RESPONSE_EXPECTED);
    }
    else
    {
        ProxyRequest proxyRequest;
        proxyRequest.binding      = &binding;
        proxyRequest.memberId     = header.memberId;
        proxyRequest.requestId    = header.requestId;
        proxyRequest.proxyCluster = header.srcClusterId;
        proxyRequest.peerCluster  = header.tgtClusterId;
        proxyRequest.addressId    = header.addressId;
        call->start(proxyRequest);

        ::someip::ISomeIpSerializable* const response
            = (binding.responseSlot != nullptr) ? &binding.responseSlot->serializable() : nullptr;
        isSent
            = (channel.callMethod(
                   header.memberId,
                   request,
                   _config.majorVersion,
                   response,
                   *call,
                   _config.requestTimeoutMs)
               == ::someip::RPC_SENT_SUCCESSFULLY);
        if (!isSent)
        {
            call->cancel();
        }
    }
    if (binding.requestSlot != nullptr)
    {
        binding.requestSlot->serializeFrom(nullptr);
    }
    return isSent;
}

void ConsumerEndpoint::serviceStatusChanged(
    ::someip::ServiceDescription const& service, ServiceStatus const status)
{
    if ((service.serviceId != _serviceId) || (service.instanceId != _config.consumedInstanceId))
    {
        return;
    }
    if (status == ServiceStatus::SERVICE_AVAILABLE)
    {
        openChannels(::ip::IPEndpoint(service.ipAddress, service.port));
    }
    else if (status == ServiceStatus::SERVICE_UNAVAILABLE)
    {
        closeChannels();
    }
}

void ConsumerEndpoint::updateEventgroupDescription(
    ::someip::ServiceDescription& /*service*/, ServiceStatus const /*status*/)
{}

::someip::MethodDetail const* ConsumerEndpoint::getMethodDetail(uint16_t const methodId) const
{
    MemberBinding const* const binding = _members.find(_config.consumedInstanceId, methodId);
    // Events never arrive as an RPC method call.
    if ((binding == nullptr) || (binding->kind == MemberBinding::Kind::Event))
    {
        return nullptr;
    }
    return &binding->methodDetail;
}

::someip::EventDetail const* ConsumerEndpoint::getEventDetail(uint16_t /*eventId*/) const
{
    return nullptr;
}

void ConsumerEndpoint::eventReceived(
    ::someip::service_id::type const serviceId,
    uint16_t const eventId,
    ::someip::instance_id::type const instanceId,
    ::someip::major_version::type const /*majorVersion*/,
    ::someip::SomeIpParser& parser)
{
    if ((serviceId != _serviceId) || (instanceId != _config.consumedInstanceId))
    {
        return;
    }
    MemberBinding const* const binding = _members.find(instanceId, eventId);
    if ((binding == nullptr) || (binding->requestSlot == nullptr))
    {
        return;
    }

    parser >> binding->requestSlot->serializable();
    if (!parser.isGood())
    {
        return;
    }

    Message msg
        = Message::createEvent(_serviceId, eventId, instanceId, _connection.getSourceClusterId());
    allocateAndDispatch(_connection, msg, *binding->requestSlot);
}

void ConsumerEndpoint::callDone(
    ConsumerCall& call, ProxyRequest const& request, ::someip::ServiceResultCode const result)
{
    if (!_isAvailable || !(call.channel().getRemoteIp() == _consumedEndpoint))
    {
        // The instance went away or moved while this call was out.
        reopenChannel(call);
    }

    ErrorState error = errorStateFor(result);
    if (result == ::someip::RPC_POSITIVE_RESPONSE)
    {
        Message response = Message::createResponse(
            _serviceId,
            request.memberId,
            request.requestId,
            _config.consumedInstanceId,
            request.peerCluster,
            request.proxyCluster,
            request.addressId);
        IPayloadSlot const* const slot = request.binding->responseSlot;
        if (slot == nullptr)
        {
            dispatchAndRelease(_connection, response, /*hasPayload=*/false);
            return;
        }
        if (loadPayloadFromSlot(response, *slot))
        {
            dispatchAndRelease(_connection, response, /*hasPayload=*/true);
            return;
        }
        error = ErrorState::QueueFullError; // allocator exhausted
    }

    Message errorResponse = Message::createErrorResponse(
        _serviceId,
        request.memberId,
        request.requestId,
        _config.consumedInstanceId,
        request.peerCluster,
        request.proxyCluster,
        request.addressId,
        error);
    dispatchAndRelease(_connection, errorResponse, /*hasPayload=*/false);
}

ErrorState ConsumerEndpoint::errorStateFor(::someip::ServiceResultCode const result)
{
    switch (result)
    {
        case ::someip::RPC_POSITIVE_RESPONSE:     return ErrorState::NoError;
        case ::someip::BUSY_ERROR:                return ErrorState::ServiceBusy;
        case ::someip::RPC_TIMEOUT:
        case ::someip::RPC_SERVICE_NOT_AVAILABLE:
        case ::someip::RPC_ERROR_NO_CHANNEL:      return ErrorState::ServiceNotFound;
        case ::someip::RPC_INVALID_PAYLOAD:       return ErrorState::DeserializationError;
        case ::someip::COULD_NOT_DELIVER:
        case ::someip::OUT_OF_MEMORY:             return ErrorState::QueueFullError;
        default:                                  return ErrorState::UserDefinedError;
    }
}

void ConsumerEndpoint::advanceSession(::someip::RpcChannel& channel)
{
    auto const next = static_cast<uint16_t>(channel.getSessionId() + 1U);
    channel.setSessionId((next == 0U) ? 1U : next);
}

void ConsumerEndpoint::openChannels(::ip::IPEndpoint const& endpoint)
{
    _consumedEndpoint = endpoint;
    _isAvailable      = true;
    while (!_calls.full())
    {
        _calls.emplace_back(*this, _network, _rpcSender);
        _calls.back().channel().setClientId(
            static_cast<uint16_t>(_config.clientId + (_calls.size() - 1U)));
    }
    for (auto& call : _calls)
    {
        if (!call.isBusy())
        {
            reopenChannel(call);
        }
    }
}

void ConsumerEndpoint::closeChannels()
{
    _isAvailable = false;
    for (auto& call : _calls)
    {
        if (!call.isBusy())
        {
            call.channel().close();
        }
    }
}

void ConsumerEndpoint::reopenChannel(ConsumerCall& call)
{
    if (!_isAvailable)
    {
        call.channel().close();
    }
    else if (_config.proto == ::someip::proto::SD_L4_PROTO_TCP)
    {
        call.channel().openTcp(_serviceId, _consumedEndpoint, _config.providerPort);
    }
    else
    {
        call.channel().openUdp(_serviceId, _consumedEndpoint, _config.providerPort);
    }
}

ConsumerCall* ConsumerEndpoint::idleCall()
{
    if (!_isAvailable)
    {
        return nullptr;
    }
    for (auto& call : _calls)
    {
        if (!call.isBusy() && call.channel().isOpen())
        {
            return &call;
        }
    }
    return nullptr;
}

void ConsumerEndpoint::describeQuery(
    ::someip::ServiceQuery& query, ::someip::eventgroup_id::type const eventGroupId)
{
    query.description.serviceId    = _serviceId;
    query.description.instanceId   = _config.consumedInstanceId;
    query.description.majorVersion = _config.majorVersion;
    query.description.minorVersion = ::someip::minor_version::ANY;
    query.description.eventGroup   = eventGroupId;
    query.description.port         = _config.providerPort;
    query.description.proto        = _config.proto;
    query.sdConfig                 = _config.findConfig;
    query.listener                 = this;
}

void ConsumerEndpoint::restartQueryPhase(::someip::ServiceQuery& query)
{
    query.state           = ::someip::ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE;
    query.timestamp       = 0U;
    query.repetitionCount = 0U;
}

} // namespace middleware::bindings::someip::internal
