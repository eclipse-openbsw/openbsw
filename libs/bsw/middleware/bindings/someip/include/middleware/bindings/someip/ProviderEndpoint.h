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

#include "ip/IPAddress.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/MemberBinding.h"
#include "middleware/bindings/someip/MemberTable.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/bindings/someip/SomeIpBridgeConfig.h"
#include "middleware/core/Message.h"
#include "someip/IEventSender.h"
#include "someip/ISomeIpStack.h"
#include "someip/ProvidedService.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceHandler.h"
#include "someip/ServiceInfo.h"
#include "someip/SomeIpConstants.h"

#include <etl/span.h>

#include <cstdint>

namespace middleware::bindings::someip
{

namespace internal
{

/**
 * The provided instance of one service on SOME/IP: offers it, answers the calls that
 * arrive for it through its skeleton, and sends the skeleton's events and responses.
 * Works on storage its owner sizes.
 */
class ProviderEndpoint final : public ::someip::ServiceHandler
{
public:
    using Message = ::middleware::core::Message;

    /**
     * \param callbackPool     The RPC callbacks the stack takes per call.
     * \param pendingRequests  One slot per request that may be in flight to the skeleton.
     */
    ProviderEndpoint(
        ISomeIpConnection& connection,
        ::someip::ISomeIpStack& stack,
        ::someip::IEventSender& eventSender,
        uint16_t serviceId,
        SomeIpBridgeConfig const& config,
        MemberTable& members,
        RpcCallbackPool& callbackPool,
        ::etl::span<::someip::CallDoneClosure*> const& pendingRequests);

    ProviderEndpoint(ProviderEndpoint const&)            = delete;
    ProviderEndpoint& operator=(ProviderEndpoint const&) = delete;

    /** \see SomeIpServiceEndpoint::bindEvent() */
    void bindEvent(
        uint16_t memberId,
        IPayloadSlot& payloadSlot,
        ::etl::span<uint16_t const> const& eventGroupIds,
        bool isMulticast);

    /** \see SomeIpServiceEndpoint::bindMethod() */
    void bindMethod(
        uint16_t memberId,
        MemberBinding::Kind kind,
        IPayloadSlot* requestSlot,
        IPayloadSlot* responseSlot);

    /** \see SomeIpServiceEndpoint::offerService() */
    void offerService();

    /** Puts the offered service back into the SD startup phase. */
    void restartStartupPhase();

    /** Sends an event, or a field's notification, of the provided instance. */
    bool sendEvent(Message const& msg, MemberBinding const& binding);

    /** Completes the call that the skeleton's response answers. */
    bool sendResponse(Message const& msg, MemberBinding const& binding);

    /** \see ServiceHandler::getRequest() */
    ::someip::ISomeIpSerializable* getRequest(uint16_t methodId) override;

    /** \see ServiceHandler::getResponse() */
    ::someip::ISomeIpSerializable* getResponse(uint16_t methodId) override;

    /** \see ServiceHandler::getMethodDetail() */
    ::someip::MethodDetail const* getMethodDetail(uint16_t methodId) const override;

    /**
     * Initial events are not replayed: a field's current value reaches a new
     * subscriber only through its getter or its next notification.
     */
    bool notifyInitialEvents(
        ::someip::service_id::type,
        ::someip::instance_id::type,
        ::someip::major_version::type,
        ::someip::eventgroup_id::type,
        ::ip::IPAddress const&,
        ::someip::port::type,
        ::someip::proto::type) override;

protected:
    /**
     * \see ServiceHandler::callMethod()
     *
     * Reached through ServiceHandler::dispatchMethod(), which has already checked
     * the request and response against getMethodDetail().
     */
    void callMethod(
        uint16_t methodId,
        ::someip::ISomeIpSerializable const*,
        ::someip::ISomeIpSerializable*,
        ::someip::CallDoneClosure& done) override;

private:
    /**
     * Correlates a request id, this bridge's own slot index, to the closure given
     * to callMethod(). That closure stays alive until it is released, which is
     * exactly as long as this table needs it.
     */
    uint16_t obtainPendingRequest(::someip::CallDoneClosure& done);

    ::someip::CallDoneClosure* findPendingRequest(uint16_t requestId) const;

    void releasePendingRequest(uint16_t requestId);

    ::someip::ServiceDescription makeDescription(uint16_t port) const;

    ISomeIpConnection& _connection;
    ::someip::ISomeIpStack& _stack;
    ::someip::IEventSender& _eventSender;
    uint16_t _serviceId;
    SomeIpBridgeConfig const& _config;
    MemberTable& _members;
    ::etl::span<::someip::CallDoneClosure*> _pendingRequests;
    ::someip::ProvidedService _providedService{};
};

} // namespace internal

} // namespace middleware::bindings::someip
