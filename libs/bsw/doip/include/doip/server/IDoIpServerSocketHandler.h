/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <doip/common/DoIpTcpConnection.h>
#include <tcp/socket/AbstractSocket.h>

namespace doip
{
class IDoIpServerSocketHandlerListener;

/**
 * Interface for the handler class providing TCP server and connection sockets.
 */
class IDoIpServerSocketHandler
{
public:
    /**
     * Start the socket handler. All server sockets will be prepared for binding.
     * \param listener reference to listener that receives callbacks
     */
    virtual void start(IDoIpServerSocketHandlerListener& listener) = 0;

    /**
     * Stop the socket handler. All server sockets are closed.
     */
    virtual void stop() = 0;

    /**
     * Allows direct acquiring of a TCP socket.
     * \return pointer to socket if available
     */
    virtual ::tcp::AbstractSocket* acquireSocket() = 0;

    /**
     * Releases a socket that was allocated or accepted from this handler.
     * \param socket reference to socket to release
     * \param type type of the socket (plain or TLS)
     */
    virtual void
    releaseSocket(::tcp::AbstractSocket& socket, DoIpTcpConnection::ConnectionType type)
        = 0;

protected:
    ~IDoIpServerSocketHandler() = default;

    IDoIpServerSocketHandler& operator=(IDoIpServerSocketHandler const&) = default;
};

} // namespace doip
