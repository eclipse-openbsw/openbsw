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
#include "ip/IPEndpoint.h"
#include "middleware/core/DatabaseManipulator.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/TransceiverBase.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"
#include "middleware/someip/SomeIpCodec.h"
#include "someip/IEventListener.h"
#include "someip/IEventSender.h"
#include "someip/INetwork.h"
#include "someip/IRpcSender.h"
#include "someip/IServiceListener.h"
#include "someip/ISomeIpStack.h"
#include "someip/ProvidedService.h"
#include "someip/RpcChannel.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceHandler.h"
#include "someip/ServiceInfo.h"
#include "someip/ServiceQuery.h"
#include "someip/SomeIpConstants.h"

#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/error_handler.h>
#include <etl/pool.h>
#include <etl/span.h>
#include <etl/vector.h>

#include <cstdint>

namespace middleware::someip
{

/**
 * SOME/IP network parameters the middleware deployment has no vocabulary for.
 * Supplied by whoever wires the endpoint into an application, from the service's
 * SOME/IP deployment.
 */
struct SomeIpBridgeConfig
{
    ::someip::major_version::type majorVersion{1U};
    ::someip::minor_version::type minorVersion{1U};
    ::someip::instance_id::type instanceId{1U};

    /**
     * Port this node offers the service on, and receives the consumed instance's
     * events on.
     */
    uint16_t providerPort{0U};

    /**
     * Peer instance a proxy of this service consumes, and the eventgroup it
     * subscribes to. Unused if no proxy subscribes.
     */
    ::someip::instance_id::type consumedInstanceId{0U};
    ::someip::eventgroup_id::type consumedEventGroupId{0U};

    uint8_t proto{::someip::proto::SD_L4_PROTO_UDP};
    ::someip::ttl::type ttl{::someip::ttl::type{}};

    /** SD announcement timing. Left default, this is the stack's own ramp. */
    ::someip::SdOfferConfig sdConfig{};

    /** SD FindService timing for the consumed instance. */
    ::someip::SdConfig findConfig{};

    /**
     * How long a request to the consumed instance waits for its response
     * before the proxy is answered with an error.
     */
    uint32_t requestTimeoutMs{1000U};

    /**
     * SOME/IP client id of the first request channel to the consumed instance.
     * The bridge uses MaxPending consecutive ids, one per channel, which is how
     * the stack tells their responses apart.
     */
    uint16_t clientId{1U};

    /**
     * Group the multicast-flagged events publish to. The same endpoint must be
     * registered on the provided eventgroup, since that is what puts the multicast
     * option in the Ack and so tells a subscriber where to listen. Unset leaves
     * every event unicast.
     */
    ::ip::IPAddress eventMulticastAddress{};
    uint16_t eventMulticastPort{0U};
};

namespace internal
{

/**
 * Type-erased accessor for one payload's wire encoding.
 * Lets the dispatch table hold entries for differently typed and differently
 * tagged payloads uniformly. Each MemberBinding owns one per direction.
 */
class IPayloadSlot
{
public:
    virtual ~IPayloadSlot() = default;

    /** Returns the underlying ISomeIpSerializable, for handing to the stack. */
    virtual ::someip::ISomeIpSerializable& serializable() = 0;

    /**
     * Serializes the object at \p src, rather than this slot's own value, until
     * called again with nullptr. \p src must point at a live object of the
     * slot's own T; there is no runtime type check.
     */
    virtual void serializeFrom(void const* src) = 0;

    /**
     * Attaches this slot's decoded value to \p msg as the middleware payload.
     * Uses the typed path, which copy-constructs rather than copying bytes, so a
     * type holding pointers into itself is rebased onto the new storage instead of
     * being left pointing at this slot.
     */
    virtual ::middleware::core::HRESULT allocateInto(::middleware::core::Message& msg) const = 0;
};

template<typename T, typename Tag = internal::ScalarTag<T>>
class PayloadSlot final : public IPayloadSlot
{
public:
    ::someip::ISomeIpSerializable& serializable() override { return _value; }

    void serializeFrom(void const* const src) override
    {
        _value.serializeFrom(static_cast<T const*>(src));
    }

    ::middleware::core::HRESULT allocateInto(::middleware::core::Message& msg) const override
    {
        return ::middleware::core::MessagePayloadBuilder::getInstance().allocate(_value.get(), msg);
    }

private:
    SomeIpSerializableValue<T, Tag> _value;
};

/**
 * What a SomeIpConnection needs from each service it carries: which service and
 * instances it is, a way to send that service's messages to the wire, and the
 * start and stop of discovery for its consumed instance.
 */
class IServiceEndpoint
{
public:
    virtual ~IServiceEndpoint() = default;

    virtual uint16_t getServiceId() const          = 0;
    virtual uint16_t getProvidedInstanceId() const = 0;
    virtual uint16_t getConsumedInstanceId() const = 0;

