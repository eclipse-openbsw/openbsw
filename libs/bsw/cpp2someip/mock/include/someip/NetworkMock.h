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

#include "someip/INetwork.h"
#include "someip/INetworkListener.h"
#include "someip/SomeIpConstants.h"

#include <gmock/gmock.h>

namespace someip
{
class NetworkMock : public INetwork
{
public:
    MOCK_METHOD(void, setSdListener, (INetworkListener & listener));
    MOCK_METHOD(void, setRpcListener, (INetworkListener & listener));

    MOCK_METHOD(bool, initSdPort, (port::type port));
    MOCK_METHOD(bool, initUdpPort, (port::type port));
    MOCK_METHOD(void, shutdownUdpPort, (port::type port));
    MOCK_METHOD(bool, initTcpPort, (port::type port));
    MOCK_METHOD(
        bool,
        initTcpPortWithExternalBuffers,
        (port::type port, ::etl::ivector<::etl::span<uint8_t>>&));
    MOCK_METHOD(void, shutdownTcpPort, (port::type port));

    MOCK_METHOD(bool, isStarted, (), (const));
    MOCK_METHOD(bool, start, ());
    MOCK_METHOD(void, stop, ());

    MOCK_METHOD(::ip::IPAddress const&, getMulticastIp, (), (const));
    MOCK_METHOD(::ip::IPAddress const&, getLocalIp, (), (const));
    MOCK_METHOD(uint8_t, getSubnetId, (), (const));
    MOCK_METHOD(
        (::etl::expected<port::type, PortError>), getSdPort, (bool multicast), (const, override));

    MOCK_METHOD(
        ::etl::optional<NetworkChannel>,
        getSdChannel,
        (port::type localPort, ::ip::IPEndpoint const& remoteEndpoint),
        (const));
    MOCK_METHOD(
        ::etl::optional<NetworkChannel>,
        getRpcChannel,
        (port::type localPort, ::ip::IPEndpoint const& remoteEndpoint, proto::type proto),
        (const));
    MOCK_METHOD(
        ::etl::optional<NetworkChannel>,
        openUdpChannel,
        (port::type localPort, ::ip::IPEndpoint const& remoteEndpoint));
    MOCK_METHOD(
        ::etl::optional<NetworkChannel>,
        openTcpChannel,
        (port::type localPort, ::ip::IPEndpoint const& remoteEndpoint));
    MOCK_METHOD(
        ::etl::optional<NetworkChannel>,
        openTcpChannelWithExternalReassembleBuffer,
        (port::type localPort, ::ip::IPEndpoint const& remoteEndpoint, ::etl::span<uint8_t>));
};

} // namespace someip
