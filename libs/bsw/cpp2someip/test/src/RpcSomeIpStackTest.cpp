/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcSomeIpStack.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
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

struct RpcSomeIpStackTest : ::testing::Test
{
    RpcSomeIpStackTest()
    {
        _networkConfig._udpConfig._rpcProxies.getProxy(0U)->setSocket(_rpcSocketMock);
        _server = _networkConfig._tcpConfig._tcpServers.getServer(0U);
    }

    ::someip::declare::RpcNetworkConfig<
        ::udp::AbstractDatagramSocketMock,
        1U, // NumUdpRpcSockets
        ::tcp::AbstractServerSocketMock,
        1U, // NumTcpServerSockets
        ::tcp::AbstractSocketMock,
        1U,    // NumTcpRpcSockets
        1500U> // BUFFER_SIZE
        _networkConfig{
            make_ip4(224U, 1U, 255U, 255U),
            25U // SUBNET_ID
        };

    TcpServer* _server;
    async::ContextType _ethernetContext{0U};
    declare::RpcSomeIpStack<16U, 16U, 16U, 10U, 1U, 1U> _stack{_ethernetContext, _networkConfig};
    udp::AbstractDatagramSocketMock _rpcSocketMock;
    StrictMock<SystemTimerMock> _stm;
};

/**
 * Make sure RpcSomeIpStack is initialized correctly.
 */
TEST_F(RpcSomeIpStackTest, init_RpcSomeIpStack)
{
    _stack.initSdPort(30490U);
    _stack.initUdpPort(30501U);
    _stack.initTcpPort(30501U);
    _stack.init();

    EXPECT_FALSE(_stack.isStarted());
    EXPECT_CALL(_rpcSocketMock, bind(_, _))
        .WillRepeatedly(Return(udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(_rpcSocketMock, join(_))
        .WillRepeatedly(Return(udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(_rpcSocketMock, isBound()).WillRepeatedly(Return(false));

    auto& tcpSocket     = _server->getServerSocket();
    auto& tcpSocketMock = dynamic_cast<AbstractServerSocketMock&>(tcpSocket);
    EXPECT_CALL(tcpSocketMock, isClosed()).WillRepeatedly(Return(true));
    EXPECT_CALL(tcpSocketMock, bind(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(tcpSocketMock, accept()).WillRepeatedly(Return(true));
    EXPECT_TRUE(_stack.start());
    EXPECT_TRUE(_stack.isStarted());
    EXPECT_TRUE(_stack.isInitialized());

    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(1);

    _stack.stop();
    _stack.shutdown();

    EXPECT_FALSE(_stack.isInitialized());
}

/**
 * Test process of adding and removing remote service to RpcSomeIpStack.
 */
TEST_F(RpcSomeIpStackTest, add_and_remove_remote_service)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 2U;
    major_version::type majorVersion = 3U;
    eventgroup_id::type eventGroup   = 4U;
    ttl::type ttl                    = 5U;
    uint16_t port                    = 6U;
    uint8_t proto                    = static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_UDP);

    StrictMock<someip::ServiceHandlerMock<1U>> handler;

    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.majorVersion = majorVersion;
    service.description.eventGroup   = eventGroup;
    service.description.ttl          = ttl;
    service.description.port         = port;
    service.description.proto        = proto;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());
    _stack.addRemoteService(service.description);
    _stack.removeRemoteService(service.description);
}

/**
 * Test process of adding and removing subscriptions to RpcSomeIpStack.
 */
TEST_F(RpcSomeIpStackTest, add_and_remove_subscription)
{
    service_id::type serviceId       = 1U;
    instance_id::type instanceId     = 4U;
    major_version::type majorVersion = 3U;
    eventgroup_id::type eventGroup   = 4U;
    ttl::type ttl                    = 5U;
    uint16_t port                    = 6U;
    uint8_t proto                    = static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_UDP);

    StrictMock<someip::ServiceHandlerMock<1U>> handler;

    ProvidedService service(handler);
    service.description.serviceId    = serviceId;
    service.description.instanceId   = instanceId;
    service.description.majorVersion = majorVersion;
    service.description.eventGroup   = eventGroup;
    service.description.ttl          = ttl;
    service.description.port         = port;
    service.description.proto        = proto;

    _stack.addSubscription(service.description);
    _stack.removeSubscription(service.description);
}

} // anonymous namespace
