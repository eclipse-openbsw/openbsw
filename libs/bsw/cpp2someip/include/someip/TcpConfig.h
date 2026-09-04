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
#include "someip/TcpProxy.h"
#include "someip/TcpServer.h"

#include <ip/IPEndpoint.h>

#include <etl/array.h>
#include <etl/expected.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <cstdint>

namespace someip
{
/**
 * The TCP server configuration.
 */
class TcpServerConfig
{
public:
    struct ClientBuffers
    {
        TcpServer::ClientBufferPool* clientBufferPool;
        uint16_t serverPort;
    };

    TcpServerConfig(
        ::etl::span<TcpServer> servers,
        ::etl::span<uint16_t> ports,
        ::etl::span<ClientBuffers> clientBuffers);

    size_t getSize() const;

    TcpServer* getServer(size_t pos) const;
    TcpServer* nextServer() const;
    TcpServer* getOpenServer(uint16_t localPort) const;

    bool setBufferPool(uint16_t port, ::etl::ivector<::etl::span<uint8_t>>& pool);
    TcpServer::ClientBufferPool* getBufferPool(uint16_t port) const;

    bool initPort(uint16_t port) const;
    ::etl::expected<uint16_t, PortError> getPort(size_t pos) const;

    void close();

protected:
    void setServers(::etl::span<TcpServer> servers) { _servers = servers; }

private:
    ::etl::span<TcpServer> _servers;
    ::etl::span<uint16_t> _ports;
    ::etl::span<ClientBuffers> _clientBuffers;
};

/**
 * The TCP proxy configuration.
 */
class TcpProxyConfig
{
public:
    enum class BufferType : uint8_t
    {
        Internal,
        External
    };

    TcpProxyConfig(::etl::span<TcpProxy> proxies, BufferType bufferType);

    void setListener(INetworkListener& listener) const;

    size_t getSize() const;

    TcpProxy* getProxy(size_t pos) const;
    TcpProxy* getProxy(::tcp::AbstractSocket const& socket) const;
    TcpProxy* nextProxy() const;
    TcpProxy* getOpenProxy(::ip::IPAddress const& remoteIp, uint16_t localPort) const;
    TcpProxy* getOpenProxy(uint16_t localPort, ::ip::IPEndpoint const& remoteEndpoint) const;

    BufferType getBufferType() const { return _bufferType; }

    void setProxies(::etl::span<TcpProxy> proxies) { _proxies = proxies; }

    void close();

private:
    ::etl::span<TcpProxy> _proxies;
    BufferType _bufferType;
};

/**
 * The TCP configuration.
 */
class TcpConfig
{
public:
    TcpConfig(TcpConfig const&)            = delete;
    TcpConfig& operator=(TcpConfig const&) = delete;

    TcpServerConfig& _tcpServers;
    TcpProxyConfig& _tcpProxies;

protected:
    TcpConfig(TcpServerConfig& tcpServers, TcpProxyConfig& tcpProxies);
};

namespace internal
{
/**
 * Internal TCP server resources.
 */
template<class SocketType, uint8_t NumSockets, size_t NumClientBuffers = 0U>
class TcpServerResources : public TcpServerConfig
{
    static_assert((NumClientBuffers == 0U) || (NumClientBuffers == NumSockets), "");

public:
    explicit TcpServerResources(TcpProxyConfig& clientPool)
    : TcpServerConfig(
        ::etl::span<TcpServer>(),
        ::etl::span<uint16_t>(portArray),
        ::etl::span<ClientBuffers>(clientBuffersArray))
    , socketArray()
    , portArray()
    , servers()
    , clientBuffersArray()
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        for (size_t i = 0U; i < NumSockets; ++i)
#pragma GCC diagnostic pop
        {
            servers.emplace_back(socketArray[i], clientPool);
            portArray[i] = port::INVALID;
        }

        for (auto& clientBuffer : clientBuffersArray)
        {
            clientBuffer.serverPort = port::INVALID;
        }
        setServers(servers);
    }

