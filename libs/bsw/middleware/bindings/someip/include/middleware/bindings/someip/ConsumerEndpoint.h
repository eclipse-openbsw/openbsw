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

#include "ip/IPEndpoint.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/MemberBinding.h"
#include "middleware/bindings/someip/MemberTable.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/bindings/someip/SomeIpBridgeConfig.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "someip/IEventListener.h"
#include "someip/INetwork.h"
#include "someip/IRpcSender.h"
#include "someip/IServiceListener.h"
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

namespace middleware::bindings::someip
{

namespace internal
{

class ConsumerEndpoint;

/** A proxy's request to the consumed instance: what its answer is addressed to. */
struct ProxyRequest
{
    MemberBinding const* binding{nullptr};
    uint16_t memberId{0U};
    uint16_t requestId{0U};
    uint8_t proxyCluster{0U};
    uint8_t peerCluster{0U};
    uint8_t addressId{0U};
};

/**
 * One request channel to the consumed instance, and the proxy request it is
 * serving. The stack allows one outstanding call per channel, so a busy
 * channel is skipped until its response or timeout completes the call.
 */
class ConsumerCall final : public ::someip::CallDoneClosure
{
public:
    ConsumerCall(
        ConsumerEndpoint& consumer, ::someip::INetwork& network, ::someip::IRpcSender& sender);

    /** Called by the stack with the call's outcome. */
    ::someip::ServiceResultCode operator()(::someip::ServiceResultCode result) override;

    ::someip::RpcChannel& channel() { return _channel; }

    bool isBusy() const { return _isBusy; }

    void start(ProxyRequest const& request)
    {
        _request = request;
        _isBusy  = true;
    }

    void cancel() { _isBusy = false; }

private:
    ConsumerEndpoint& _consumer;
    ::someip::RpcChannel _channel;
    ProxyRequest _request{};
    bool _isBusy{false};
};

/**
 * The consumed instance of one service on SOME/IP: finds and subscribes to it, sends
 * its proxy's requests on request channels, and delivers the responses and events back
 * to the proxy. Works on storage its owner sizes.
 */
class ConsumerEndpoint final
: public ::someip::IServiceListener
, public ::someip::IEventListener
{
public:
    using Message = ::middleware::core::Message;

    /**
     * \param network    The stack's network, which the request channels are opened on.
     * \param rpcSender  The stack's RPC handler, which sends the requests and matches
     *                   their responses.
     * \param calls      The request channels: as many as requests may be in flight.
     */
    ConsumerEndpoint(
        ISomeIpConnection& connection,
        ::someip::ISomeIpStack& stack,
        ::someip::INetwork& network,
        ::someip::IRpcSender& rpcSender,
        uint16_t serviceId,
        SomeIpBridgeConfig const& config,
        MemberTable& members,
        ::etl::ivector<ConsumerCall>& calls);

    ConsumerEndpoint(ConsumerEndpoint const&)            = delete;
    ConsumerEndpoint& operator=(ConsumerEndpoint const&) = delete;

    /** \see SomeIpServiceEndpoint::bindConsumedEvent() */
    void bindEvent(uint16_t memberId, IPayloadSlot& payloadSlot);

    /** \see SomeIpServiceEndpoint::bindConsumedMethod() */
    void bindMethod(
        uint16_t memberId,
        MemberBinding::Kind kind,
        IPayloadSlot* requestSlot,
        IPayloadSlot* responseSlot);

    /**
     * Two queries: FindService is only emitted for a query without an
     * eventgroup, while the Subscribe is driven by the eventgroup one.
     */
    bool startQuerying();

    void stopQuerying();

    /** Puts the find for the consumed instance, if one is running, back into the SD startup phase.
     */
    void restartStartupPhase();

    /** \see SomeIpServiceEndpoint::isConsumedInstanceAvailable() */
    bool isInstanceAvailable() const;

    /**
     * Sends a proxy's request to the consumed instance on an idle channel.
     * Returning false makes the proxy's call fail at once: the instance is not
     * found yet, every channel is busy, or the request could not be sent.
     * Otherwise the proxy is answered later, when the call completes.
     */
    bool sendRequest(Message const& msg, MemberBinding const& binding);

    /**
     * Opens the request channels to the consumed instance where SD found it, and
     * closes them when it goes away. Until then a proxy's request fails at once.
     */
    void serviceStatusChanged(
        ::someip::ServiceDescription const& service, ServiceStatus status) override;

    /** \see IServiceListener::updateEventgroupDescription() */
    void updateEventgroupDescription(::someip::ServiceDescription&, ServiceStatus) override;

    /** What the stack checks a message of the consumed instance against. */
    ::someip::MethodDetail const* getMethodDetail(uint16_t methodId) const override;

    /**
     * Required by IServiceListener, but never asked for: a received event is
     * routed through eventReceived() by service, instance and event id alone.
     */
    ::someip::EventDetail const* getEventDetail(uint16_t) const override;

    /**
     * Delivers an event of the consumed instance to its proxy. The stack hands
     * every event to every listener; only the consumed instance's are bound here.
     */
    void eventReceived(
        ::someip::service_id::type serviceId,
        uint16_t eventId,
        ::someip::instance_id::type instanceId,
        ::someip::major_version::type,
        ::someip::SomeIpParser& parser) override;

private:
    friend class ConsumerCall;

    /**
     * Answers the proxy whose request \p call carried: with the parsed response,
     * or with an error for a timeout or a SOME/IP error. The response is
     * addressed back the way the request came, as a skeleton's would be.
     */
    void
    callDone(ConsumerCall& call, ProxyRequest const& request, ::someip::ServiceResultCode result);

    /** How a SOME/IP call outcome reaches the proxy's future. */
    static ::middleware::core::ErrorState errorStateFor(::someip::ServiceResultCode result);

    /** A session id of 0 means session handling is off, so it is skipped on wrap. */
    static void advanceSession(::someip::RpcChannel& channel);

    /**
     * Created on first use rather than in the constructor: a channel binds its
     * timeout to its own address, so it is constructed once, in place.
     */
    void openChannels(::ip::IPEndpoint const& endpoint);

    /** A busy channel is left to finish; callDone() closes it then. */
    void closeChannels();

    /** Points an idle channel at the consumed instance, or closes it when it is gone. */
    void reopenChannel(ConsumerCall& call);

    ConsumerCall* idleCall();

    /**
     * The address is left unset: FindService matches on service, major version
     * and instance, and addSubscribe() fills in the local address. The port is
     * passed through verbatim as the local event port.
     */
    void describeQuery(::someip::ServiceQuery& query, ::someip::eventgroup_id::type eventGroupId);

    static void restartQueryPhase(::someip::ServiceQuery& query);

    ISomeIpConnection& _connection;
    ::someip::ISomeIpStack& _stack;
    ::someip::INetwork& _network;
    ::someip::IRpcSender& _rpcSender;
    uint16_t _serviceId;
    SomeIpBridgeConfig const& _config;
    MemberTable& _members;
    ::etl::ivector<ConsumerCall>& _calls;
    ::someip::ServiceQuery _serviceQuery{};
    ::someip::ServiceQuery _eventGroupQuery{};
    ::ip::IPEndpoint _consumedEndpoint{};
    bool _isQuerying{false};
    bool _isAvailable{false};
};

} // namespace internal

} // namespace middleware::bindings::someip
