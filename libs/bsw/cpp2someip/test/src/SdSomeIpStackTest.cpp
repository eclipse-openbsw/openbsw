/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdSomeIpStack.h"

#include "someip/EventListenerMock.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/SomeIpConstants.h"

#include <tcp/socket/AbstractServerSocketMock.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::ip;
using namespace ::testing;

using namespace ::someip;
using tcp::AbstractServerSocketMock;

struct SdSomeIpStackTest : ::testing::Test
{
    SdSomeIpStackTest() = default;

    TcpServer* _server = nullptr;
    ::someip::declare::SdSomeIpStack<
        ::udp::AbstractDatagramSocketMock,
        1U,
        ::tcp::AbstractServerSocketMock,
        1U,
        ::tcp::AbstractSocketMock,
        1U,
        1500,
        0U,
        false,
        1U,
        16U,
        16U,
        16U,
        16U,
        16U,
        10U,
        1U, // NumEventBuffers
        1U> // NumMulticastReceptions
        _stack{
            ::ip::make_ip4(224U, 1U, 255U, 255U),
            ::ip::make_ip4(192U, 0U, 2U, 0U), // SLAVE_IP
            25U,                              // SUBNET_ID
            _ethernetContext};
    udp::AbstractDatagramSocketMock _rpcSocketMock;
    async::ContextType _ethernetContext{0U};
};

/**
 * Test process of registering and unregistering of ProvidedService to SdSomeIpStack.
 */
TEST_F(SdSomeIpStackTest, register_and_unregister_ProvidedService)
{
    service_id::type const serviceId       = 1U;
    instance_id::type const instanceId     = 2U;
    major_version::type const majorVersion = 3U;
    eventgroup_id::type const eventGroup   = 4U;
    ::someip::ttl::type const ttl          = 5U;
    port::type const port                  = 6U;
    proto::type const proto                = proto::SD_L4_PROTO_UDP;

    StrictMock<someip::ServiceHandlerMock<1U>> handler;

    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.majorVersion = majorVersion;
    service.description.eventGroup   = eventGroup;
    service.description.ttl          = ttl;
    service.description.port         = port;
    service.description.proto        = proto;

    _stack.registerProvidedService(service);
    _stack.unregisterProvidedService(service);
}

/**
 * Test process of registering and unregistering of ServiceQuery to SdSomeIpStack.
 */
TEST_F(SdSomeIpStackTest, register_and_unregister_ServiceQuery)
{
    service_id::type const serviceId       = 0x1234U;
    major_version::type const majorVersion = 0x01U;
    minor_version::type const minorVersion = 0U;

    auto serviceQuery                  = make<ServiceQuery>();
    serviceQuery.description.serviceId = serviceId;
    serviceQuery.description.instanceId
        = static_cast<::someip::instance_id::type>(::someip::instance_id::ANY);
    serviceQuery.description.majorVersion = majorVersion;
    serviceQuery.description.minorVersion = minorVersion;
    serviceQuery.listener                 = nullptr;

    _stack.registerServiceQuery(serviceQuery);
    _stack.unregisterServiceQuery(serviceQuery);
}

/**
 * Test process of adding and removing of EventListener to SdSomeIpStack.
 */
TEST_F(SdSomeIpStackTest, add_and_remove_EventListener)
{
    ::someip::EventListenerMock eventListener;
    _stack.addEventListener(eventListener);
    _stack.removeEventListener(eventListener);
}

} // namespace