    /** Called once by the connection the endpoint is added to. */
    virtual void attach(::middleware::core::IClusterConnectionConfigurationBase& connection) = 0;

    /** Sends one middleware message of this service to the wire. */
    virtual bool send(::middleware::core::Message const& msg) = 0;

    /** Starts SD FindService and Subscribe for the consumed instance. */
    virtual bool startQuerying() = 0;
    virtual void stopQuerying()  = 0;
};

} // namespace internal

/**
 * One row of the bridge's dispatch table.
 * Holds everything needed to move one service member between a middleware member
 * id and the stack. Built once per member at setup, then looked up by member id.
 */
struct MemberBinding
{
    enum class Kind : uint8_t
    {
        Event,           // notification, no request or response
        FireAndForget,   // method, no response
        RequestResponse, // method, request and response
        RequestOnly,     // method, request but no response value
        ResponseOnly     // method, no request but a response value
    };

    // Widest members first, so the table carries no padding between them.

    /** Null where that direction carries no payload. */
    internal::IPayloadSlot* requestSlot{nullptr};
    internal::IPayloadSlot* responseSlot{nullptr};

    /** Eventgroups this member belongs to. An empty span sends unicast only. */
    ::etl::span<uint16_t const> eventGroupIds{};

    /**
     * What the stack is told when this member arrives as a method call. Fixed
     * at bind time, so answering is a lookup, and each member's answer has its
     * own storage rather than one object overwritten per call.
     */
    ::someip::MethodDetail methodDetail{};

    uint16_t memberId{0U};

    /** The provided instance, or the consumed one for a bindConsumedEvent(). */
    uint16_t instanceId{0U};

    Kind kind{Kind::Event};

    /** Publish to the config's multicast group rather than to each subscriber. */
    bool isMulticast{false};
};

/**
 * Carries one service over SOME/IP: one provided instance, and optionally one
 * consumed instance. For the provided instance it answers requests and sends
 * events; for the consumed one it sends the proxy's requests and delivers the
 * responses and events back to it. It reaches the middleware through the
 * SomeIpConnection it is added to, which may carry other services as well.
 *
 * \tparam MaxMembers  capacity of the dispatch table; size it to what is bound.
 * \tparam MaxPending  how many requests may be in flight in each direction: from
 *                     the wire to the skeleton, and from the proxy to the wire.
 */
template<uint8_t MaxMembers, uint8_t MaxPending>
class SomeIpServiceEndpoint final
: public internal::IServiceEndpoint
, public ::someip::ServiceHandler
, public ::someip::IServiceListener
, public ::someip::IEventListener
{
public:
    using Message = ::middleware::core::Message;
    using HRESULT = ::middleware::core::HRESULT;

    /**
     * \param network    The stack's network, which the request channels to the
     *                   consumed instance are opened on.
     * \param rpcSender  The stack's RPC handler, which sends those requests and
     *                   matches their responses.
     */
    SomeIpServiceEndpoint(
        ::someip::ISomeIpStack& stack,
        ::someip::IEventSender& eventSender,
        ::someip::INetwork& network,
        ::someip::IRpcSender& rpcSender,
        uint16_t const serviceId,
        SomeIpBridgeConfig const& config)
    : ::someip::ServiceHandler(_rpcCallbackPool)
    , _stack(stack)
    , _eventSender(eventSender)
    , _network(network)
    , _rpcSender(rpcSender)
    , _serviceId(serviceId)
    , _config(config)
    {}

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
        MemberBinding binding;
        binding.memberId      = memberId;
        binding.kind          = MemberBinding::Kind::Event;
        binding.instanceId    = _config.instanceId;
        binding.requestSlot   = &payloadSlot;
        binding.eventGroupIds = eventGroupIds;
        binding.isMulticast   = isMulticast;
        addBinding(binding);
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
        MemberBinding binding;
        binding.memberId     = memberId;
        binding.kind         = kind;
        binding.instanceId   = _config.instanceId;
        binding.requestSlot  = requestSlot;
        binding.responseSlot = responseSlot;
        binding.methodDetail = methodDetailFor(memberId, kind);
        addBinding(binding);
    }

    /** Registers an event of the consumed instance, delivered to its proxy. */
    void bindConsumedEvent(uint16_t const memberId, internal::IPayloadSlot& payloadSlot)
    {
        MemberBinding binding;
        binding.memberId    = memberId;
        binding.kind        = MemberBinding::Kind::Event;
        binding.instanceId  = _config.consumedInstanceId;
        binding.requestSlot = &payloadSlot;
        addBinding(binding);
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
        MemberBinding binding;
        binding.memberId     = memberId;
        binding.kind         = kind;
        binding.instanceId   = _config.consumedInstanceId;
        binding.requestSlot  = requestSlot;
        binding.responseSlot = responseSlot;
        addBinding(binding);
    }

