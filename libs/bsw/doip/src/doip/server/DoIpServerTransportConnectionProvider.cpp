// Copyright 2025 Accenture.

#include "doip/server/DoIpServerTransportConnectionProvider.h"

#include "doip/common/DoIpLock.h"
#include "doip/common/IDoIpTcpConnection.h"
#include "doip/server/DoIpServerTransportConnection.h"
#include "doip/server/IDoIpServerSocketHandler.h"
#include "doip/server/IDoIpServerTransportConnectionPool.h"
#include "doip/server/IDoIpServerTransportConnectionProviderCallback.h"

#include <doip/server/DoIpServerLogger.h>

#include <estd/big_endian.h>
#include <estd/memory.h>

namespace doip
{
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.
using ::util::logger::DOIP;
using ::util::logger::Logger;

using ::tcp::AbstractSocket;

DoIpServerTransportConnectionProvider::DoIpServerTransportConnectionProvider(
    IDoIpServerTransportConnectionProviderCallback& callback,
    IDoIpServerSocketHandler& socketHandler,
    IDoIpServerTransportConnectionPool& connectionPool,
    DoIpServerTransportConnectionConfig const& config)
: _callback(callback)
, _socketHandler(socketHandler)
, _connectionPool(connectionPool)
, _config(config)
{}

void DoIpServerTransportConnectionProvider::start() { _socketHandler.start(*this); }

void DoIpServerTransportConnectionProvider::stop() { _socketHandler.stop(); }

void DoIpServerTransportConnectionProvider::releaseConnection(
    DoIpServerTransportConnection& connection)
{
    Logger::debug(
        DOIP, "DoIpServerTransportConnectionProvider::releaseConnection(%p)", &connection);
    AbstractSocket& socket = connection.getConnection().getSocket();
    // Cancel timeouts outside of _connectionPool.releaseConnection() to avoid nested critical
    // sections and deadlock. Suspend sending to avoid new timeouts being set.
    connection.suspendSending();
    connection.getConnection().shutdown();
    connection.shutdown();
    {
        // RAII mutex
        DoIpLock const lock;
        _connectionPool.releaseConnection(connection);
    }
    _socketHandler.releaseSocket(socket, connection.type());
}

void DoIpServerTransportConnectionProvider::serverSocketBound(
    uint8_t const serverSocketId,
    ip::IPEndpoint const& localEndpoint,
    DoIpTcpConnection::ConnectionType const type)
{
    (void)serverSocketId;
    (void)localEndpoint;
    if (type != DoIpTcpConnection::ConnectionType::PLAIN)
    {
        return;
    }

    AbstractSocket* const socket = _socketHandler.acquireSocket();
    if (socket != nullptr)
    {
        _socketHandler.releaseSocket(*socket, type);
    }
}

bool DoIpServerTransportConnectionProvider::filterConnection(
    uint8_t const serverSocketId, ip::IPEndpoint const& remoteEndpoint)
{
    uint8_t const socketGroupId = _callback.getSocketGroupId(serverSocketId, remoteEndpoint);
    return _callback.filterConnection(socketGroupId, remoteEndpoint);
}

void DoIpServerTransportConnectionProvider::connectionAccepted(
    uint8_t const serverSocketId,
    ::tcp::AbstractSocket& socket,
    DoIpTcpConnection::ConnectionType const type)
{
    DoIpServerTransportConnection* const connection = _connectionPool.createConnection(
        getSocketGroupId(serverSocketId, socket), socket, _config, type);
    if (connection != nullptr)
    {
        _callback.connectionAccepted(*connection);
    }
    else
    {
        Logger::warn(
            DOIP,
            "DoIpServerTransportConnectionProvider::connectionAccepted(): Failed to create "
            "DoIpServerTransportConnection.");
        _socketHandler.releaseSocket(socket, type);
    }
}

uint8_t DoIpServerTransportConnectionProvider::getSocketGroupId(
    uint8_t const serverSocketId, ::tcp::AbstractSocket const& socket) const
{
    auto const remoteIpAddress = socket.getRemoteIPAddress();
    auto const remotePort      = socket.getRemotePort();
    return _callback.getSocketGroupId(
        serverSocketId, ::ip::IPEndpoint(remoteIpAddress, remotePort));
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace doip
