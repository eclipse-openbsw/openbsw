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

#include "someip/NetworkChannel.h"
#include "someip/SomeIpConstants.h"

#include <etl/expected.h>
#include <etl/optional.h>
#include <etl/vector.h>
#include <cstdint>

namespace someip
{
class INetworkListener;

/**
 * Responsible for the network resources.
 */
class INetwork
{
protected:
    INetwork() = default;

public:
    INetwork(INetwork const&)            = delete;
    INetwork& operator=(INetwork const&) = delete;

    virtual ~INetwork() = default;

    /**
     * Pure virtual fuction that sets listener for SD.
     *
     * \pre network is stopped.
     */
    virtual void setSdListener(INetworkListener& listener) = 0;

    /**
     * Pure virtual function that sets listener for RPC.
     *
     * \pre network is stopped.
     */
    virtual void setRpcListener(INetworkListener& listener) = 0;

    /**
     * Pure virtual function that configures a SD port.
     *
     * \pre network is stopped.
     *
     * \return true on success
     */
    virtual bool initSdPort(uint16_t port) = 0;

    /**
     * Pure virtual function that configures a UDP port.
     *
     * If stopped, the port will be configured for being opened on start.
     * If started, the port will be opened immediately.
     *
     * \return true on success
     */
    virtual bool initUdpPort(uint16_t port) = 0;

    /**
     * Pure virtual function that closes a UDP port.
     *
     * \pre network is started.
     */
    virtual void shutdownUdpPort(uint16_t port) = 0;

    /**
     * Pure virtual function that configures a TCP port.
     *
     * If stopped, the port will be configured for being opened on start.
     * If started, the port will be opened immediately.
     *
     * \return true on success
     */
    virtual bool initTcpPort(uint16_t port) = 0;

    /**
     * Pure virtual function that configures
     * a TCP port with external buffering for reassembling received data.
     *
     * The quantity of buffers shall be equal to the max number of expected clients for the
     * particular 'port'.
     * If stopped, the port will be configured for being opened on start.
     * If started, the port will be opened immediately.
     *
     * \return true on success
     */
    virtual bool
    initTcpPortWithExternalBuffers(uint16_t port, ::etl::ivector<::etl::span<uint8_t>>& buffers)
        = 0;

    /**
     * Pure virtual function that closes a TCP port.
     *
     * \pre network is started.
     */
    virtual void shutdownTcpPort(uint16_t port) = 0;

    /**
     * Pure virtual function that provides information whether
     * the network is started.
     */
    virtual bool isStarted() const = 0;

    /**
     * Pure virtual function that starts the network by opening all
     * configured network resources.
     *
     * Opens all configured network resources.
     *
     * \return true on success
     */
    virtual bool start() = 0;

    /**
     * Pure virtual function that stops the network by stopping all
     * configured network resources.
     */
    virtual void stop() = 0;

    /**
     * Pure virtual function that returns multicast ip.
     */
    virtual ::ip::IPAddress const& getMulticastIp() const = 0;

    /**
     * Pure virtual function that returns local ip.
     */
    virtual ::ip::IPAddress const& getLocalIp() const = 0;

    /**
     * Pure virtual function that returns subnet id.
     */
    virtual uint8_t getSubnetId() const = 0;

    /**
     * Pure virtual function that returns SD port.
     */
    virtual ::etl::expected<uint16_t, PortError> getSdPort(bool multicast = true) const = 0;

    /**
     * Pure virtual function that returns a SD channel.
     *
     * \note Succeeds if a SD proxy is preconfigured and open for this localPort.
     *
     * \param localPort the channel is associated with.
     * \param remoteEndpoint the channel is associated with.
     *
     * \return an optional channel.
     */
    virtual ::etl::optional<NetworkChannel>
    getSdChannel(uint16_t localPort, ::ip::IPEndpoint const& remoteEndpoint) const = 0;

    /**
     * Pure virtual function that opens a RPC channel for a remoteEndpoint.
     *
     * \note Succeeds if a RPC proxy is currently open and associated with this localPort.
     *
     * \param localPort the channel is associated with.
     * \param remoteEndpoint the channel is associated with.
     * \param proto the protocol used for this channel.
     *
     * \return an optional channel.
     */
    virtual ::etl::optional<NetworkChannel>
    getRpcChannel(uint16_t localPort, ::ip::IPEndpoint const& remoteEndpoint, uint8_t proto) const
        = 0;

    /**
     * Pure virtual function that opens a UDP RPC channel for a remoteEndpoint.
     *
     * \note Succeeds if a UDP RPC proxy is already open and associated with this localPort or a new
     * one could be opened.
     *
     * \param localPort the channel is associated with.
     * \param remoteEndpoint the channel is associated with.
     *
     * \return an optional channel.
     */
    virtual ::etl::optional<NetworkChannel>
    openUdpChannel(uint16_t localPort, ::ip::IPEndpoint const& remoteEndpoint) = 0;

    /**
     * Pure virtual function that opens a TCP RPC channel for a remoteEndpoint.
     *
     * \note Succeeds if a new TCP RPC proxy could be opened for the remoteEndpoint.
     *
     * \param localPort the channel is associated with.
     * \param remoteEndpoint the channel is associated with.
     *
     * \return an optional channel, which will be associated with the remote port if successfully.
     */
    virtual ::etl::optional<NetworkChannel>
    openTcpChannel(uint16_t localPort, ::ip::IPEndpoint const& remoteEndpoint) = 0;

    /**
     * Pure virtual function that opens a TCP RPC channel for a remoteEndpoint.
     *
     * \note Succeeds if a new TCP RPC proxy could be opened for the remoteEndpoint.
     *
     * \param localPort the channel is associated with.
     * \param remoteEndpoint the channel is associated with.
     * \param buffer the external buffer to be used for an incoming tcp data.
     *
     * \return an optional channel, which will be associated with the remote port if successfully.
     */
    virtual ::etl::optional<NetworkChannel> openTcpChannelWithExternalReassembleBuffer(
        uint16_t localPort, ::ip::IPEndpoint const& remoteEndpoint, ::etl::span<uint8_t> buffer)
        = 0;

    // INTERFACE_END
};

} // namespace someip