    /**
     * Registers the ProvidedService, starting SD OfferService on the wire.
     * Called once, after every bindEvent() and bindMethod().
     */
    void offerService()
    {
        _providedService.setHandler(*this);
        _providedService.description = makeDescription(_config.providerPort);
        _providedService.setSdConfig(_config.sdConfig);
        static_cast<void>(_stack.registerProvidedService(_providedService));
    }

    /**
     * Puts the offered service, and the find for the consumed instance if one is
     * running, back into the SD startup phase.
     */
    void restartStartupPhase()
    {
        _providedService.init();
        if (_isQuerying)
        {
            restartQueryPhase(_serviceQuery);
            restartQueryPhase(_eventGroupQuery);
        }
    }

    /**
     * Whether SD has found the consumed instance, so a request to it can be sent.
     * Until then, a proxy's request fails at once.
     */
    bool isConsumedInstanceAvailable() const { return _isConsumedAvailable; }

    /** \see IServiceEndpoint::getServiceId() */
    uint16_t getServiceId() const override { return _serviceId; }

    /** \see IServiceEndpoint::getProvidedInstanceId() */
    uint16_t getProvidedInstanceId() const override { return _config.instanceId; }

    /** \see IServiceEndpoint::getConsumedInstanceId() */
    uint16_t getConsumedInstanceId() const override { return _config.consumedInstanceId; }

    /** \see IServiceEndpoint::attach() */
    void attach(::middleware::core::IClusterConnectionConfigurationBase& connection) override
    {
        _connection = &connection;
    }

    /**
     * Sends one middleware message of this service to the wire. The connection
     * frees the payload afterwards.
     */
    bool send(Message const& msg) override
    {
        MemberBinding const* const binding
            = findBinding(msg.getHeader().serviceInstanceId, msg.getHeader().memberId);
        if (binding == nullptr)
        {
            return false;
        }

        if (msg.isEvent())
        {
            return sendEventOrFieldNotification(msg, *binding);
        }
        if (msg.isResponse())
        {
            return sendProviderResponse(msg, *binding);
        }
        if (msg.isRequest() || msg.isFireAndForgetRequest())
        {
            return sendConsumerRequest(msg, *binding);
        }
        return false;
    }

    /** \see ServiceHandler::getRequest() */
    ::someip::ISomeIpSerializable* getRequest(uint16_t const methodId) override
    {
        MemberBinding const* const binding = findBinding(_config.instanceId, methodId);
        return (binding != nullptr) && (binding->requestSlot != nullptr)
                   ? &binding->requestSlot->serializable()
                   : nullptr;
    }

    /** \see ServiceHandler::getResponse() */
    ::someip::ISomeIpSerializable* getResponse(uint16_t const methodId) override
    {
        MemberBinding const* const binding = findBinding(_config.instanceId, methodId);
        return (binding != nullptr) && (binding->responseSlot != nullptr)
                   ? &binding->responseSlot->serializable()
                   : nullptr;
    }

    /**
     * Satisfies both ServiceHandler's and IServiceListener's pure virtual; a second
     * definition would not be valid.
     */
    ::someip::MethodDetail const* getMethodDetail(uint16_t const methodId) const override
    {
        MemberBinding const* const binding = findBinding(_config.instanceId, methodId);
        // Events never arrive as an RPC method call.
        if ((binding == nullptr) || (binding->kind == MemberBinding::Kind::Event))
        {
            return nullptr;
        }
        return &binding->methodDetail;
    }

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
        ::someip::proto::type) override
    {
        return true;
    }

    /**
     * Opens the request channels to the consumed instance where SD found it, and
     * closes them when it goes away. Until then a proxy's request fails at once.
     */
    void serviceStatusChanged(
        ::someip::ServiceDescription const& service, ServiceStatus const status) override
    {
        if ((service.serviceId != _serviceId) || (service.instanceId != _config.consumedInstanceId))
        {
            return;
        }
        if (status == ServiceStatus::SERVICE_AVAILABLE)
        {
            openConsumerChannels(::ip::IPEndpoint(service.ipAddress, service.port));
        }
        else if (status == ServiceStatus::SERVICE_UNAVAILABLE)
        {
            closeConsumerChannels();
        }
    }

    /** \see IServiceListener::updateEventgroupDescription() */
    void updateEventgroupDescription(::someip::ServiceDescription&, ServiceStatus const) override {}

    /**
     * Required by IServiceListener, but never asked for: a received event is
     * routed through eventReceived() by service, instance and event id alone.
     */
    ::someip::EventDetail const* getEventDetail(uint16_t) const override { return nullptr; }

    /**
     * Delivers an event of the consumed instance to its proxy. The stack hands
     * every event to every listener; only the consumed instance's are bound here.
     */
    void eventReceived(
        ::someip::service_id::type const serviceId,
        uint16_t const eventId,
        ::someip::instance_id::type const instanceId,
        ::someip::major_version::type const,
        ::someip::SomeIpParser& parser) override
    {
        if ((serviceId != _serviceId) || (instanceId != _config.consumedInstanceId))
        {
            return;
        }
        MemberBinding const* const binding = findBinding(instanceId, eventId);
        if ((binding == nullptr) || (binding->requestSlot == nullptr))
        {
            return;
        }

        parser >> binding->requestSlot->serializable();
        if (!parser.isGood())
        {
            return;
        }

        Message msg = Message::createEvent(_serviceId, eventId, instanceId, getSourceClusterId());
        allocateAndDispatch(msg, *binding->requestSlot);
    }

