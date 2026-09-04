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

#include "someip/IEventReceiver.h"
#include "someip/IEventSender.h"
#include "someip/INetwork.h"
#include "someip/IRpcHandler.h"
#include "someip/IRpcSender.h"
#include "someip/IServiceRegistry.h"
#include "someip/ISomeIpStack.h"

#include <cstdint>

namespace someip
{
/**
 * Contains the thread-save public API of the SOME/IP Stack.
 */
class SomeIpStack : public ISomeIpStack
{
public:
    SomeIpStack(
        INetwork& network,
        IRpcSender& rpcSender,
        IEventReceiver& eventReceiver,
        IEventSender& eventSender,
        IServiceRegistry& registry);

    SomeIpStack(SomeIpStack const&)            = delete;
    SomeIpStack& operator=(SomeIpStack const&) = delete;

    /* INTERNAL */

    INetwork& getNetwork() const { return _network; }

    IRpcSender& getRpcSender() const { return _rpcSender; }

    IEventSender& getEventSender() const { return _eventSender; }

    /* CONFIG */

    bool initSdPort(uint16_t port) override;
    bool initUdpPort(uint16_t port) override;
    bool initTcpPort(uint16_t port) override;

    /* LIFECYCLE */

    virtual bool isInitialized() const;
    virtual bool init();
    virtual void shutdown();

    virtual bool isStarted() const;
    virtual bool start();
    virtual void stop();

    /* CLIENT */

    bool registerServiceQuery(ServiceQuery& query) override;
    void unregisterServiceQuery(ServiceQuery& query) override;

    void addEventListener(IEventListener& listener) override;
    void removeEventListener(IEventListener& listener) override;

    /* SERVER */

    bool registerProvidedService(ProvidedService& service) override;
    void unregisterProvidedService(ProvidedService& service) override;

protected:
    virtual bool doInit()     = 0;
    virtual bool doStart()    = 0;
    virtual void doStop()     = 0;
    virtual void doShutdown() = 0;

private:
    enum class State : uint8_t
    {
        UNDEFINED,
        STOPPED,
        STARTED
    };

    INetwork& _network;
    IRpcSender& _rpcSender;
    IEventReceiver& _eventReceiver;
    IEventSender& _eventSender;

    State _state;

protected:
    IServiceRegistry& _serviceRegistry;
};
} // namespace someip
