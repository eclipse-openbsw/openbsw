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

#include "someip/SomeIpConstants.h"
#include "someip/UdpProxy.h"

#include <etl/expected.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <cstdint>

namespace someip
{
/**
 * The UDP proxy configuration.
 */
class UdpProxyConfig
{
public:
    UdpProxyConfig(
        ::etl::span<UdpProxy> proxies,
        ::etl::ivector<UdpProxy*>& serverProxies,
        ::etl::ivector<uint16_t>& ports);

    void setListener(INetworkListener& listener) const;

    size_t getSize() const;

    UdpProxy* getProxy(size_t pos) const;
    UdpProxy* nextProxy() const;
    UdpProxy* getOpenProxy(uint16_t localPort) const;

    bool initPort(uint16_t port) const;
    ::etl::expected<uint16_t, PortError> getPort(size_t pos) const;

    void close();

    void shutdownPort(uint16_t port);

    void addToServers(UdpProxy& proxy);

private:
    void releaseServer(UdpProxy& proxy);

    ::etl::span<UdpProxy> _proxies;
    ::etl::ivector<UdpProxy*>& _serverProxies;
    ::etl::ivector<uint16_t>& _ports;
};

/**
 * The UDP configuration.
 */
class UdpConfig
{
public:
    UdpConfig(UdpConfig const&)            = delete;
    UdpConfig& operator=(UdpConfig const&) = delete;

    UdpProxyConfig& _sdProxies;
    UdpProxyConfig& _rpcProxies;

protected:
    UdpConfig(UdpProxyConfig& sdProxies, UdpProxyConfig& rpcProxies)
    : _sdProxies(sdProxies), _rpcProxies(rpcProxies)
    {}
};

namespace internal
{
/**
 * Internal UDP proxy resources.
 */
template<class SocketType, uint8_t NumSockets>
class UdpProxyResources : public UdpProxyConfig
{
public:
    UdpProxyResources(
        ::etl::span<uint8_t> const& inputBuffer, ::etl::span<uint8_t> const& outputBuffer)
    : UdpProxyConfig(
        ::etl::span<UdpProxy>(&_proxyArray[0], NumSockets), _serverProxyList, _portList)
    {
        for (uint8_t i = 0U; i < NumSockets; ++i)
        {
            UdpProxy& proxy = _proxyArray[i];
            proxy.setSocket(_socketArray[i]);
            proxy.setInputBuffer(inputBuffer);
            proxy.setOutputBuffer(outputBuffer);
        }
    }

private:
    SocketType _socketArray[NumSockets];
    UdpProxy _proxyArray[NumSockets];

    ::etl::vector<UdpProxy*, NumSockets> _serverProxyList;
    ::etl::vector<uint16_t, NumSockets> _portList;
};

// Specialization for zero proxies
template<class SocketType>
class UdpProxyResources<SocketType, 0> : public UdpProxyConfig
{
public:
    UdpProxyResources(::etl::span<uint8_t> const&, ::etl::span<uint8_t> const&)
    : UdpProxyConfig(::etl::span<UdpProxy>(), _emptyServerProxies, _emptyPorts)
    {}

    // Override methods that would modify the static vectors to be no-ops
    bool initPort(uint16_t) const { return false; }

    void shutdownPort(uint16_t) {}

    void addToServers(UdpProxy&) {}

private:
    static ::etl::vector<UdpProxy*, 1> _emptyServerProxies;
    static ::etl::vector<uint16_t, 1> _emptyPorts;
};

// Define static members for the template specialization
template<class SocketType>
::etl::vector<UdpProxy*, 1> UdpProxyResources<SocketType, 0>::_emptyServerProxies;

template<class SocketType>
::etl::vector<uint16_t, 1> UdpProxyResources<SocketType, 0>::_emptyPorts;

/**
 * Internal UDP resources.
 */
template<class UdpSocketType, uint8_t NumUdpSdSockets, uint8_t NumUdpRpcSockets>
class UdpResources : public UdpConfig
{
public:
    UdpResources(::etl::span<uint8_t> const& inputBuffer, ::etl::span<uint8_t> const& outputBuffer)
    : UdpConfig(_sdResources, _rpcResources)
    , _sdResources(inputBuffer, outputBuffer)
    , _rpcResources(inputBuffer, outputBuffer)
    {}

private:
    UdpProxyResources<UdpSocketType, NumUdpSdSockets> _sdResources;
    UdpProxyResources<UdpSocketType, NumUdpRpcSockets> _rpcResources;
};

} // namespace internal
} // namespace someip
