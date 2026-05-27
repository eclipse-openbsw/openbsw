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

namespace doip
{
class DoIpServerTransportConnection;

/**
 * Interface for creation of new transport connections.
 */
class IDoIpServerTransportConnectionProviderCallback
{
public:
    /**
     * Get socket group for an incoming connection on a server socket.
     * \param serverSocketId identifier of server socket
     * \param remoteEndpoint reference to connecting remote endpoint
     * \return identifier of the socket group this connection belongs to
     */
    virtual uint8_t
    getSocketGroupId(uint8_t serverSocketId, ::ip::IPEndpoint const& remoteEndpoint) const
        = 0;

    /**
     * Filter a connection before it is accepted. This is the chance to
     * prepare the system to receive a connection.
     * \param socketGroupId socket group identifier
     * \param remoteEndpoint reference to connecting remote endpoint
     * \return true if the connection is valid, otherwise it will be rejected
     */
    virtual bool filterConnection(uint8_t socketGroupId, ::ip::IPEndpoint const& remoteEndpoint)
        = 0;

    /**
     * Called if connection has been accepted.
     * \param connection connection to add
     */
    virtual void connectionAccepted(DoIpServerTransportConnection& connection) = 0;

protected:
    ~IDoIpServerTransportConnectionProviderCallback() = default;
    IDoIpServerTransportConnectionProviderCallback&
    operator=(IDoIpServerTransportConnectionProviderCallback const&)
        = default;
};

} // namespace doip
