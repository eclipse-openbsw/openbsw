/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "async/Types.h"
#include "lifecycle/AsyncLifecycleComponent.h"
#include "lwipSocket/udp/LwipDatagramSocket.h"
#include "someip/EventListener.h"
#include "someip/PeriodicRoutine.h"
#include "someip/ProvidedService.h"
#include "someip/ProvidedServiceHandler.h"
#include "someip/RpcChannel.h"
#include "someip/SdSomeIpStack.h"
#include "someip/ServiceHandler.h"
#include "someip/ServiceListener.h"
#include "someip/ServiceQuery.h"
#include "someip/SocketDummies.h"

namespace systems
{

class SomeIpSystem final : public ::lifecycle::AsyncLifecycleComponent
{
public:
    explicit SomeIpSystem(::async::ContextType);

    SomeIpSystem(SomeIpSystem const&)                = delete;
    SomeIpSystem& operator=(SomeIpSystem const&)     = delete;
    SomeIpSystem(SomeIpSystem&&) noexcept            = delete;
    SomeIpSystem& operator=(SomeIpSystem&&) noexcept = delete;
    ~SomeIpSystem()                                  = default;

    void init() override;
    void run() override;
    void shutdown() override;

private:
    enum : uint32_t
    {
        ENDPOINT_NUM             = 1U,
        NUM_UDP_SOCKETS          = 2U,
        NUM_TCP_SERVERS          = 0U,
        NUM_TCP_SOCKETS          = 0U,
        BUFFER_SIZE              = 1500U,
        NUM_REMOTE_ENDPOINTS     = 1U,
        NUM_REMOTE_SERVICES      = 0U,
        NUM_REMOTE_SUBSCRIPTIONS = static_cast<uint32_t>(ENDPOINT_NUM),
        NUM_LOCAL_SERVICES
        = static_cast<uint32_t>(ENDPOINT_NUM * 2), // number of registerProvidedService calls
        NUM_LOCAL_QUERIES          = 2U,
        NUM_EVENT_BUFFERS          = 0U,
        NUM_MULTICAST_RECEPTIONS   = 2U,
        NUM_SUBSCRIPTION_ENDPOINTS = 2U,
        SD_PORT                    = 30490U,
        SERVICE_PORT               = 30501U,
        CLIENT_PORT                = 30502U
    };

    ::async::TimeoutType _timeout;
    ::async::ContextType _context;

    // actual generic part
    ::someip::declare::SdSomeIpStack<
        udp::LwipDatagramSocket,
        NUM_UDP_SOCKETS,
        ServerSocketDummy,
        NUM_TCP_SERVERS,
        SocketDummy,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE,
        0U,
        false,
        0U,
        NUM_REMOTE_ENDPOINTS,
        NUM_REMOTE_SERVICES,
        NUM_REMOTE_SUBSCRIPTIONS,
        NUM_LOCAL_SERVICES,
        NUM_LOCAL_QUERIES,
        NUM_SUBSCRIPTION_ENDPOINTS>
        _stack;

    /*
     * CallbackPool stores all request contexts that are yet to be handled.
     */
    using CallbackPool = ::etl::pool<::someip::ServiceHandler::RpcCallback, 1>;

    // service-related members {{
    CallbackPool _methodCallbacks;
    ::someip::ProvidedService _providedService;
    ::someip::ProvidedService _providedServiceEg;
    ::ProvidedServiceHandler<CallbackPool, SERVICE_PORT> _providedServiceHandler;
    // }} service-related members

    // client-related members {{
    EventListener _eventListener;
    ::someip::ServiceQuery _consumedServiceQuery;
    ::someip::ServiceQuery _consumedServiceQueryEg;
    ::someip::RpcChannel _rpcChannel;
    ServiceListener _serviceListener;
    PeriodicRoutine _routine;
    // }} client-related members
};

} // namespace systems
