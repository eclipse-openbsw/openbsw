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

#include <tcp/socket/AbstractSocket.h>

namespace doip
{
class DoIpServerTransportConnection;
class DoIpServerTransportConnectionConfig;

/**
 * Interface for a DoIP transport connection pool.
 */
class IDoIpServerTransportConnectionPool
{
public:
    /**
     * Create a transport connection.
     * \param socketGroupId identifier of socket group the connection belongs to
     * \param socket socket to use
     * \param config common configuration for transport connections
     * \return pointer to connection if memory is available, 0 otherwise
     */
    virtual DoIpServerTransportConnection* createConnection(
        uint8_t socketGroupId,
        ::tcp::AbstractSocket& socket,
        DoIpServerTransportConnectionConfig const& config,
        DoIpTcpConnection::ConnectionType type)
        = 0;

    /**
     * Release a transport connection.
     * \param connection connection to release
     */
    virtual void releaseConnection(DoIpServerTransportConnection& connection) = 0;

protected:
    ~IDoIpServerTransportConnectionPool() = default;
    IDoIpServerTransportConnectionPool& operator=(IDoIpServerTransportConnectionPool const&)
        = default;
};

} // namespace doip
