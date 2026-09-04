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

#include "someip/TcpProxy.h"

#include <ip/IPEndpoint.h>
#include <tcp/socket/AbstractServerSocket.h>
#include <tcp/socket/AbstractSocket.h>
#include <tcp/socket/ISocketProvidingConnectionListener.h>

#include <etl/vector.h>
#include <cstdint>

namespace someip
{
class TcpProxyConfig;

/**
 * A TCP server for accepting incoming connections.
 */
class TcpServer
: public ::tcp::ISocketProvidingConnectionListener
, public TcpProxy::IConnectionListener
{
public:
    using ClientBufferPool = ::etl::ivector<::etl::span<uint8_t>>;

    TcpServer(::tcp::AbstractServerSocket& socket, TcpProxyConfig& tcpClientPool);

    bool isInitialized() const;

    ::tcp::AbstractServerSocket& getServerSocket() const;

    size_t getNumProxies() const;
    TcpProxy* getProxy(size_t pos) const;

    bool isOpen() const;
    bool open(::ip::IPEndpoint const& localEndpoint);
    void close();

    ::etl::expected<uint16_t, PortError> getLocalPort() const;

    void setClientBufferPool(ClientBufferPool& buffers);

    /** \see ISocketProvidingConnectionListener */
    ::tcp::AbstractSocket* getSocket(::ip::IPAddress const& ipAddr, uint16_t port) override;

    /** \see ISocketProvidingConnectionListener */
    void connectionAccepted(::tcp::AbstractSocket& socket) override;

    void connectionChanged(TcpProxy& proxy) override;

private:
    ::tcp::AbstractServerSocket& _socket;
    TcpProxyConfig& _tcpClientPool;
    ClientBufferPool* _pClientBufferPool;
};

} // namespace someip
