/********************************************************************************
 * Copyright (c) 2026 KPIT Group
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/SampleConsumer.h"
#include "middleware/SampleProvider.h"

#include <async/Async.h>
#include <async/IRunnable.h>
#include <async/Types.h>
#include <middleware/someip/SomeIpBridge.h>
#include <someip/IEventSender.h>
#include <someip/INetwork.h>
#include <someip/IRpcSender.h>
#include <someip/ISomeIpStack.h>
#include <someip/ProvidedService.h>

#include <cstdint>

namespace middlewaresample
{

/**
 * Carries three members of the FLYNC model's service over SOME/IP with the
 * middleware SOME/IP bridge: echoUINT8, the TestFieldUINT8 field and the
 * TestEventUINT8Multicast event. SampleProvider serves this node's instance
 * through the generated skeleton; SampleConsumer uses the peer node's instance
 * through the generated proxy.
 *
 * This is the deployment wiring the bridge library leaves to the application:
 * the service's ids, instances, port, SD timing and multicast group, which
 * members go on SOME/IP and how, and which cluster connection the bridge takes
 * over.
 *
 * It runs on SomeIpSystem's stack and task: init() is called before the stack
 * starts, run() and shutdown() from SomeIpSystem's own.
 */
class SampleSomeIp final : public ::async::IRunnable
{
public:
    /// Port the service is offered on, and the peer's instance is consumed from.
    static constexpr uint16_t PORT = 30555U;

    SampleSomeIp(
        ::someip::ISomeIpStack& stack,
        ::someip::IEventSender& eventSender,
        ::someip::INetwork& network,
        ::someip::IRpcSender& rpcSender);

    /**
     * Binds the members, offers the service and its eventgroups, and redirects the
     * cluster connection to the bridge, so the skeleton and the proxy are
     * initialized on it.
     */
    void init();

    /**
     * Starts the cycle on the stack's task. The first tick initializes the
     * skeleton and the proxy; every later one sends an event, then, once SD has
     * found the peer's instance, one request to it.
     */
    void run(::async::ContextType context);

    void shutdown();

    /// \see ::async::IRunnable::execute()
    void execute() override;

private:
    static constexpr uint8_t MAX_MEMBERS = 10U;
    static constexpr uint8_t MAX_PENDING = 1U;
    static constexpr uint32_t CYCLE_MS   = 1000U;

    using Endpoint   = ::middleware::someip::SomeIpServiceEndpoint<MAX_MEMBERS, MAX_PENDING>;
    using Connection = ::middleware::someip::SomeIpConnection<1U, 1U>;
    template<typename T>
    using Slot = ::middleware::someip::internal::PayloadSlot<T>;

    void bindMembers();
    void offerEventGroup(::someip::ProvidedService& service, uint16_t eventGroupId);

    ::someip::ISomeIpStack& _stack;
    Endpoint _endpoint;
    Connection _connection;
    ::someip::ProvidedService _eventGroupAll;
    ::someip::ProvidedService _eventGroupMulticast;
    SampleProvider _provider;
    SampleConsumer _consumer;
    ::async::TimeoutType _timeout;
    bool _isInitialized{false};

    // One slot per member and direction, for this node's instance and the peer's.
    Slot<uint8_t> _echoRequest;
    Slot<uint8_t> _echoResponse;
    Slot<uint8_t> _fieldNotification;
    Slot<uint8_t> _fieldGetResponse;
    Slot<uint8_t> _fieldSetRequest;
    Slot<uint8_t> _fieldSetResponse;
    Slot<uint8_t> _event;
    Slot<uint8_t> _peerEchoRequest;
    Slot<uint8_t> _peerEchoResponse;
    Slot<uint8_t> _peerFieldNotification;
    Slot<uint8_t> _peerFieldGetResponse;
    Slot<uint8_t> _peerFieldSetRequest;
    Slot<uint8_t> _peerFieldSetResponse;
    Slot<uint8_t> _peerEvent;
};

} // namespace middlewaresample
