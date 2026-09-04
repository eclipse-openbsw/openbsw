/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/Network.h"

#include "someip/INetworkListener.h"
#include "someip/NetworkConfig.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <etl/error_handler.h>
#include <ip/to_str.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

Network::Network(NetworkConfig const& config) : _config(config), _started(false) {}

void Network::setSdListener(INetworkListener& listener)
{
    if (!isStarted())
    {
        _config._udpConfig._sdProxies.setListener(listener);
    }
}

void Network::setRpcListener(INetworkListener& listener)
{
    if (!isStarted())
    {
        _config._udpConfig._rpcProxies.setListener(listener);
        _config._tcpConfig._tcpProxies.setListener(listener);
    }
}

bool Network::initSdPort(uint16_t const port)
{
    if (!isStarted())
    {
        return _config._udpConfig._sdProxies.initPort(port);
    }

    return false;
}

bool Network::initUdpPort(uint16_t const port)
{
    if (!isStarted())
    {
        return _config._udpConfig._rpcProxies.initPort(port);
    }

    if (nullptr != _config._udpConfig._rpcProxies.getOpenProxy(port))
    {
        return true; // already open
    }

    INFO_LOG(SOMEIP, "Network: open udp-proxy on port: %d", port);

    return openUdp(port);
}

void Network::shutdownUdpPort(uint16_t const port)
{
    if (isStarted())
    {
        return;
    }

    _config._udpConfig._rpcProxies.shutdownPort(port);
}

bool Network::initTcpPort(uint16_t const port)
{
    if (!isStarted())
    {
        return _config._tcpConfig._tcpServers.initPort(port);
    }

    if (nullptr != _config._tcpConfig._tcpServers.getOpenServer(port))
    {
        return true; // already open
    }

    INFO_LOG(SOMEIP, "Network: open tcp-server on port: %d", port);
    TcpServer* const server = getIdleServer();
    if (server != nullptr)
    {
        return openTcp(port, *server);
    }

    ERROR_LOG(SOMEIP, "Network: No idle server available!");
    return false;
}

bool Network::initTcpPortWithExternalBuffers(
    uint16_t port, ::etl::ivector<::etl::span<uint8_t>>& buffers)
{
    ETL_ASSERT(
        _config._tcpConfig._tcpProxies.getBufferType() == TcpProxyConfig::BufferType::External,
        ETL_ERROR_GENERIC("tcp proxies must use external buffer type"));

    if (!_config._tcpConfig._tcpServers.setBufferPool(port, buffers))
    {
        ERROR_LOG(SOMEIP, "Network: init tcp server, fail to set buffer for port: %d", port);
        return false;
    }
    return initTcpPort(port);
}

void Network::shutdownTcpPort(uint16_t const port)
{
    if (isStarted())
    {
        return;
    }

    TcpServer* const server = _config._tcpConfig._tcpServers.getOpenServer(port);

    if (nullptr != server)
    {
        server->close();
    }
}

bool Network::isStarted() const { return _started; }

bool Network::start()
{
    if (isStarted())
    {
        return true;
    }

    INFO_LOG(SOMEIP, "Network: start");

    if (startSd())
    {
        if (startUdp())
        {
            if (startTcp())
            {
                _started = true;
            }
        }
    }

    return _started;
}

void Network::stop()
{
    if (!isStarted())
    {
        return;
    }

    INFO_LOG(SOMEIP, "Network: stop");

    stopSd();
    stopUdp();
    stopTcp();

    _started = false;
}

::ip::IPAddress const& Network::getMulticastIp() const { return _config._multicastIp; }

::ip::IPAddress const& Network::getLocalIp() const { return _config._localIp; }

uint8_t Network::getSubnetId() const { return _config._subnetId; }

::etl::expected<port::type, PortError> Network::getSdPort(bool const multicast) const
{
    size_t const idx = (multicast ? static_cast<size_t>(0U) : static_cast<size_t>(1U));

    if (_config._udpConfig._sdProxies.getSize() > idx)
    {
        return _config._udpConfig._sdProxies.getPort(idx);
    }

    return ::etl::unexpected<PortError>(PortError::NOT_AVAILABLE);
}

::etl::optional<NetworkChannel>
Network::getSdChannel(uint16_t const localPort, ::ip::IPEndpoint const& remoteEndpoint) const
{
    UdpProxy* const proxy = _config._udpConfig._sdProxies.getOpenProxy(localPort);
    if (proxy != nullptr)
    {
        return NetworkChannel(*proxy, remoteEndpoint);
    }

    char remoteEndpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
    WARN_LOG(
        SOMEIP,
        "Network: no sd-channel at port %d to %s",
        localPort,
        ::ip::to_str(remoteEndpoint, remoteEndpointStr).data());

    return {};
}

