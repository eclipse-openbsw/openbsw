/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkConfig.h"

#include "TestConstants.h"

#include <tcp/socket/AbstractServerSocketMock.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::udp;
using namespace ::tcp;
using namespace ::someip;
using namespace ::someip::declare;
using namespace ::test;

static uint8_t const NUM_UDP_SOCKETS = 2U;
static uint8_t const NUM_TCP_SERVERS = 1U;
static uint8_t const NUM_TCP_SOCKETS = 2U;
static uint8_t const NUM_TP_STREAMS  = 2U;

/**
 * Test correctness of SdNetworkConfig initialization.
 */
TEST(NetworkConfig, test_correct_SdConfig_initialization)
{
    SdNetworkConfig<
        AbstractDatagramSocketMock,
        NUM_UDP_SOCKETS,
        AbstractServerSocketMock,
        NUM_TCP_SERVERS,
        AbstractSocketMock,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE>
        config(IPV4_MULTICAST_IP, IPV4_LOCAL_IP_1, IPV4_SUBNET_ID);

    EXPECT_EQ(IPV4_MULTICAST_IP, config._multicastIp);
    EXPECT_EQ(IPV4_LOCAL_IP_1, config._localIp);
    EXPECT_EQ(IPV4_SUBNET_ID, config._subnetId);

    UdpConfig& udpConfig = config._udpConfig;

    EXPECT_EQ(1U, udpConfig._sdProxies.getSize());
    EXPECT_EQ(NUM_UDP_SOCKETS, udpConfig._rpcProxies.getSize());

    TcpConfig& tcpConfig = config._tcpConfig;

    EXPECT_EQ(NUM_TCP_SERVERS, tcpConfig._tcpServers.getSize());
    EXPECT_EQ(NUM_TCP_SOCKETS, tcpConfig._tcpProxies.getSize());
}

/**
 * Test correctness of SdNetworkConfig initialization with TP.
 */
TEST(NetworkConfig, test_correct_SdConfig_initialization_with_tp)
{
    SdNetworkConfig<
        AbstractDatagramSocketMock,
        NUM_UDP_SOCKETS,
        AbstractServerSocketMock,
        NUM_TCP_SERVERS,
        AbstractSocketMock,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE,
        NUM_TP_STREAMS>
        config(IPV4_MULTICAST_IP, IPV4_LOCAL_IP_1, IPV4_SUBNET_ID);

    TpConfig& tpConfig = config._tpConfig;

    EXPECT_EQ(1U, tpConfig.tpSenders.size());
    EXPECT_EQ(NUM_TP_STREAMS, tpConfig.tpReceivers.size());
}

/**
 * Test correctness of SdNetworkConfig initialization with EndpointOption.
 */
TEST(NetworkConfig, test_correct_SdConfig_initialization_with_EndpointOption)
{
    SdNetworkConfig<
        AbstractDatagramSocketMock,
        NUM_UDP_SOCKETS,
        AbstractServerSocketMock,
        NUM_TCP_SERVERS,
        AbstractSocketMock,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE,
        0U,
        true>
        config(IPV4_MULTICAST_IP, IPV4_LOCAL_IP_1, IPV4_SUBNET_ID);

    UdpConfig& udpConfig = config._udpConfig;
    EXPECT_EQ(2U, udpConfig._sdProxies.getSize());
}

/**
 * Test correctness of RpcNetworkConfig initialization.
 */
TEST(NetworkConfig, test_correct_RpcConfig_initialization)
{
    RpcNetworkConfig<
        AbstractDatagramSocketMock,
        NUM_UDP_SOCKETS,
        AbstractServerSocketMock,
        NUM_TCP_SERVERS,
        AbstractSocketMock,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE>
        config(IPV4_LOCAL_IP_1, IPV4_SUBNET_ID);

    EXPECT_EQ(NetworkResource::INVALID_IP, config._multicastIp);
    EXPECT_EQ(IPV4_LOCAL_IP_1, config._localIp);
    EXPECT_EQ(IPV4_SUBNET_ID, config._subnetId);

    UdpConfig& udpConfig = config._udpConfig;

    EXPECT_EQ(0U, udpConfig._sdProxies.getSize());
    EXPECT_EQ(NUM_UDP_SOCKETS, udpConfig._rpcProxies.getSize());

    TcpConfig& tcpConfig = config._tcpConfig;

    EXPECT_EQ(NUM_TCP_SERVERS, tcpConfig._tcpServers.getSize());
    EXPECT_EQ(NUM_TCP_SOCKETS, tcpConfig._tcpProxies.getSize());
}

/**
 * Test correctness of RpcNetworkConfig initialization with TP.
 */
TEST(NetworkConfig, test_correct_RpcConfig_initialization_with_tp)
{
    RpcNetworkConfig<
        AbstractDatagramSocketMock,
        NUM_UDP_SOCKETS,
        AbstractServerSocketMock,
        NUM_TCP_SERVERS,
        AbstractSocketMock,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE,
        NUM_TP_STREAMS>
        config(IPV4_LOCAL_IP_1, IPV4_SUBNET_ID);

    TpConfig& tpConfig = config._tpConfig;

    EXPECT_EQ(1U, tpConfig.tpSenders.size());
    EXPECT_EQ(NUM_TP_STREAMS, tpConfig.tpReceivers.size());
}
} // anonymous namespace