protected:
    /**
     * \see ServiceHandler::callMethod()
     *
     * Reached through ServiceHandler::dispatchMethod(), which has already checked
     * the request and response against getMethodDetail().
     */
    void callMethod(
        uint16_t const methodId,
        ::someip::ISomeIpSerializable const*,
        ::someip::ISomeIpSerializable* const,
        ::someip::CallDoneClosure& done) override
    {
        MemberBinding const* const binding = findBinding(_config.instanceId, methodId);
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
                getTargetClusterId(),
                getSourceClusterId());
            // The skeleton reads its argument out of the Message, so the parsed
            // value has to be copied across. Referencing the slot is safe: this
            // dispatch completes before callMethod() returns.
            bool const hasPayload = (binding->requestSlot != nullptr);
            if (!hasPayload || loadPayloadFromSlot(msg, *binding->requestSlot))
            {
                dispatchAndRelease(msg, hasPayload);
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
            getTargetClusterId(),
            getSourceClusterId(),
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

        HRESULT const result = dispatchMessage(msg);
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

private:
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
            SomeIpServiceEndpoint& bridge,
            ::someip::INetwork& network,
            ::someip::IRpcSender& sender)
        : _bridge(bridge), _channel(network, sender)
        {}

        /** Called by the stack with the call's outcome. */
        ::someip::ServiceResultCode operator()(::someip::ServiceResultCode const result) override
        {
            // Freed before the proxy is answered, which may already send the next request.
            ProxyRequest const request = _request;
            _isBusy                    = false;
            _bridge.consumerCallDone(*this, request, result);
            return result;
        }

        ::someip::RpcChannel& channel() { return _channel; }

        bool isBusy() const { return _isBusy; }

        void start(ProxyRequest const& request)
        {
            _request = request;
            _isBusy  = true;
        }

        void cancel() { _isBusy = false; }

    private:
        SomeIpServiceEndpoint& _bridge;
        ::someip::RpcChannel _channel;
        ProxyRequest _request{};
        bool _isBusy{false};
    };

    /**
     * Hands a message from the wire to the middleware, through the connection
     * this endpoint is added to.
     */
    HRESULT dispatchMessage(Message const& msg) const
    {
        return (_connection != nullptr) ? _connection->dispatchMessage(msg)
                                        : HRESULT::NotRegistered;
    }

    /** The cluster of this node's skeletons and proxies. */
    uint8_t getSourceClusterId() const
    {
        return (_connection != nullptr) ? _connection->getSourceClusterId() : INVALID_CLUSTER_ID;
    }

    /** The cluster standing in for the remote side. */
    uint8_t getTargetClusterId() const
    {
        return (_connection != nullptr) ? _connection->getTargetClusterId() : INVALID_CLUSTER_ID;
    }

    /** Reserved by the middleware for "no cluster". */
    static constexpr uint8_t INVALID_CLUSTER_ID = 0xFFU;

    /**
     * A table too small for the service is a configuration error: a member
     * dropped here would surface only as "method not available" on the wire.
     * Kept sorted by key so a lookup is a binary search; inserting after equal
     * keys keeps the first-bound member the one found.
     */
    void addBinding(MemberBinding const& binding)
    {
        if (_bindings.full())
        {
            ETL_ASSERT_FAIL(
                ETL_ERROR_GENERIC("SomeIpServiceEndpoint: binding table full, raise MaxMembers"));
            return;
        }
        auto const position = ::etl::upper_bound(
            _bindings.begin(),
            _bindings.end(),
            keyOf(binding),
            [](uint32_t const key, MemberBinding const& entry) { return key < keyOf(entry); });
        _bindings.insert(position, binding);
    }

    static ::someip::MethodDetail
    methodDetailFor(uint16_t const methodId, MemberBinding::Kind const kind)
    {
        ::someip::MethodDetail detail{};
        detail.methodId = methodId;
        switch (kind)
        {
            case MemberBinding::Kind::FireAndForget:
                detail.methodType   = ::someip::SomeIpMethodType::METHOD_REQUEST_ONLY;
                detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET;
                break;
            case MemberBinding::Kind::RequestResponse:
                detail.methodType   = ::someip::SomeIpMethodType::METHOD_REQUEST_RESPONSE;
                detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE;
                break;
            case MemberBinding::Kind::RequestOnly:
                detail.methodType   = ::someip::SomeIpMethodType::METHOD_REQUEST_ONLY;
                detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE;
                break;
            case MemberBinding::Kind::ResponseOnly:
                detail.methodType   = ::someip::SomeIpMethodType::METHOD_RESPONSE_ONLY;
                detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE;
                break;
            case MemberBinding::Kind::Event:
            default:                         break;
        }
        return detail;
    }

    MemberBinding const* findBinding(uint16_t const instanceId, uint16_t const memberId) const
    {
        uint32_t const key = keyOf(instanceId, memberId);
        auto const it      = ::etl::lower_bound(
            _bindings.begin(),
            _bindings.end(),
            key,
            [](MemberBinding const& entry, uint32_t const k) { return keyOf(entry) < k; });
        return ((it != _bindings.end()) && (keyOf(*it) == key)) ? &(*it) : nullptr;
    }

    /** Instance in the high half, member in the low half. */
    static uint32_t keyOf(uint16_t const instanceId, uint16_t const memberId)
    {
        return (static_cast<uint32_t>(instanceId) << 16U) | static_cast<uint32_t>(memberId);
    }

    static uint32_t keyOf(MemberBinding const& binding)
    {
        return keyOf(binding.instanceId, binding.memberId);
    }

    /**
     * Two queries: FindService is only emitted for a query without an
     * eventgroup, while the Subscribe is driven by the eventgroup one.
     */
    bool startQuerying() override
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

    void stopQuerying() override
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

    /**
     * The address is left unset: FindService matches on service, major version
     * and instance, and addSubscribe() fills in the local address. The port is
     * passed through verbatim as the local event port.
     */
    void
    describeQuery(::someip::ServiceQuery& query, ::someip::eventgroup_id::type const eventGroupId)
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

    static void restartQueryPhase(::someip::ServiceQuery& query)
    {
        query.state           = ::someip::ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE;
        query.timestamp       = 0U;
        query.repetitionCount = 0U;
    }

    ::someip::ServiceDescription makeDescription(uint16_t const port) const
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

    bool sendEventOrFieldNotification(Message const& msg, MemberBinding const& binding) const
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

    bool sendProviderResponse(Message const& msg, MemberBinding const& binding) const
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

    /**
     * Sends a proxy's request to the consumed instance on an idle channel.
     * Returning false makes the proxy's call fail at once: the instance is not
     * found yet, every channel is busy, or the request could not be sent.
     * Otherwise the proxy is answered later, from consumerCallDone().
     */
    bool sendConsumerRequest(Message const& msg, MemberBinding const& binding) const
    {
        // Events of the consumed instance are received, never sent.
        if (binding.kind == MemberBinding::Kind::Event)
        {
            return false;
        }
        ConsumerCall* const call = idleConsumerCall();
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
                = (binding.responseSlot != nullptr) ? &binding.responseSlot->serializable()
                                                    : nullptr;
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

    /**
     * Answers the proxy whose request \p call carried: with the parsed response,
     * or with an error for a timeout or a SOME/IP error. The response is
     * addressed back the way the request came, as a skeleton's would be.
     */
    void consumerCallDone(
        ConsumerCall& call, ProxyRequest const& request, ::someip::ServiceResultCode const result)
    {
        if (!_isConsumedAvailable || !(call.channel().getRemoteIp() == _consumedEndpoint))
        {
            // The instance went away or moved while this call was out.
            reopenConsumerChannel(call);
        }

        ::middleware::core::ErrorState error = errorStateFor(result);
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
            internal::IPayloadSlot const* const slot = request.binding->responseSlot;
            if (slot == nullptr)
            {
                dispatchAndRelease(response, /*hasPayload=*/false);
                return;
            }
            if (loadPayloadFromSlot(response, *slot))
            {
                dispatchAndRelease(response, /*hasPayload=*/true);
                return;
            }
            error = ::middleware::core::ErrorState::QueueFullError; // allocator exhausted
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
        dispatchAndRelease(errorResponse, /*hasPayload=*/false);
    }

    /** How a SOME/IP call outcome reaches the proxy's future. */
    static ::middleware::core::ErrorState errorStateFor(::someip::ServiceResultCode const result)
    {
        switch (result)
        {
            case ::someip::RPC_POSITIVE_RESPONSE: return ::middleware::core::ErrorState::NoError;
            case ::someip::BUSY_ERROR:            return ::middleware::core::ErrorState::ServiceBusy;
            case ::someip::RPC_TIMEOUT:
            case ::someip::RPC_SERVICE_NOT_AVAILABLE:
            case ::someip::RPC_ERROR_NO_CHANNEL:
                return ::middleware::core::ErrorState::ServiceNotFound;
            case ::someip::RPC_INVALID_PAYLOAD:
                return ::middleware::core::ErrorState::DeserializationError;
            case ::someip::COULD_NOT_DELIVER:
            case ::someip::OUT_OF_MEMORY:     return ::middleware::core::ErrorState::QueueFullError;
            default:                          return ::middleware::core::ErrorState::UserDefinedError;
        }
    }

    /** A session id of 0 means session handling is off, so it is skipped on wrap. */
    static void advanceSession(::someip::RpcChannel& channel)
    {
        uint16_t const next = static_cast<uint16_t>(channel.getSessionId() + 1U);
        channel.setSessionId((next == 0U) ? 1U : next);
    }

    /**
     * Created on first use rather than in the constructor: a channel binds its
     * timeout to its own address, so it is constructed once, in place.
     */
    void openConsumerChannels(::ip::IPEndpoint const& endpoint)
    {
        _consumedEndpoint    = endpoint;
        _isConsumedAvailable = true;
        while (!_consumerCalls.full())
        {
            _consumerCalls.emplace_back(*this, _network, _rpcSender);
            _consumerCalls.back().channel().setClientId(
                static_cast<uint16_t>(_config.clientId + (_consumerCalls.size() - 1U)));
        }
        for (auto& call : _consumerCalls)
        {
            if (!call.isBusy())
            {
                reopenConsumerChannel(call);
            }
        }
    }

    /** A busy channel is left to finish; consumerCallDone() closes it then. */
    void closeConsumerChannels()
    {
        _isConsumedAvailable = false;
        for (auto& call : _consumerCalls)
        {
            if (!call.isBusy())
            {
                call.channel().close();
            }
        }
    }

    /** Points an idle channel at the consumed instance, or closes it when it is gone. */
    void reopenConsumerChannel(ConsumerCall& call)
    {
        if (!_isConsumedAvailable)
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

    ConsumerCall* idleConsumerCall() const
    {
        if (!_isConsumedAvailable)
        {
            return nullptr;
        }
        for (auto& call : _consumerCalls)
        {
            if (!call.isBusy() && call.channel().isOpen())
            {
                return &call;
            }
        }
        return nullptr;
    }

    /**
     * Points \p slot at the Message's own payload rather than copying it in. The
     * stack serializes before sendEvent() or the response closure returns, and
     * write() frees the payload only after that; the caller clears it then.
     */
    static void serializeInPlace(Message const& msg, internal::IPayloadSlot& slot)
    {
        auto const raw = ::middleware::core::MessagePayloadBuilder::readRawPayload(msg);
        // Slot and message payload have the same type by construction.
        slot.serializeFrom(raw.data());
    }

    /** Attaches the value decoded into \p slot to \p msg as its payload. */
    static bool loadPayloadFromSlot(Message& msg, internal::IPayloadSlot const& slot)
    {
        return slot.allocateInto(msg) == HRESULT::Ok;
    }

    /**
     * Dispatch is synchronous, so the payload is consumed by the time this
     * returns, and nothing else frees it: these messages are injected straight
     * into dispatchMessage() from the wire side.
     */
    void dispatchAndRelease(Message& msg, bool const hasPayload)
    {
        static_cast<void>(dispatchMessage(msg));
        if (hasPayload)
        {
            ::middleware::core::MessagePayloadBuilder::deallocate(msg);
        }
    }

    void allocateAndDispatch(Message& msg, internal::IPayloadSlot& slot)
    {
        if (!loadPayloadFromSlot(msg, slot))
        {
            return; // allocator exhausted; drop rather than dispatch a blank
        }
        dispatchAndRelease(msg, /*hasPayload=*/true);
    }

    /**
     * Correlates a request id, this bridge's own slot index, to the closure given
     * to callMethod(). That closure stays alive until it is released, which is
     * exactly as long as this table needs it.
     */
    uint16_t obtainPendingRequest(::someip::CallDoneClosure& done)
    {
        for (uint16_t i = 0U; i < MaxPending; ++i)
        {
            if (_pendingRequests[i] == nullptr)
            {
                _pendingRequests[i] = &done;
                return i;
            }
        }
        return ::middleware::core::INVALID_REQUEST_ID;
    }

    /**
     * const because write() is const and still has to clear the slot, which is
     * why _pendingRequests is mutable.
     */
    ::someip::CallDoneClosure* findPendingRequest(uint16_t const requestId) const
    {
        return (requestId < MaxPending) ? _pendingRequests[requestId] : nullptr;
    }

    void releasePendingRequest(uint16_t const requestId) const
    {
        if (requestId < MaxPending)
        {
            _pendingRequests[requestId] = nullptr;
        }
    }

    using RpcCallbackPool = ::etl::pool<ServiceHandler::RpcCallback, MaxPending>;

    ::someip::ISomeIpStack& _stack;
    ::someip::IEventSender& _eventSender;
    ::someip::INetwork& _network;
    ::someip::IRpcSender& _rpcSender;
    uint16_t _serviceId;
    SomeIpBridgeConfig _config;
    ::middleware::core::IClusterConnectionConfigurationBase* _connection{nullptr};

    ::someip::ProvidedService _providedService{};
    ::someip::ServiceQuery _serviceQuery{};
    ::someip::ServiceQuery _eventGroupQuery{};
    bool _isQuerying{false};

    RpcCallbackPool _rpcCallbackPool{};
    mutable ::etl::array<::someip::CallDoneClosure*, MaxPending> _pendingRequests{};

    /** mutable for the same reason: write() is const and still starts a call. */
    mutable ::etl::vector<ConsumerCall, MaxPending> _consumerCalls{};
    ::ip::IPEndpoint _consumedEndpoint{};
    bool _isConsumedAvailable{false};

    /** Sorted by keyOf(); see addBinding(). */
    ::etl::vector<MemberBinding, MaxMembers> _bindings{};
};

/**
 * The configuration of one middleware cluster connection whose services travel
 * over SOME/IP instead of shared memory. It is installed on the connection from
 * the cluster of this node's skeletons and proxies to a cluster standing in for
 * the remote side, and routes each message to the endpoint of its service.
 *
 * \tparam MaxEndpoints     how many service endpoints it carries: one per
 *                          service, or per instance of a service.
 * \tparam MaxTransceivers  how many skeletons, and how many proxies, one service
 *                          may register on it.
 */
template<uint8_t MaxEndpoints, uint8_t MaxTransceivers>
class SomeIpConnection : public ::middleware::core::IClusterConnectionConfiguration
{
public:
    using Base    = ::middleware::core::IClusterConnectionConfiguration;
    using Message = ::middleware::core::Message;
    using HRESULT = ::middleware::core::HRESULT;

    SomeIpConnection(uint8_t const sourceClusterId, uint8_t const targetClusterId)
    : _sourceClusterId(sourceClusterId), _targetClusterId(targetClusterId)
    {}

    /**
     * Adds the endpoint of one service. Called once per endpoint, before the
     * connection is installed and before any skeleton or proxy is initialized.
     */
    void add(internal::IServiceEndpoint& endpoint)
    {
        if (_endpoints.full())
        {
            ETL_ASSERT_FAIL(
                ETL_ERROR_GENERIC("SomeIpConnection: endpoint table full, raise MaxEndpoints"));
            return;
        }
        _endpoints.push_back(&endpoint);
        endpoint.attach(*this);

        // Instances of one service share its registrations.
        uint16_t const serviceId = endpoint.getServiceId();
        if (::middleware::core::meta::DbManipulator::getTransceiversByServiceId(
                _proxyTransceivers.begin(), _proxyTransceivers.end(), serviceId)
            == _proxyTransceivers.end())
        {
            size_t const index = _proxyTransceivers.size();
            _proxyTransceivers.emplace_back(&_proxyLists[index], serviceId, 0U);
            _skeletonTransceivers.emplace_back(&_skeletonLists[index], serviceId, 0U);
        }
    }

    /** \see IClusterConnectionConfigurationBase::getSourceClusterId() */
    uint8_t getSourceClusterId() const override { return _sourceClusterId; }

    /** \see IClusterConnectionConfigurationBase::getTargetClusterId() */
    uint8_t getTargetClusterId() const override { return _targetClusterId; }

    /** \see IClusterConnectionConfigurationBase::registeredTransceiversCount() */
    size_t registeredTransceiversCount(uint16_t const serviceId) const override
    {
        return ::middleware::core::meta::DbManipulator::registeredTransceiversCount(
                   _proxyTransceivers.begin(), _proxyTransceivers.end(), serviceId)
               + ::middleware::core::meta::DbManipulator::registeredTransceiversCount(
                   _skeletonTransceivers.begin(), _skeletonTransceivers.end(), serviceId);
    }

    /**
     * A proxy's subscription is what starts SD FindService and Subscribe for the
     * instance it consumes, on the endpoint that consumes it.
     */
    HRESULT
    subscribe(::middleware::core::ProxyBase& proxy, uint16_t const serviceInstanceId) override
    {
        internal::IServiceEndpoint* const endpoint
            = consumerOf(proxy.getServiceId(), serviceInstanceId);
        if (endpoint == nullptr)
        {
            return HRESULT::InstanceNotFound;
        }
        HRESULT const result = ::middleware::core::meta::DbManipulator::subscribe(
            _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, serviceInstanceId);
        if ((result == HRESULT::Ok) && !endpoint->startQuerying())
        {
            ::middleware::core::meta::DbManipulator::unsubscribe(
                _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, proxy.getServiceId());
            return HRESULT::TransceiverInitializationFailed;
        }
        return result;
    }

    /** \see IClusterConnectionConfiguration::unsubscribe() */
    void unsubscribe(::middleware::core::ProxyBase& proxy, uint16_t const serviceId) override
    {
        ::middleware::core::meta::DbManipulator::unsubscribe(
            _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, serviceId);
        internal::IServiceEndpoint* const endpoint = consumerOf(serviceId, proxy.getInstanceId());
        if (endpoint != nullptr)
        {
            endpoint->stopQuerying();
        }
    }

    /** \see IClusterConnectionConfiguration::subscribe() */
    HRESULT
    subscribe(::middleware::core::SkeletonBase& skeleton, uint16_t const serviceInstanceId) override
    {
        return ::middleware::core::meta::DbManipulator::subscribe(
            _skeletonTransceivers.begin(),
            _skeletonTransceivers.end(),
            skeleton,
            serviceInstanceId);
    }

    /** \see IClusterConnectionConfiguration::unsubscribe() */
    void unsubscribe(::middleware::core::SkeletonBase& skeleton, uint16_t const serviceId) override
    {
        ::middleware::core::meta::DbManipulator::unsubscribe(
            _skeletonTransceivers.begin(), _skeletonTransceivers.end(), skeleton, serviceId);
    }

    /** No middleware-side timeouts: the stack owns SOME/IP request timeouts. */
    void registerTimeoutTransceiver(::middleware::core::ITimeoutHandler&) override {}

    /** \see ITimeoutConfiguration::unregisterTimeoutTransceiver() */
    void unregisterTimeoutTransceiver(::middleware::core::ITimeoutHandler&) override {}

    /** Nothing is ever registered, so there is nothing to update. */
    void updateTimeouts() override {}

    /**
     * Sends one middleware message to the wire, through the endpoint of its
     * service. The connection is the terminal consumer of everything it is
     * handed, so it frees the payload: sendMessage() deallocates only on its
     * same-cluster path, and replacing the SHM queue removes the receiver that
     * would otherwise do it.
     */
    bool write(Message const& msg) const override
    {
        internal::IServiceEndpoint* const endpoint
            = endpointOf(msg.getHeader().serviceId, msg.getHeader().serviceInstanceId);
        bool const isHandled = (endpoint != nullptr) && endpoint->send(msg);
        ::middleware::core::MessagePayloadBuilder::deallocate(msg);
        return isHandled;
    }

    /** \see IClusterConnectionConfigurationBase::dispatchMessage() */
    HRESULT dispatchMessage(Message const& msg) const override
    {
        return Base::dispatchMessage(
            _proxyTransceivers.begin(),
            _proxyTransceivers.end(),
            _skeletonTransceivers.begin(),
            _skeletonTransceivers.end(),
            msg);
    }

private:
    /** The endpoint of \p serviceId that provides or consumes \p instanceId. */
    internal::IServiceEndpoint*
    endpointOf(uint16_t const serviceId, uint16_t const instanceId) const
    {
        for (internal::IServiceEndpoint* const endpoint : _endpoints)
        {
            if ((endpoint->getServiceId() == serviceId)
                && ((endpoint->getProvidedInstanceId() == instanceId)
                    || (endpoint->getConsumedInstanceId() == instanceId)))
            {
                return endpoint;
            }
        }
        return nullptr;
    }

    /** The endpoint of \p serviceId that consumes \p instanceId. */
    internal::IServiceEndpoint*
    consumerOf(uint16_t const serviceId, uint16_t const instanceId) const
    {
        for (internal::IServiceEndpoint* const endpoint : _endpoints)
        {
            if ((endpoint->getServiceId() == serviceId)
                && (endpoint->getConsumedInstanceId() == instanceId))
            {
                return endpoint;
            }
        }
        return nullptr;
    }

    uint8_t _sourceClusterId;
    uint8_t _targetClusterId;
    ::etl::vector<internal::IServiceEndpoint*, MaxEndpoints> _endpoints{};

    // One list per service and direction, pointed at by that service's container.
    ::etl::array<::etl::vector<::middleware::core::TransceiverBase*, MaxTransceivers>, MaxEndpoints>
        _proxyLists{};
    ::etl::array<::etl::vector<::middleware::core::TransceiverBase*, MaxTransceivers>, MaxEndpoints>
        _skeletonLists{};
    ::etl::vector<::middleware::core::meta::TransceiverContainer, MaxEndpoints>
        _proxyTransceivers{};
    ::etl::vector<::middleware::core::meta::TransceiverContainer, MaxEndpoints>
        _skeletonTransceivers{};
};

} // namespace middleware::someip
