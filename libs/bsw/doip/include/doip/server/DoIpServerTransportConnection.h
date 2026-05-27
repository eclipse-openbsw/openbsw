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

#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpTcpConnection.h"
#include "doip/server/DoIpServerConnectionHandler.h"
#include "doip/server/DoIpServerTransportMessageHandler.h"

#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>

namespace doip
{
class DoIpServerTransportConnectionConfig;
class IDoIpServerConnectionHandlerCallback;

/**
 * Class that represents a server transport connection.
 * It is responsible for base connection handling with routing activation and .
 */
// multiple inheritance of interfaces is OK
class DoIpServerTransportConnection
: public DoIpServerConnectionHandler
, public ::etl::forward_link<0>
{
public:
    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param socketGroupId identifier of socket group the connection belongs to
     * \param socket reference to socket
     * \param config reference to config
     * \param diagnosticSendJobBlockPool reference to block pool for transport message send jobs
     * \param protocolSendJobBlockPool reference to block pool for protocol send jobs
     * \param type connection type, e. g. PLAIN or TLS
     */
    DoIpServerTransportConnection(
        DoIpConstants::ProtocolVersion protocolVersion,
        uint8_t socketGroupId,
        ::tcp::AbstractSocket& socket,
        DoIpServerTransportConnectionConfig const& config,
        ::etl::ipool& diagnosticSendJobBlockPool,
        ::etl::ipool& protocolSendJobBlockPool,
        DoIpTcpConnection::ConnectionType type);

    /**
     * Get the contained connection.
     * \return reference to connection
     */
    DoIpTcpConnection& getConnection();

    /**
     * Send a transport message to the tester.
     * \param transportMessage transport message to send
     * \param pNotificationListener listener interface
     * \return true if message has been sent successfully, false otherwise
     */
    bool send(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener);

    /**
     * mark the connection for closing.
     * \param closeMode close mode for the TCP connection
     */
    void markForClose(IDoIpTcpConnection::CloseMode closeMode);

    /**
     * Check whether the connection has been marked for closing.
     * \return true if marked for closing
     */
    bool isMarkedForClose() const;

    DoIpTcpConnection::ConnectionType type() const;

private:
    DoIpTcpConnection _connection;
    DoIpServerTransportMessageHandler _transportMessageHandler;
    uint8_t _writeBuffer[DoIpConstants::DOIP_HEADER_LENGTH];
    bool _isMarkedForClose;
    DoIpTcpConnection::ConnectionType _type;
};

/**
 * Inline implementations.
 */
// connections are managed from the outside
inline DoIpTcpConnection& DoIpServerTransportConnection::getConnection() { return _connection; }

inline void
DoIpServerTransportConnection::markForClose(IDoIpTcpConnection::CloseMode const closeMode)
{
    _connection.setCloseMode(closeMode);
    _isMarkedForClose = true;
}

inline bool DoIpServerTransportConnection::isMarkedForClose() const { return _isMarkedForClose; }

inline DoIpTcpConnection::ConnectionType DoIpServerTransportConnection::type() const
{
    return _type;
}

} // namespace doip
