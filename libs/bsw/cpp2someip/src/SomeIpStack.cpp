/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpStack.h"

#include "someip/INetwork.h"
#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

SomeIpStack::SomeIpStack(
    INetwork& network,
    IRpcSender& rpcSender,
    IEventReceiver& eventReceiver,
    IEventSender& eventSender,
    IServiceRegistry& registry)
: _network(network)
, _rpcSender(rpcSender)
, _eventReceiver(eventReceiver)
, _eventSender(eventSender)
, _state(State::UNDEFINED)
, _serviceRegistry(registry)
{}

bool SomeIpStack::initSdPort(uint16_t const port)
{
    if (isInitialized())
    {
        return false;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::initSdPort(%d)", port);

    return _network.initSdPort(port);
}

bool SomeIpStack::initUdpPort(uint16_t const port)
{
    if (isInitialized())
    {
        return false;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::initUdpPort(%d)", port);

    return _network.initUdpPort(port);
}

bool SomeIpStack::initTcpPort(uint16_t const port)
{
    if (isInitialized())
    {
        return false;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::initTcpPort(%d)", port);

    return _network.initTcpPort(port);
}

bool SomeIpStack::isInitialized() const { return _state != State::UNDEFINED; }

bool SomeIpStack::isStarted() const { return _state == State::STARTED; }

bool SomeIpStack::init()
{
    if (isInitialized())
    {
        return true;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::init()");

    if (doInit())
    {
        _state = State::STOPPED;
        return true;
    }

    ERROR_LOG(SOMEIP, "SomeIpStack::init() failed");
    return false;
}

bool SomeIpStack::start()
{
    if (!isInitialized())
    {
        if (!init())
        {
            return false;
        }
    }

    if (isStarted())
    {
        return true;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::start()");

    if (doStart())
    {
        _state = State::STARTED;
        return true;
    }

    ERROR_LOG(SOMEIP, "SomeIpStack::start() failed");
    return false;
}

void SomeIpStack::stop()
{
    if (!isStarted())
    {
        return;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::stop()");

    doStop();

    _state = State::STOPPED;
}

void SomeIpStack::shutdown()
{
    if (isStarted())
    {
        stop();
    }

    if (!isInitialized())
    {
        return;
    }

    INFO_LOG(SOMEIP, "SomeIpStack::shutdown()");

    doShutdown();

    _state = State::UNDEFINED;
}

bool SomeIpStack::registerServiceQuery(ServiceQuery& query)
{
    return _serviceRegistry.registerServiceQuery(query);
}

void SomeIpStack::unregisterServiceQuery(ServiceQuery& query)
{
    _serviceRegistry.unregisterServiceQuery(query);
}

void SomeIpStack::addEventListener(IEventListener& listener)
{
    _eventReceiver.addEventListener(listener);
}

void SomeIpStack::removeEventListener(IEventListener& listener)
{
    _eventReceiver.removeEventListener(listener);
}

bool SomeIpStack::registerProvidedService(ProvidedService& service)
{
    return _serviceRegistry.registerProvidedService(service);
}

void SomeIpStack::unregisterProvidedService(ProvidedService& service)
{
    _serviceRegistry.unregisterProvidedService(service);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
