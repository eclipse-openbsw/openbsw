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

#include "doip/common/DoIpTcpConnection.h"
#include "doip/server/IDoIpServerConnectionFilter.h"

namespace doip
{
/**
 * Callback interface for DoIP transport layer.
 */
class IDoIpServerTransportLayerCallback : public IDoIpServerConnectionFilter
{
public:
    /**
     * Get unique identifier of a socket group information for an incoming connection.
     * \param serverSocketId identifier of server socket
     * \param remoteEndpoint reference to connecting remote endpoint
     * \return unique identifier of the socket group
     */
    virtual uint8_t
    getSocketGroupId(uint8_t serverSocketId, ::ip::IPEndpoint const& remoteEndpoint) const
        = 0;

    /**
     * Get maximum number of connections allowed for a socket group.
     * \param socketGroupId identifier of the socket group to get the maximum number of
     *        connections for
     * \return maximum number of connections allowed for this group
     */
    virtual uint8_t getMaxConnectionCount(uint8_t socketGroupId) const = 0;

    /**
     * Filter a connection before it is accepted. This is the chance to
     * prepare the system to receive a connection.
     * \param socketGroupId socket group identifier
     * \param remoteEndpoint reference to connecting remote endpoint
     * \return true if the system has been successfully prepared for accepting
     *         the connection
     */
    virtual bool filterConnection(uint8_t socketGroupId, ::ip::IPEndpoint const& remoteEndpoint)
        = 0;

    /**
     * Called if a routing activation was successful.
     * \param sourceAddress source address of the external tester
     * \param internalSourceAddress internal source address for routing
     * \param localEndpoint reference to local endpoint of the connection
     * \param remoteEndpoint reference to remote endpoint of the connection
     * \param type connection type
     */
    virtual void routingActive(
        uint16_t sourceAddress,
        uint16_t internalSourceAddress,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint,
        DoIpTcpConnection::ConnectionType type)
        = 0;
    /**
     * Called if the last active connection for an internal source address is closed.
     * \param internalSourceAddress internal source address for routing
     */
    virtual void routingInactive(uint16_t internalSourceAddress) = 0;

    /**
     * Called if a DoIP connection is closed.
     * \param sourceAddress source address of the external tester
     */
    virtual void connectionClosed(uint16_t sourceAddress) = 0;

protected:
    ~IDoIpServerTransportLayerCallback() = default;
};

} // namespace doip
