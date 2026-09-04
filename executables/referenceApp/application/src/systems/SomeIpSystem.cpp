/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/SomeIpSystem.h"

#include "async/Types.h"
#include "ethConfig.h"
#include "ip/IPAddress.h"
#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"

#include <etl/tuple.h>

etl::tuple<uint16_t, ::someip::PortRangeReturnCode>
computeNextLocalPort(uint16_t const requestedPort, uint16_t const)
{
    return etl::make_tuple(requestedPort, ::someip::PortRangeReturnCode::OK);
}

namespace systems
{

namespace
{

static constexpr auto multicastIp     = ::ip::make_ip4(225, 0, 0, 1);
static constexpr auto remoteServiceIp = ::ip::make_ip4(192, 168, 0, 20);

} // namespace

SomeIpSystem::SomeIpSystem(::async::ContextType const context)
: _timeout()
, _context(context)
, _stack(multicastIp, ::eth0::IP_ADDRESS, uint8_t{}, _context)
, _methodCallbacks()
, _providedService()
, _providedServiceHandler(_methodCallbacks, _stack.getEventTransceiver())
, _eventListener()
, _consumedServiceQuery()
, _rpcChannel(_stack.getNetwork(), _stack.getRpcHandler())
, _serviceListener(_rpcChannel)
, _routine(_rpcChannel)
{}

void SomeIpSystem::init()
{
    _stack.initSdPort(SD_PORT);
    _stack.initUdpPort(SERVICE_PORT);
    _stack.initUdpPort(CLIENT_PORT);
    _stack.init();
    _stack.addEventListener(_eventListener);

    /*
     * Register the service {{
     */

    // event group
    ::someip::ServiceDescription desc{
        0U,
        10U,
        0xCAFEU,
        1U,
        0x8001,
        ::eth0::IP_ADDRESS,
        SERVICE_PORT,
        ::someip::proto::SD_L4_PROTO_UDP,
        1U};

    _providedServiceEg.setHandler(_providedServiceHandler);
    _providedServiceEg.description = desc;
    _stack.registerProvidedService(_providedServiceEg);

    // service
    desc
        = {0U,
           10U,
           0xCAFEU,
           1U,
           ::someip::eventgroup_id::ALL,
           ::eth0::IP_ADDRESS,
           SERVICE_PORT,
           ::someip::proto::SD_L4_PROTO_UDP,
           1U};

    _providedService.setHandler(_providedServiceHandler);
    _providedService.description = desc;
    _stack.registerProvidedService(_providedService);

    /*
     * }} register the service.
     */

    /*
     * Register the client {{
     */

    // event group
    desc
        = {0U,
           1U,
           0xBABEU,
           1U,
           0x8001,
           remoteServiceIp,
           CLIENT_PORT,
           ::someip::proto::SD_L4_PROTO_UDP,
           1U};

    _consumedServiceQueryEg.description = desc;
    _consumedServiceQueryEg.listener    = &_serviceListener;
    _stack.registerServiceQuery(_consumedServiceQueryEg);

    // client
    desc
        = {0U,
           1U,
           0xBABEU,
           1U,
           ::someip::eventgroup_id::ALL,
           remoteServiceIp,
           CLIENT_PORT,
           ::someip::proto::SD_L4_PROTO_UDP,
           1U};

    _consumedServiceQuery.description = desc;
    _consumedServiceQuery.listener    = &_serviceListener;
    _stack.registerServiceQuery(_consumedServiceQuery);

    /*
     * }} register the client
     */

    _stack.start();
    transitionDone();
}

void SomeIpSystem::run()
{
    ::async::scheduleAtFixedRate(
        _context, _routine, _timeout, 100, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void SomeIpSystem::shutdown()
{
    _stack.shutdown();
    transitionDone();
}

} // namespace systems
