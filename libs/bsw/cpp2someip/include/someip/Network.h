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
#include "someip/NetworkConfig.h"
#include "someip/SomeIpConstants.h"

#include <etl/vector.h>

namespace someip
{
class UdpProxy;
class TcpProxy;
class TcpServer;

class Network : public INetwork
{
public:
    explicit Network(NetworkConfig const& config);

    /**
     * Function that sets SDListener if network is not started.
     */
    void setSdListener(INetworkListener& listener) final;

    /**
     * Function that sets RpcListener if network is not started.
     */
    void setRpcListener(INetworkListener& listener) final;

    /**
     * Function that initializes SD port if network is not started.
     */
    bool initSdPort(port::type port) final;

    /**
     * Function that initializes UDP port if network is not started.
     */
    bool initUdpPort(port::type port) final;

    /**
     * Function that shuts down UDP port if network is not started.
     */
    void shutdownUdpPort(port::type port) final;

    /**
     * Function that ensures TCP server is open and initializes TCP port if network is not started.
     */
    bool initTcpPort(port::type port) final;

    /**
     * Function that tries to set external buffer and ensures
     * TCP server is open and initializes TCP port if network is not started.
     */
    bool initTcpPortWithExternalBuffers(
        uint16_t port, ::etl::ivector<::etl::span<uint8_t>>& buffers) final;

    /**
     * Function that closes TCP server associated with port if network is not started.
     */
    void shutdownTcpPort(port::type port) final;

    /**
     * Function that returns whether network is started.
     */
    bool isStarted() const final;

    /**
     * Lifecycle function that starts necessary SD, UDP and TCP resources if possible.
     */
    bool start() final;

    /**
     * Lifecycle function that stops usage of SD, UDP and TCP resources.
     */
    void stop() final;

    /**
     * Function that returns multicast IP address.
     */
    ::ip::IPAddress const& getMulticastIp() const final;

    /**
     * Function that returns local IP address.
     */
    ::ip::IPAddress const& getLocalIp() const final;

    /**
     * Function that returns subnet ID.
     */
    uint8_t getSubnetId() const final;

    /**
     * Function that returns SD port if possible.
     */
    ::etl::expected<uint16_t, PortError> getSdPort(bool multicast) const final;

    /**
     * Function that returns SD channel associated with passed
     * local port and remote Endpoint if possible.
     */
    ::etl::optional<NetworkChannel>
    getSdChannel(port::type localPort, ::ip::IPEndpoint const& remoteEndpoint) const final;

    /**
     * Function that returns RPC channel associated with passed
     * local port, remote Endpoint and proto if possible.
     */
    ::etl::optional<NetworkChannel> getRpcChannel(
        port::type, ::ip::IPEndpoint const& remoteEndpoint, proto::type proto) const final;

    /**
     * Function that opens UDP channel associated with passed
     * local port and remote Endpoint if possible.
     */
    ::etl::optional<NetworkChannel>
    openUdpChannel(port::type localPort, ::ip::IPEndpoint const& remoteEndpoint) final;

    /**
     * Function that opens TCP channel associated with passed local port
     * and remote Endpoint if possible using internal buffer.
     */
    ::etl::optional<NetworkChannel>
    openTcpChannel(port::type localPort, ::ip::IPEndpoint const& remoteEndpoint) final;

    /**
     * Function that opens TCP channel associated
     * with passed local port and remote Endpoint if possible using external buffer.
     */
    ::etl::optional<NetworkChannel> openTcpChannelWithExternalReassembleBuffer(
        port::type localPort,
        ::ip::IPEndpoint const& remoteEndpoint,
        ::etl::span<uint8_t> buffer) final;

private:
    bool startSd() const;
    void stopSd() const;

    bool startUdp() const;
    bool openUdp(port::type port) const;
    void stopUdp() const;

    bool openLocalUdpProxy(UdpProxy& proxy, port::type port) const;

    bool startTcp() const;
    bool openTcp(port::type port, TcpServer& server) const;
    void stopTcp() const;

    TcpProxy* getTcpProxy(port::type localPort, ::ip::IPEndpoint const& remoteEndpoint) const;

    TcpServer* getIdleServer() const;

    NetworkConfig const& _config;
    bool _started;
};

} // namespace someip
