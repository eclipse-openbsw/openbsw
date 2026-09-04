/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpConfig.h"

#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPEndpoint;
using ::util::logger::SOMEIP;

TcpServerConfig::TcpServerConfig(
    ::etl::span<TcpServer> servers,
    ::etl::span<uint16_t> const ports,
    ::etl::span<ClientBuffers> const clientBuffers)
: _servers(servers), _ports(ports), _clientBuffers(clientBuffers)
{}

size_t TcpServerConfig::getSize() const { return _servers.size(); }

TcpServer* TcpServerConfig::getServer(size_t const pos) const { return &_servers[pos]; }

TcpServer* TcpServerConfig::nextServer() const
{
    for (auto& server : _servers)
    {
        if (!server.isOpen())
        {
            return &server;
        }
    }

    return nullptr;
}

TcpServer* TcpServerConfig::getOpenServer(uint16_t const localPort) const
{
    for (auto& server : _servers)
    {
        auto const portResult = server.getLocalPort();
        if (server.isOpen() && portResult.has_value() && (portResult.value() == localPort))
        {
            return &server;
        }
    }

    return nullptr;
}

bool TcpServerConfig::initPort(uint16_t const port) const
{
    for (uint16_t& _port : _ports)
    {
        if ((_port == port) || (_port == port::INVALID))
        {
            _port = port;
            return true;
        }
    }
    return false;
}

bool TcpServerConfig::setBufferPool(uint16_t const port, ::etl::ivector<::etl::span<uint8_t>>& pool)
{
    for (ClientBuffers& clientBuffers : _clientBuffers)
    {
        if ((clientBuffers.serverPort == port) || (clientBuffers.serverPort == port::INVALID))
        {
            clientBuffers.serverPort       = port;
            clientBuffers.clientBufferPool = &pool;
            return true;
        }
    }

    return false;
}

TcpServer::ClientBufferPool* TcpServerConfig::getBufferPool(uint16_t const port) const
{
    for (ClientBuffers& clientBuffers : _clientBuffers)
    {
        if (clientBuffers.serverPort == port)
        {
            return clientBuffers.clientBufferPool;
        }
    }

    return nullptr;
}

::etl::expected<uint16_t, PortError> TcpServerConfig::getPort(size_t const pos) const
{
    if (pos < _ports.size())
    {
        uint16_t const port = _ports.at(pos);
        if (port == port::INVALID)
        {
            return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
        }
        return port;
    }
    return ::etl::unexpected<PortError>(PortError::OUT_OF_RANGE);
}

void TcpServerConfig::close()
{
    for (size_t i = 0U; i < _servers.size(); ++i)
    {
        TcpServer& server = _servers.at(i);
        if (server.isOpen())
        {
            INFO_LOG(SOMEIP, "Network: close tcp-server[%d]", i);
            server.close();
        }
    }
}

TcpProxyConfig::TcpProxyConfig(::etl::span<TcpProxy> proxies, BufferType const bufferType)
: _proxies(proxies), _bufferType(bufferType)
{}

void TcpProxyConfig::setListener(INetworkListener& listener) const
{
    for (auto& proxy : _proxies)
    {
        proxy.setListener(listener);
    }
}

size_t TcpProxyConfig::getSize() const { return _proxies.size(); }

TcpProxy* TcpProxyConfig::getProxy(size_t const pos) const { return &_proxies[pos]; }

TcpProxy* TcpProxyConfig::getProxy(::tcp::AbstractSocket const& socket) const
{
    for (TcpProxy& tcpProxy : _proxies)
    {
        if (&(tcpProxy.getSocket()) == (&socket))
        {
            return &tcpProxy;
        }
    }

    return nullptr;
}

TcpProxy* TcpProxyConfig::nextProxy() const
{
    for (auto& proxy : _proxies)
    {
        if (proxy.isIdle())
        {
            return &proxy;
        }
    }

    return nullptr;
}

TcpProxy*
TcpProxyConfig::getOpenProxy(::ip::IPAddress const& remoteIp, uint16_t const localPort) const
{
    for (auto& proxy : _proxies)
    {
        auto const portResult = proxy.getLocalPort();
        if (proxy.isInitialized() && (proxy.isOpen()) && portResult.has_value()
            && (portResult.value() == localPort)
            && (proxy.getRemoteEndpoint().getAddress() == remoteIp))
        {
            return &proxy;
        }
    }

    return nullptr;
}

TcpProxy*
TcpProxyConfig::getOpenProxy(uint16_t const localPort, IPEndpoint const& remoteEndpoint) const
{
    for (auto& proxy : _proxies)
    {
        auto const portResult = proxy.getLocalPort();
        if (proxy.isOpen() && portResult.has_value() && (portResult.value() == localPort)
            && (proxy.getRemoteEndpoint() == remoteEndpoint))
        {
            return &proxy;
        }
    }

    return nullptr;
}

void TcpProxyConfig::close()
{
    for (size_t i = 0U; i < _proxies.size(); ++i)
    {
        TcpProxy& proxy = _proxies[i];
        if (proxy.isOpen())
        {
            INFO_LOG(SOMEIP, "Network: close tcp-proxy[%d]", i);
            proxy.close();
        }
    }
}

TcpConfig::TcpConfig(TcpServerConfig& tcpServers, TcpProxyConfig& tcpProxies)
: _tcpServers(tcpServers), _tcpProxies(tcpProxies)
{}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
