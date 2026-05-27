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

#include "DoIpServerTransportConnection.h"

#include <ip/IPEndpoint.h>
#include <tcp/socket/AbstractSocket.h>

namespace doip
{
/**
 * Listener interface for the DoIP socket handler.
 */
class IDoIpServerSocketHandlerListener
{
public:
    /**
     * Called whenever a server socket has been successfully bound to a new endpoint.
     * \param serverSocketId identifier of server socket
     * \param localEndpoint endpoint holding both address and port the socket has been bound to
     */
    virtual void serverSocketBound(
        uint8_t serverSocketId,
        ip::IPEndpoint const& localEndpoint,
        DoIpTcpConnection::ConnectionType type)
        = 0;

    /**
     * Called to allow filtering for connecting endpoints. Whenever a connection should
     * be accepted this function will be called before trying to acquire a socket for the
     * connection. \param serverSocketId identifier of server socket \param remoteEndpoint reference
     * to remote endpoint (i.e. the remote endpoint that tries to open the connection) \return
     *  - true if the source endpoint address valid and a socket should be acquired
     *  - false to reject the source endpoint
     */
    virtual bool filterConnection(uint8_t serverSocketId, ip::IPEndpoint const& remoteEndpoint) = 0;

    /**
     * Called whenever a socket has been acquired for a connecting endpoint which now should be
     * handled by the listening component. It is up to the listener to release the socket.
     * \param serverSocketId identifier of server socket
     * \param socket reference to the socket
     */
    virtual void connectionAccepted(
        uint8_t serverSocketId,
        ::tcp::AbstractSocket& socket,
        DoIpTcpConnection::ConnectionType type)
        = 0;

protected:
    ~IDoIpServerSocketHandlerListener() = default;

    IDoIpServerSocketHandlerListener& operator=(IDoIpServerSocketHandlerListener const&) = default;
};

} // namespace doip