    ::etl::array<SocketType, NumSockets> socketArray;
    ::etl::array<uint16_t, NumSockets> portArray;
    ::etl::vector<TcpServer, NumSockets> servers;
    ::etl::array<ClientBuffers, NumClientBuffers> clientBuffersArray;
};

// Specialization for zero TCP servers
template<class SocketType, size_t NumClientBuffers>
class TcpServerResources<SocketType, 0, NumClientBuffers> : public TcpServerConfig
{
public:
    explicit TcpServerResources(TcpProxyConfig& /* clientPool */)
    : TcpServerConfig(
        ::etl::span<TcpServer>(), ::etl::span<uint16_t>(), ::etl::span<ClientBuffers>())
    {}
};

/**
 * Internal TCP proxy resources.
 */
template<class SocketType, uint8_t NumSockets, size_t BufferSize>
class TcpProxyResources : public TcpProxyConfig
{
public:
    TcpProxyResources(
        ::etl::span<uint8_t> const& inputBuffer, ::etl::span<uint8_t> const& outputBuffer)
    : TcpProxyConfig(::etl::span<TcpProxy>(), BufferType::Internal)
    , socketArray()
    , bufferArray()
    , proxies()
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        for (uint8_t i = 0U; i < NumSockets; ++i)
        {
            proxies.emplace_back(socketArray[i]);
            proxies.back().setInputBuffer(inputBuffer);
            proxies.back().setOutputBuffer(outputBuffer);
            proxies.back().setInternalBuffer(bufferArray[i]);
        }
#pragma GCC diagnostic pop
        setProxies(proxies);
    }

    ::etl::array<SocketType, NumSockets> socketArray;
    ::etl::array<uint8_t, BufferSize> bufferArray[NumSockets];
    ::etl::vector<TcpProxy, NumSockets> proxies;
};

template<class SocketType, uint8_t NumSockets>
class TcpProxyResources<SocketType, NumSockets, 0U> : public TcpProxyConfig
{
public:
    TcpProxyResources(
        ::etl::span<uint8_t> const& inputBuffer, ::etl::span<uint8_t> const& outputBuffer)
    : TcpProxyConfig(::etl::span<TcpProxy>(), BufferType::External), socketArray(), proxies()
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        for (uint8_t i = 0U; i < NumSockets; ++i)
#pragma GCC diagnostic pop
        {
            proxies.emplace_back(socketArray[i]);
            proxies.back().setInputBuffer(inputBuffer);
            proxies.back().setOutputBuffer(outputBuffer);
        }
        setProxies(proxies);
    }

    ::etl::array<SocketType, NumSockets> socketArray;
    ::etl::vector<TcpProxy, NumSockets> proxies;
};

// Specialization for zero TCP proxies with zero buffer size
template<class SocketType>
class TcpProxyResources<SocketType, 0, 0U> : public TcpProxyConfig
{
public:
    TcpProxyResources(
        ::etl::span<uint8_t> const& /* inputBuffer */,
        ::etl::span<uint8_t> const& /* outputBuffer */)
    : TcpProxyConfig(::etl::span<TcpProxy>(), BufferType::External)
    {}
};

// Specialization for zero TCP proxies with internal buffer
template<class SocketType, size_t BufferSize>
class TcpProxyResources<SocketType, 0, BufferSize> : public TcpProxyConfig
{
public:
    TcpProxyResources(
        ::etl::span<uint8_t> const& /* inputBuffer */,
        ::etl::span<uint8_t> const& /* outputBuffer */)
    : TcpProxyConfig(::etl::span<TcpProxy>(), BufferType::Internal)
    {}
};

/**
 * Internal TCP resources.
 */
template<
    class TcpServerSocketType,
    uint8_t NumTcpServerSockets,
    class TcpSocketType,
    uint8_t NumTcpRpcSockets,
    size_t TcpBufferSize>
class TcpResources : public TcpConfig
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

    TcpResources(::etl::span<uint8_t> const& inputBuffer, ::etl::span<uint8_t> const& outputBuffer)
    : TcpConfig(_serverResources, _proxyResources)
    , _proxyResources(inputBuffer, outputBuffer)
    , _serverResources(_proxyResources)
    {}

#pragma GCC diagnostic pop

private:
    TcpProxyResources<TcpSocketType, NumTcpRpcSockets, TcpBufferSize> _proxyResources;
    TcpServerResources<
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpBufferSize == 0 ? NumTcpServerSockets : 0U>
        _serverResources;
};

} // namespace internal
} // namespace someip