::etl::optional<NetworkChannel> Network::getRpcChannel(
    uint16_t const localPort, ::ip::IPEndpoint const& remoteEndpoint, uint8_t proto) const
{
    if (proto == proto::SD_L4_PROTO_UDP)
    {
        UdpProxy* const udpProxy = _config._udpConfig._rpcProxies.getOpenProxy(localPort);
        if (udpProxy != nullptr)
        {
            return NetworkChannel(*udpProxy, remoteEndpoint);
        }
    }
    else if (proto == proto::SD_L4_PROTO_TCP)
    {
        TcpProxy* const tcpProxy
            = _config._tcpConfig._tcpProxies.getOpenProxy(localPort, remoteEndpoint);
        if (tcpProxy != nullptr)
        {
            return NetworkChannel(*tcpProxy, remoteEndpoint, false, _config._useMagicCookie);
        }
    }
    else
    {
        ERROR_LOG(SOMEIP, "Network:getRpcChannel invalid proto parameter");
        return ::etl::optional<NetworkChannel>();
    }

    char remoteEndpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
    WARN_LOG(
        SOMEIP,
        "Network: no %s rpc-channel at port %d to %s",
        proto == proto::SD_L4_PROTO_UDP ? "UDP" : "TCP",
        localPort,
        ::ip::to_str(remoteEndpoint, remoteEndpointStr).data());

    return ::etl::optional<NetworkChannel>();
}

::etl::optional<NetworkChannel>
Network::openUdpChannel(port::type const localPort, ::ip::IPEndpoint const& remoteEndpoint)
{
    ::ip::IPAddress const remoteIp = remoteEndpoint.getAddress();
    bool const isMulticast         = ::ip::isMulticastAddress(remoteIp);

    // find a proxy that is already open on the requested port
    UdpProxy* proxy = _config._udpConfig._rpcProxies.getOpenProxy(localPort);
    if (proxy != nullptr)
    {
        if (isMulticast)
        {
            // The remoteIp is a multicast address in this context.
            if (!proxy->join(remoteIp))
            {
                // Since the sockets of all proxies are bound to the interface of the local ip, we
                // can add a multicast group to any open proxy with matching port.
                ERROR_LOG(SOMEIP, "Network: failed to join existing proxy to multicast group");
                return {};
            }
        }
        return NetworkChannel(*proxy, remoteEndpoint);
    }

    proxy = _config._udpConfig._rpcProxies.nextProxy();

    if (proxy == nullptr)
    {
        ERROR_LOG(SOMEIP, "Network: no free udp-proxy");
        return {};
    }

    if (!openLocalUdpProxy(*proxy, localPort))
    {
        ERROR_LOG(SOMEIP, "Network: failed to open udp-proxy");
        return {};
    }

    if (isMulticast)
    {
        // the remoteIp is a multicast address in this context
        if (!proxy->join(remoteIp))
        {
            ERROR_LOG(SOMEIP, "Network: failed to join multicast group");
            return {};
        }
    }

    return NetworkChannel(*proxy, remoteEndpoint);
}

TcpProxy*
Network::getTcpProxy(port::type const localPort, ::ip::IPEndpoint const& remoteEndpoint) const
{
    TcpProxy* proxy = _config._tcpConfig._tcpProxies.getOpenProxy(localPort, remoteEndpoint);
    if (proxy == nullptr)
    {
        proxy = _config._tcpConfig._tcpProxies.nextProxy();
        if (proxy == nullptr)
        {
            ERROR_LOG(SOMEIP, "Network: no free tcp-proxy");
        }
    }

    return proxy;
}

::etl::optional<NetworkChannel>
Network::openTcpChannel(port::type const localPort, ::ip::IPEndpoint const& remoteEndpoint)
{
    ETL_ASSERT(
        _config._tcpConfig._tcpProxies.getBufferType() == TcpProxyConfig::BufferType::Internal,
        ETL_ERROR_GENERIC("tcp proxies must use internal buffer type"));

    return openTcpChannelWithExternalReassembleBuffer(
        localPort, remoteEndpoint, ::etl::span<uint8_t>());
}

::etl::optional<NetworkChannel> Network::openTcpChannelWithExternalReassembleBuffer(
    uint16_t const localPort, ::ip::IPEndpoint const& remoteEndpoint, ::etl::span<uint8_t> buffer)
{
    TcpProxy* proxy = getTcpProxy(localPort, remoteEndpoint);

    if (proxy != nullptr)
    {
        if (!proxy->isOpen())
        {
            if (_config._tcpConfig._tcpProxies.getBufferType()
                == TcpProxyConfig::BufferType::External)
            {
                ETL_ASSERT(
                    buffer.size() > 0U,
                    ETL_ERROR_GENERIC("external buffer size must be greater than 0"));
                proxy->setInternalBuffer(buffer);
            }

            ::ip::IPEndpoint const localEndpoint(_config._localIp, localPort);
            if (!proxy->open(localEndpoint, remoteEndpoint))
            {
                proxy = nullptr;
            }
        }
    }

    if (proxy == nullptr)
    {
        return {};
    }
    return NetworkChannel(*proxy, remoteEndpoint, false, _config._useMagicCookie);
}

