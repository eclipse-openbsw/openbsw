/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/server/DoIpServerConnectionHandler.h"
#include "doip/server/DoIpServerTransportConnection.h"
#include "doip/server/DoIpServerTransportMessageHandler.h"
#include "doip/server/IDoIpServerSocketHandlerListener.h"
#include <etl/ipool.h>

namespace doip
{
class DoIpServerTransportConnection;
class DoIpServerTransportConnectionConfig;
class IDoIpServerSocketHandler;
class IDoIpServerTransportConnectionPool;
class IDoIpServerTransportConnectionProviderCallback;

/**
 * Base class for providing server transport connections.
 * It is responsible for base connection handling with routing activation and .
 */
class DoIpServerTransportConnectionProvider : private IDoIpServerSocketHandlerListener
{
public:
    /**
     * Type of connection pool.
     */
    using ConnectionPool = ::etl::ipool;
    /**
     * Constructor.
     * \param constructCallback function that creates a connection
     * \param callback callback for provider
     * \param socketHandler socket handler
     * \param connectionPool reference to connection pool
     * \param config reference to transport layer configuration
     */
    DoIpServerTransportConnectionProvider(
        IDoIpServerTransportConnectionProviderCallback& callback,
        IDoIpServerSocketHandler& socketHandler,
        IDoIpServerTransportConnectionPool& connectionPool,
        DoIpServerTransportConnectionConfig const& config);

    /**
     * Start the provider.
     */
    void start();

    /**
     * Stop the provider.
     */
    void stop();

    /**
     * Release a connection.
     * \param connection connection to release
     */
    void releaseConnection(DoIpServerTransportConnection& connection);

private:
    void serverSocketBound(
        uint8_t serverSocketId,
        ip::IPEndpoint const& localEndpoint,
        DoIpTcpConnection::ConnectionType type) override;

    bool filterConnection(uint8_t serverSocketId, ip::IPEndpoint const& remoteEndpoint) override;

    void connectionAccepted(
        uint8_t serverSocketId,
        ::tcp::AbstractSocket& socket,
        DoIpTcpConnection::ConnectionType type) override;

    void connectionDetached();

    uint8_t getSocketGroupId(uint8_t serverSocketId, ::tcp::AbstractSocket const& socket) const;

    IDoIpServerTransportConnectionProviderCallback& _callback;
    IDoIpServerSocketHandler& _socketHandler;
    IDoIpServerTransportConnectionPool& _connectionPool;
    DoIpServerTransportConnectionConfig const& _config;
};

} // namespace doip
