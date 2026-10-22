/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/bindings/someip/ConsumerEndpoint.h"
#include "middleware/bindings/someip/IServiceEndpoint.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/MemberBinding.h"
#include "middleware/bindings/someip/MemberTable.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/bindings/someip/ProviderEndpoint.h"
#include "middleware/bindings/someip/SomeIpBridgeConfig.h"
#include "middleware/core/Message.h"
#include "someip/IEventSender.h"
#include "someip/INetwork.h"
#include "someip/IRpcSender.h"
#include "someip/ISomeIpStack.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceHandler.h"

#include <etl/array.h>
#include <etl/pool.h>
#include <etl/span.h>
#include <etl/vector.h>

#include <cstdint>

namespace middleware::bindings::someip
{

/**
 * Carries one service over SOME/IP: one provided instance, and optionally one
 * consumed instance. For the provided instance it answers requests and sends
 * events; for the consumed one it sends the proxy's requests and delivers the
 * responses and events back to it. It adds itself to the SomeIpConnection it is
 * constructed with, which may carry other services as well.
 *
 * The work is done by a provider part and a consumer part that are not templates;
 * this class sizes their storage.
 *
 * \tparam MaxMembers  capacity of the dispatch table; size it to what is bound.
 * \tparam MaxPending  how many requests may be in flight in each direction: from
 *                     the wire to the skeleton, and from the proxy to the wire.
 */
template<uint8_t MaxMembers, uint8_t MaxPending>
class SomeIpServiceEndpoint final : public internal::IServiceEndpoint
{
public:
    using Message = ::middleware::core::Message;

    /**
     * \param connection The connection that carries the service; the endpoint adds
     *                   itself to it.
     * \param network    The stack's network, which the request channels to the
     *                   consumed instance are opened on.
     * \param rpcSender  The stack's RPC handler, which sends those requests and
     *                   matches their responses.
     */
    SomeIpServiceEndpoint(
        ISomeIpConnection& connection,
        ::someip::ISomeIpStack& stack,
        ::someip::IEventSender& eventSender,
        ::someip::INetwork& network,
        ::someip::IRpcSender& rpcSender,
        uint16_t const serviceId,
        SomeIpBridgeConfig const& config)
    : _serviceId(serviceId)
    , _config(config)
    , _members(_bindings)
    , _provider(
          connection,
          stack,
          eventSender,
          serviceId,
          _config,
          _members,
          _rpcCallbackPool,
          ::etl::span<::someip::CallDoneClosure*>(_pendingRequests.data(), _pendingRequests.size()))
    , _consumer(connection, stack, network, rpcSender, serviceId, _config, _members, _consumerCalls)
    {
        connection.add(*this);
    }

    SomeIpServiceEndpoint(SomeIpServiceEndpoint const&)            = delete;
    SomeIpServiceEndpoint& operator=(SomeIpServiceEndpoint const&) = delete;

    /**
     * Registers a notification-only member: an event, or a field's notifier.
     * A field's getter and setter are methods on the wire and use bindMethod().
     * Called once per member, before the stack is started.
     */
    void bindEvent(
        uint16_t const memberId,
        internal::IPayloadSlot& payloadSlot,
        ::etl::span<uint16_t const> const eventGroupIds = {},
        bool const isMulticast                          = false)
    {
        _provider.bindEvent(memberId, payloadSlot, eventGroupIds, isMulticast);
    }

    /**
     * Registers a method, or a field's getter or setter.
     * Called once per member, before the stack is started.
     */
    void bindMethod(
        uint16_t const memberId,
        MemberBinding::Kind const kind,
        internal::IPayloadSlot* const requestSlot,
        internal::IPayloadSlot* const responseSlot)
    {
        _provider.bindMethod(memberId, kind, requestSlot, responseSlot);
    }

    /** Registers an event of the consumed instance, delivered to its proxy. */
    void bindConsumedEvent(uint16_t const memberId, internal::IPayloadSlot& payloadSlot)
    {
        _consumer.bindEvent(memberId, payloadSlot);
    }

    /**
     * Registers a method, or a field's getter or setter, of the consumed
     * instance: its proxy's calls go to the wire, and the responses back to it.
     * The slots serialize the request and parse the response.
     */
    void bindConsumedMethod(
        uint16_t const memberId,
        MemberBinding::Kind const kind,
        internal::IPayloadSlot* const requestSlot,
        internal::IPayloadSlot* const responseSlot)
    {
        _consumer.bindMethod(memberId, kind, requestSlot, responseSlot);
    }

    /**
     * Registers the ProvidedService, starting SD OfferService on the wire.
     * Called once, after every bindEvent() and bindMethod().
     */
    void offerService() { _provider.offerService(); }

    /**
     * Puts the offered service, and the find for the consumed instance if one is
     * running, back into the SD startup phase.
     */
    void restartStartupPhase()
    {
        _provider.restartStartupPhase();
        _consumer.restartStartupPhase();
    }

    /**
     * Whether SD has found the consumed instance, so a request to it can be sent.
     * Until then, a proxy's request fails at once.
     */
    bool isConsumedInstanceAvailable() const { return _consumer.isInstanceAvailable(); }

    /** The provided instance's part: the stack's handler for its calls and eventgroups. */
    internal::ProviderEndpoint& provider() { return _provider; }

    /** The consumed instance's part: the stack's listener for its discovery and events. */
    internal::ConsumerEndpoint& consumer() { return _consumer; }

    /** \see IServiceEndpoint::getServiceId() */
    uint16_t getServiceId() const override { return _serviceId; }

    /** \see IServiceEndpoint::getProvidedInstanceId() */
    uint16_t getProvidedInstanceId() const override { return _config.instanceId; }

    /** \see IServiceEndpoint::getConsumedInstanceId() */
    uint16_t getConsumedInstanceId() const override { return _config.consumedInstanceId; }

    /**
     * Sends one middleware message of this service to the wire. The connection
     * frees the payload afterwards.
     */
    bool send(Message const& msg) override
    {
        MemberBinding const* const binding
            = _members.find(msg.getHeader().serviceInstanceId, msg.getHeader().memberId);
        if (binding == nullptr)
        {
            return false;
        }

        if (msg.isEvent())
        {
            return _provider.sendEvent(msg, *binding);
        }
        if (msg.isResponse())
        {
            return _provider.sendResponse(msg, *binding);
        }
        if (msg.isRequest() || msg.isFireAndForgetRequest())
        {
            return _consumer.sendRequest(msg, *binding);
        }
        return false;
    }

    /** \see IServiceEndpoint::startQuerying() */
    bool startQuerying() override { return _consumer.startQuerying(); }

    /** \see IServiceEndpoint::stopQuerying() */
    void stopQuerying() override { _consumer.stopQuerying(); }

private:
    uint16_t _serviceId;
    SomeIpBridgeConfig _config;

    ::etl::vector<MemberBinding, MaxMembers> _bindings{};
    ::etl::pool<::someip::ServiceHandler::RpcCallback, MaxPending> _rpcCallbackPool{};
    ::etl::array<::someip::CallDoneClosure*, MaxPending> _pendingRequests{};
    ::etl::vector<internal::ConsumerCall, MaxPending> _consumerCalls{};

    internal::MemberTable _members;
    internal::ProviderEndpoint _provider;
    internal::ConsumerEndpoint _consumer;
};

} // namespace middleware::bindings::someip
