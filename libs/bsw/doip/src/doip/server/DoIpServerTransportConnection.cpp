/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportConnection.h"

#include "doip/server/DoIpServerTransportConnectionConfig.h"

namespace doip
{
DoIpServerTransportConnection::DoIpServerTransportConnection(
    DoIpConstants::ProtocolVersion const protocolVersion,
    uint8_t const socketGroupId,
    ::tcp::AbstractSocket& socket,
    DoIpServerTransportConnectionConfig const& config,
    ::etl::ipool& diagnosticSendJobBlockPool,
    ::etl::ipool& protocolSendJobBlockPool,
    DoIpTcpConnection::ConnectionType const type)
: DoIpServerConnectionHandler(
    protocolVersion,
    socketGroupId,
    _connection,
    config.getContext(),
    config.getLogicalEntityAddress(),
    config.getParameters())
, ::etl::forward_link<0>()
, _connection(config.getContext(), socket, _writeBuffer)
, _transportMessageHandler(
      protocolVersion, diagnosticSendJobBlockPool, protocolSendJobBlockPool, config)
, _writeBuffer{}
, _isMarkedForClose(false)
, _type(type)
{
    addMessageHandler(_transportMessageHandler);
}

bool DoIpServerTransportConnection::send(
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const pNotificationListener)
{
    return _transportMessageHandler.send(transportMessage, pNotificationListener);
}

} // namespace doip
