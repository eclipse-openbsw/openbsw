/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/bindings/someip/ProviderEndpoint.h"

#include "ip/IPAddress.h"
#include "ip/IPEndpoint.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/MemberBinding.h"
#include "middleware/bindings/someip/MemberTable.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/bindings/someip/PayloadTransfer.h"
#include "middleware/bindings/someip/SomeIpBridgeConfig.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"
#include "someip/IEventSender.h"
#include "someip/ISomeIpSerializable.h"
#include "someip/ISomeIpStack.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceHandler.h"
#include "someip/ServiceInfo.h"
#include "someip/SomeIpConstants.h"

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace middleware::bindings::someip::internal
{

using ::middleware::core::HRESULT;

ProviderEndpoint::ProviderEndpoint(
    ISomeIpConnection& connection,
    ::someip::ISomeIpStack& stack,
    ::someip::IEventSender& eventSender,
    uint16_t const serviceId,
    SomeIpBridgeConfig const& config,
    MemberTable& members,
    RpcCallbackPool& callbackPool,
    ::etl::span<::someip::CallDoneClosure*> const& pendingRequests)
: ::someip::ServiceHandler(callbackPool)
, _connection(connection)
, _stack(stack)
, _eventSender(eventSender)
, _serviceId(serviceId)
, _config(config)
, _members(members)
, _pendingRequests(pendingRequests)
{}

void ProviderEndpoint::bindEvent(
    uint16_t const memberId,
    IPayloadSlot& payloadSlot,
    ::etl::span<uint16_t const> const& eventGroupIds,
    bool const isMulticast)
{
    MemberBinding binding;
    binding.memberId      = memberId;
    binding.kind          = MemberBinding::Kind::Event;
    binding.instanceId    = _config.instanceId;
    binding.requestSlot   = &payloadSlot;
    binding.eventGroupIds = eventGroupIds;
    binding.isMulticast   = isMulticast;
    _members.add(binding);
}

void ProviderEndpoint::bindMethod(
    uint16_t const memberId,
    MemberBinding::Kind const kind,
    IPayloadSlot* const requestSlot,
    IPayloadSlot* const responseSlot)
{
    MemberBinding binding;
    binding.memberId     = memberId;
    binding.kind         = kind;
    binding.instanceId   = _config.instanceId;
    binding.requestSlot  = requestSlot;
    binding.responseSlot = responseSlot;
    binding.methodDetail = MemberTable::methodDetailFor(memberId, kind);
    _members.add(binding);
}

void ProviderEndpoint::offerService()
{
    _providedService.setHandler(*this);
    _providedService.description = makeDescription(_config.providerPort);
    _providedService.setSdConfig(_config.sdConfig);
    static_cast<void>(_stack.registerProvidedService(_providedService));
}

void ProviderEndpoint::restartStartupPhase() { _providedService.init(); }

bool ProviderEndpoint::sendEvent(Message const& msg, MemberBinding const& binding)
{
    if (binding.requestSlot == nullptr)
    {
        return false;
    }
    serializeInPlace(msg, *binding.requestSlot);

    // A multicast-flagged member goes to the group named in the Ack as one
    // datagram; everything else fans out to the subscribers directly.
    auto const result
        = (binding.isMulticast && (_config.eventMulticastPort != 0U))
              ? _eventSender.sendMulticastEvent(
                  _serviceId,
                  _config.majorVersion,
                  _config.instanceId,
                  binding.memberId,
                  binding.eventGroupIds,
                  /*maximumDelayTime=*/0U,
                  &binding.requestSlot->serializable(),
                  _config.providerPort,
                  _config.proto,
                  ::ip::IPEndpoint(_config.eventMulticastAddress, _config.eventMulticastPort))
              : _eventSender.sendEvent(
                  _serviceId,
                  _config.majorVersion,
                  _config.instanceId,
                  binding.memberId,
                  binding.eventGroupIds,
                  /*maximumDelayTime=*/0U,
                  &binding.requestSlot->serializable(),
                  _config.providerPort,
                  _config.proto);
    binding.requestSlot->serializeFrom(nullptr);
    return result == ::someip::IEventSender::ErrorCode::EVENT_SEND_OK;
}

bool ProviderEndpoint::sendResponse(Message const& msg, MemberBinding const& binding)
{
    ::someip::CallDoneClosure* const pending = findPendingRequest(msg.getHeader().requestId);
    if (pending == nullptr)
    {
        return false;
    }
    if (binding.responseSlot != nullptr)
    {
        serializeInPlace(msg, *binding.responseSlot);
    }
    (*pending)(::someip::RPC_POSITIVE_RESPONSE);
    if (binding.responseSlot != nullptr)
    {
        binding.responseSlot->serializeFrom(nullptr);
    }
    releasePendingRequest(msg.getHeader().requestId);
    return true;
}

::someip::ISomeIpSerializable* ProviderEndpoint::getRequest(uint16_t const methodId)
{
    MemberBinding const* const binding = _members.find(_config.instanceId, methodId);
    return (binding != nullptr) && (binding->requestSlot != nullptr)
               ? &binding->requestSlot->serializable()
               : nullptr;
}

::someip::ISomeIpSerializable* ProviderEndpoint::getResponse(uint16_t const methodId)
{
    MemberBinding const* const binding = _members.find(_config.instanceId, methodId);
    return (binding != nullptr) && (binding->responseSlot != nullptr)
               ? &binding->responseSlot->serializable()
               : nullptr;
}

::someip::MethodDetail const* ProviderEndpoint::getMethodDetail(uint16_t const methodId) const
{
    MemberBinding const* const binding = _members.find(_config.instanceId, methodId);
    // Events never arrive as an RPC method call.
    if ((binding == nullptr) || (binding->kind == MemberBinding::Kind::Event))
    {
        return nullptr;
    }
    return &binding->methodDetail;
}

bool ProviderEndpoint::notifyInitialEvents(
    ::someip::service_id::type /*serviceId*/,
    ::someip::instance_id::type /*instanceId*/,
    ::someip::major_version::type /*majorVersion*/,
    ::someip::eventgroup_id::type /*eventGroupId*/,
    ::ip::IPAddress const& /*address*/,
    ::someip::port::type /*port*/,
    ::someip::proto::type /*proto*/)
{
    return true;
}

void ProviderEndpoint::callMethod(
    uint16_t const methodId,
    ::someip::ISomeIpSerializable const* /*pRequest*/,
    ::someip::ISomeIpSerializable* const /*pResponse*/,
    ::someip::CallDoneClosure& done)
{
    MemberBinding const* const binding = _members.find(_config.instanceId, methodId);
    if (binding == nullptr)
    {
        done(::someip::RPC_METHOD_NOT_AVAILABLE);
        return;
    }

    if (binding->kind == MemberBinding::Kind::FireAndForget)
    {
        Message msg = Message::createFireAndForgetRequest(
            _serviceId,
            methodId,
            _config.instanceId,
            _connection.getTargetClusterId(),
            _connection.getSourceClusterId());
        // The skeleton reads its argument out of the Message, so the parsed
        // value has to be copied across. Referencing the slot is safe: this
        // dispatch completes before callMethod() returns.
        bool const hasPayload = (binding->requestSlot != nullptr);
        if (!hasPayload || loadPayloadFromSlot(msg, *binding->requestSlot))
        {
            dispatchAndRelease(_connection, msg, hasPayload);
        }
        done(::someip::RPC_SENT_SUCCESSFULLY_NO_RESPONSE_EXPECTED);
        return;
    }

    // Cluster ids are stamped from the receiver's point of view: a request off
    // the wire has src = remote, tgt = the local cluster hosting the skeleton.
    // Reversed, the response is unroutable, because the response's target is
    // echoed from the request's source.
    uint16_t const requestId = obtainPendingRequest(done);
    if (requestId == ::middleware::core::INVALID_REQUEST_ID)
    {
        done(::someip::RPC_UNDEFINED_ERROR); // request table exhausted
        return;
    }

    Message msg = Message::createRequest(
        _serviceId,
        methodId,
        requestId,
        _config.instanceId,
        _connection.getTargetClusterId(),
        _connection.getSourceClusterId(),
        /*addressId=*/0U);

    // The wire payload was parsed into the request slot, but the skeleton reads
    // its argument out of the Message, so it must be carried across or the
    // provider is handed a zeroed argument.
    bool const hasPayload = (binding->requestSlot != nullptr);
    if (hasPayload && !loadPayloadFromSlot(msg, *binding->requestSlot))
    {
        releasePendingRequest(requestId);
        done(::someip::RPC_UNDEFINED_ERROR); // payload allocator exhausted
        return;
    }

    HRESULT const result = _connection.dispatchMessage(msg);
    if (hasPayload)
    {
        ::middleware::core::MessagePayloadBuilder::deallocate(msg);
    }
    if (result != HRESULT::Ok)
    {
        releasePendingRequest(requestId);
        done(::someip::RPC_UNDEFINED_ERROR);
    }
    // Otherwise the response arrives later via write(), which releases this slot.
}

uint16_t ProviderEndpoint::obtainPendingRequest(::someip::CallDoneClosure& done)
{
    for (size_t i = 0U; i < _pendingRequests.size(); ++i)
    {
        if (_pendingRequests[i] == nullptr)
        {
            _pendingRequests[i] = &done;
            return static_cast<uint16_t>(i);
        }
    }
    return ::middleware::core::INVALID_REQUEST_ID;
}

::someip::CallDoneClosure* ProviderEndpoint::findPendingRequest(uint16_t const requestId) const
{
    return (requestId < _pendingRequests.size()) ? _pendingRequests[requestId] : nullptr;
}

void ProviderEndpoint::releasePendingRequest(uint16_t const requestId)
{
    if (requestId < _pendingRequests.size())
    {
        _pendingRequests[requestId] = nullptr;
    }
}

::someip::ServiceDescription ProviderEndpoint::makeDescription(uint16_t const port) const
{
    ::someip::ServiceDescription desc{};
    desc.serviceId    = _serviceId;
    desc.instanceId   = _config.instanceId;
    desc.majorVersion = _config.majorVersion;
    desc.minorVersion = _config.minorVersion;
    desc.eventGroup   = ::someip::eventgroup_id::ALL;
    desc.port         = port;
    desc.proto        = _config.proto;
    desc.ttl          = _config.ttl;
    return desc;
}

} // namespace middleware::bindings::someip::internal
