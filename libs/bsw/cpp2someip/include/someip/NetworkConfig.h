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

#include "someip/NetworkResource.h"
#include "someip/TcpConfig.h"
#include "someip/TpConfig.h"
#include "someip/UdpConfig.h"

#include <etl/array.h>
#include <cstdint>

namespace someip
{
/**
 * The network configuration.
 */
class NetworkConfig
{
public:
    NetworkConfig(NetworkConfig const&)            = delete;
    NetworkConfig& operator=(NetworkConfig const&) = delete;

    ::ip::IPAddress _multicastIp;
    ::ip::IPAddress _localIp;

    UdpConfig& _udpConfig;
    TcpConfig& _tcpConfig;
    TpConfig& _tpConfig;

    uint8_t _subnetId;
    bool _useMagicCookie;

protected:
    NetworkConfig(
        ::ip::IPAddress const& multicastIp,
        ::ip::IPAddress const& localIp,
        uint8_t subnetId,
        UdpConfig& udpConfig,
        TcpConfig& tcpConfig,
        TpConfig& tpConfig);

    ~NetworkConfig() = default;
};

namespace internal
{
/**
 * Internal network resources.
 */
template<
    class UdpSocketType,
    uint8_t NumUdpSdSockets,
    uint8_t NumUdpRpcSockets,
    class TcpServerSocketType,
    uint8_t NumTcpServerSockets,
    class TcpSocketType,
    uint8_t NumTcpRpcSockets,
    size_t BufferSize,
    uint8_t NumTpStreams,
    size_t InternalTcpReassembleBufferSize = BufferSize>
class NetworkResources : public NetworkConfig
{
protected:
    NetworkResources(
        ::ip::IPAddress const& multicastIp, ::ip::IPAddress const& localIp, uint8_t const subnetId)
    : NetworkConfig(
        multicastIp, localIp, subnetId, _udpConfigInstance, _tcpConfigInstance, _tpConfigInstance)
    , _inputBuffer()
    , _outputBuffer()
    , _udpConfigInstance(_inputBuffer, _outputBuffer)
    , _tcpConfigInstance(_inputBuffer, _outputBuffer)
    , _tpConfigInstance()
    {}

    ~NetworkResources() = default;

private:
    ::etl::array<uint8_t, BufferSize> _inputBuffer;
    ::etl::array<uint8_t, BufferSize> _outputBuffer;

    internal::UdpResources<UdpSocketType, NumUdpSdSockets, NumUdpRpcSockets> _udpConfigInstance;

    internal::TcpResources<
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpSocketType,
        NumTcpRpcSockets,
        InternalTcpReassembleBufferSize>
        _tcpConfigInstance;

    internal::TpResources<NumTpStreams, BufferSize> _tpConfigInstance;
};

} // namespace internal

namespace declare
{
/**
 * Declares a network configuration with SD.
 */
template<
    class UdpSocketType,
    uint8_t NumUdpRpcSockets,
    class TcpServerSocketType,
    uint8_t NumTcpServerSockets,
    class TcpSocketType,
    uint8_t NumTcpRpcSockets,
    size_t BufferSize,
    uint8_t NumTpStreams                   = 0U,
    bool SdEndpointOption                  = false,
    size_t InternalTcpReassembleBufferSize = BufferSize>
class SdNetworkConfig
: public internal::NetworkResources<
      UdpSocketType,
      (SdEndpointOption ? 2 : 1),
      NumUdpRpcSockets,
      TcpServerSocketType,
      NumTcpServerSockets,
      TcpSocketType,
      NumTcpRpcSockets,
      BufferSize,
      NumTpStreams,
      InternalTcpReassembleBufferSize>
{
public:
    constexpr SdNetworkConfig(
        ::ip::IPAddress const& multicastIp, ::ip::IPAddress const& localIp, uint8_t const subnetId)
    : internal::NetworkResources<
        UdpSocketType,
        (SdEndpointOption ? 2 : 1),
        NumUdpRpcSockets,
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpSocketType,
        NumTcpRpcSockets,
        BufferSize,
        NumTpStreams,
        InternalTcpReassembleBufferSize>(multicastIp, localIp, subnetId)
    {}
};

/**
 * Declares a network configuration with RPC only.
 */
template<
    class UdpSocketType,
    uint8_t NumUdpRpcSockets,
    class TcpServerSocketType,
    uint8_t NumTcpServerSockets,
    class TcpSocketType,
    uint8_t NumTcpRpcSockets,
    size_t BufferSize,
    uint8_t NumTpStreams                   = 0U,
    size_t InternalTcpReassembleBufferSize = BufferSize>
class RpcNetworkConfig
: public internal::NetworkResources<
      UdpSocketType,
      0,
      NumUdpRpcSockets,
      TcpServerSocketType,
      NumTcpServerSockets,
      TcpSocketType,
      NumTcpRpcSockets,
      BufferSize,
      NumTpStreams,
      InternalTcpReassembleBufferSize>
{
public:
    RpcNetworkConfig(::ip::IPAddress const& localIp, uint8_t const subnetId)
    : internal::NetworkResources<
        UdpSocketType,
        0,
        NumUdpRpcSockets,
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpSocketType,
        NumTcpRpcSockets,
        BufferSize,
        NumTpStreams,
        InternalTcpReassembleBufferSize>(NetworkResource::INVALID_IP, localIp, subnetId)
    {}
};

} // namespace declare
} // namespace someip