bool Network::startSd() const
{
    size_t const numProxies = _config._udpConfig._sdProxies.getSize();

    for (size_t i = 0U; i < numProxies; ++i)
    {
        auto const portResult = _config._udpConfig._sdProxies.getPort(i);
        if (!portResult.has_value())
        {
            continue;
        }

        uint16_t const port = portResult.value();

        INFO_LOG(SOMEIP, "Network: open sd-proxy[%d] on port: %d", i, port);

        UdpProxy* const proxy = _config._udpConfig._sdProxies.nextProxy();
        if (proxy == nullptr)
        {
            ERROR_LOG(SOMEIP, "Network: no free sd-proxy");
            return false;
        }

        bool const resultOpen = openLocalUdpProxy(*proxy, port);
        if (!resultOpen)
        {
            ERROR_LOG(SOMEIP, "Network: failed to open sd-proxy");
            return false;
        }

        if (i == 0U)
        {
            bool const resultJoin = proxy->join(_config._multicastIp);
            if (!resultJoin)
            {
                ERROR_LOG(SOMEIP, "Network: failed to join multicast group");
                return false;
            }
        }

        _config._udpConfig._sdProxies.addToServers(*proxy);
    }

    return true;
}

void Network::stopSd() const { _config._udpConfig._sdProxies.close(); }

bool Network::startUdp() const
{
    size_t const numProxies = _config._udpConfig._rpcProxies.getSize();

    for (size_t i = 0U; i < numProxies; ++i)
    {
        auto const portResult = _config._udpConfig._rpcProxies.getPort(i);
        if (!portResult.has_value())
        {
            continue;
        }

        uint16_t const port = portResult.value();

        INFO_LOG(SOMEIP, "Network: open udp-proxy[%d] on port: %d", i, port);
        if (!openUdp(port))
        {
            return false;
        }
    }

    return true;
}

bool Network::openUdp(port::type const port) const
{
    UdpProxy* const proxy = _config._udpConfig._rpcProxies.nextProxy();
    if (proxy == nullptr)
    {
        ERROR_LOG(SOMEIP, "Network: no free udp-proxy");
        return false;
    }

    if (!openLocalUdpProxy(*proxy, port))
    {
        ERROR_LOG(SOMEIP, "Network: failed to open udp-proxy");
        return false;
    }
    _config._udpConfig._rpcProxies.addToServers(*proxy);
    return true;
}

void Network::stopUdp() const { _config._udpConfig._rpcProxies.close(); }

bool Network::openLocalUdpProxy(UdpProxy& proxy, port::type const port) const
{
    ::ip::IPEndpoint const localEndpoint(_config._localIp, port);
    return proxy.open(localEndpoint);
}

bool Network::startTcp() const
{
    size_t const numServers = _config._tcpConfig._tcpServers.getSize();

    for (size_t i = 0U; i < numServers; ++i)
    {
        auto const portResult = _config._tcpConfig._tcpServers.getPort(i);
        if (!portResult.has_value())
        {
            continue;
        }

        uint16_t const port = portResult.value();

        INFO_LOG(SOMEIP, "Network: open tcp-server[%d] on port: %d", i, port);
        TcpServer* const server = getIdleServer();

        if (server == nullptr)
        {
            return false;
        }
        if (!openTcp(port, *server))
        {
            return false;
        }
    }

    return true;
}

TcpServer* Network::getIdleServer() const
{
    TcpServer* const server = _config._tcpConfig._tcpServers.nextServer();
    if (server == nullptr)
    {
        ERROR_LOG(SOMEIP, "Network: no free tcp-server");
    }
    return server;
}

bool Network::openTcp(port::type const port, TcpServer& server) const
{
    ::ip::IPEndpoint const localEndpoint(_config._localIp, port);

    if (_config._tcpConfig._tcpProxies.getBufferType() == TcpProxyConfig::BufferType::External)
    {
        TcpServer::ClientBufferPool* const pool
            = _config._tcpConfig._tcpServers.getBufferPool(port);
        if (pool == nullptr)
        {
            ERROR_LOG(SOMEIP, "Network: failed to open tcp-server, no reassembly buffer");
            return false;
        }
        server.setClientBufferPool(*pool);
    }
    if (!server.open(localEndpoint))
    {
        ERROR_LOG(SOMEIP, "Network: failed to open tcp-server");
        return false;
    }

    return true;
}

void Network::stopTcp() const
{
    _config._tcpConfig._tcpProxies.close();
    _config._tcpConfig._tcpServers.close();
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
