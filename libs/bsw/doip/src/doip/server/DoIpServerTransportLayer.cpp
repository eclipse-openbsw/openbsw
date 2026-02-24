// Copyright 2025 Accenture.

#include "doip/server/DoIpServerTransportLayer.h"

#include "doip/common/DoIpLock.h"
#include "doip/server/DoIpServerLogger.h"
#include "doip/server/DoIpServerTransportConnection.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/IDoIpServerTransportLayerCallback.h"

#include <transport/TransportMessage.h>

namespace doip
{
using ::estd::slice;
using ::transport::AbstractTransportLayer;
using ::transport::ITransportMessageProcessedListener;
using ::transport::TransportMessage;
using ::util::logger::DOIP;
using ::util::logger::Logger;

DoIpServerTransportLayer::DoIpServerTransportLayer(
    uint8_t const busId,
    uint16_t const logicalEntityAddress,
    ::async::ContextType const context,
    IDoIpServerSocketHandler& socketHandler,
    IDoIpServerTransportConnectionPool& connectionPool,
    DoIpServerTransportLayerParameters const& parameters,
    AliveCheckHelperPool& aliveCheckHelperPool)
: AbstractTransportLayer(busId)
, IDoIpServerTransportConnectionProviderCallback()
, IDoIpServerConnectionHandlerCallback()
, ::transport::ITransportMessageProcessedListener()
, _messageProvidingListenerHelper(fProvidingListenerHelper)
, _connectionConfig(
      busId, logicalEntityAddress, context, _messageProvidingListenerHelper, *this, parameters)
, _connectionProvider(*this, socketHandler, connectionPool, _connectionConfig)
, _functionEventClose(
      ::async::Function::CallType::
          create<DoIpServerTransportLayer, &DoIpServerTransportLayer::executeEventClose>(*this))
, _callback(nullptr)
, _aliveCheckHelperPool(aliveCheckHelperPool)
, _removeLockCount(0U)
{}

AbstractTransportLayer::ErrorCode DoIpServerTransportLayer::init()
{
    _connectionProvider.start();
    return ErrorCode::TP_OK;
}

bool DoIpServerTransportLayer::shutdown(ShutdownDelegate const /* shutdownDelegate */)
{
    _connectionProvider.stop();
    return true;
}

AbstractTransportLayer::ErrorCode DoIpServerTransportLayer::send(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    Logger::debug(
        DOIP,
        "DoIpServerTransportLayer::send(0x%04X -> 0x%04X)",
        transportMessage.sourceAddress(),
        transportMessage.targetAddress());
    {
        RemoveGuard const guard(*this);
        DoIpServerTransportConnection* const connection
            = findRoutingConnectionByInternalSourceAddress(transportMessage.getTargetId());
        if (connection != nullptr)
        {
            bool const success = connection->send(transportMessage, pNotificationListener);
            if (success)
            {
                return ErrorCode::TP_OK;
            }

            return ErrorCode::TP_QUEUE_FULL;
        }
    }
    Logger::warn(
        DOIP,
        "DoIpServerTransportLayer::send(0x%04X -> 0x%04X): No connection found",
        transportMessage.getSourceId(),
        transportMessage.getTargetId());
    return ErrorCode::TP_SEND_FAIL;
}

void DoIpServerTransportLayer::setCallback(IDoIpServerTransportLayerCallback& callback)
{
    _callback = &callback;
}

void DoIpServerTransportLayer::setTransportMessageProvidingListener(
    IDoIpTransportMessageProvidingListener* const messageProvidingListener)
{
    _messageProvidingListenerHelper.setTransportMessageProvidingListener(messageProvidingListener);
}

uint8_t DoIpServerTransportLayer::getConnectionCount(uint8_t const socketGroupId) const
{
    return getSocketGroupConnectionCount(socketGroupId, false);
}

void DoIpServerTransportLayer::closeConnection(
    uint16_t const internalSourceAddress, IDoIpTcpConnection::CloseMode const closeMode)
{
    Logger::debug(
        DOIP,
        "DoIpServerTransportLayer::closeConnection(0x%04x, %d)",
        internalSourceAddress,
        closeMode);
    DoIpServerTransportConnection* connection;
    {
        // RAII mutex
        DoIpLock const lock;
        connection = findRoutingConnectionByInternalSourceAddress(internalSourceAddress);
        if (connection != nullptr)
        {
            Logger::debug(
                DOIP,
                "DoIpServerTransportLayer::suspendConnection(): mark %p for close",
                connection);
            connection->markForClose(closeMode);
        }
    }
    if (connection != nullptr)
    {
        (void)::async::execute(_connectionConfig.getContext(), _functionEventClose);
    }
}

void DoIpServerTransportLayer::closeSocketGroupConnections(
    uint8_t const socketGroupId, IDoIpTcpConnection::CloseMode const closeMode)
{
    Logger::debug(DOIP, "DoIpServerTransportLayer::closeSocketGroupConnections()");
    {
        // RAII mutex
        DoIpLock const lock;
        for (auto& conn : _connections)
        {
            if (conn.getSocketGroupId() == socketGroupId)
            {
                conn.markForClose(closeMode);
            }
        }
    }
    (void)::async::execute(_connectionConfig.getContext(), _functionEventClose);
}

void DoIpServerTransportLayer::closeAllConnections(IDoIpTcpConnection::CloseMode const closeMode)
{
    Logger::debug(DOIP, "DoIpServerTransportLayer::closeAllConnections()");
    {
        // RAII mutex
        DoIpLock const lock;
        for (auto& conn : _connections)
        {
            Logger::debug(
                DOIP, "DoIpServerTransportLayer::closeAllConnections(): mark %p for close", &conn);
            conn.markForClose(closeMode);
        }
    }
    (void)::async::execute(_connectionConfig.getContext(), _functionEventClose);
}

uint8_t DoIpServerTransportLayer::getSocketGroupId(
    uint8_t const serverSocketId, ::ip::IPEndpoint const& remoteEndpoint) const
{
    // side effects are expected in filterConnection
    return (_callback != nullptr) ? _callback->getSocketGroupId(serverSocketId, remoteEndpoint)
                                  : 0U;
}

bool DoIpServerTransportLayer::filterConnection(
    uint8_t const socketGroupId, ::ip::IPEndpoint const& remoteEndpoint)
{
    if (_callback != nullptr)
    {
        uint8_t const maxConnectionCount = _callback->getMaxConnectionCount(socketGroupId);
        return (maxConnectionCount > 0U)
               && (getSocketGroupConnectionCount(socketGroupId, false) <= maxConnectionCount)
               && _callback->filterConnection(socketGroupId, remoteEndpoint);
    }
    return true;
}

void DoIpServerTransportLayer::connectionAccepted(DoIpServerTransportConnection& connection)
{
    {
        // RAII mutex
        DoIpLock const lock;
        _connections.push_front(connection);
    }
    connection.start(*this);
}

void DoIpServerTransportLayer::transportMessageProcessed(
    TransportMessage& transportMessage, ProcessingResult const /* result */)
{
    return fProvidingListenerHelper.releaseTransportMessage(transportMessage);
}

void DoIpServerTransportLayer::execute() { releaseConnections(); }

void DoIpServerTransportLayer::executeEventClose()
{
    Logger::debug(DOIP, "DoIpServerTransportLayer::executeEventClose()");
    RemoveGuard const guard(*this);
    ConnectionList connectionsToClose;
    {
        // RAII mutex
        DoIpLock const lock;
        ConnectionList::iterator prevIt = _connections.before_begin();
        ConnectionList::iterator it     = _connections.begin();
        while (it != _connections.end())
        {
            DoIpServerTransportConnection& connection = *it;
            if (connection.isMarkedForClose())
            {
                it = _connections.erase_after(prevIt);
                connectionsToClose.push_front(connection);
            }
            else
            {
                prevIt = it;
                ++it;
            }
        }
    }
    while (!connectionsToClose.empty())
    {
        DoIpServerTransportConnection& connection = connectionsToClose.front();
        connectionsToClose.pop_front();
        connection.close();
    }
}

IDoIpServerConnectionFilter::RoutingActivationCheckResult
DoIpServerTransportLayer::checkRoutingActivation(
    uint16_t const sourceAddress,
    uint8_t const activationType,
    uint8_t const socketGroupId,
    ::ip::IPEndpoint const& localEndpoint,
    ::ip::IPEndpoint const& remoteEndpoint,
    ::estd::optional<uint32_t> const oemField,
    bool const isResuming)
{
    if (_callback != nullptr)
    {
        return _callback->checkRoutingActivation(
            sourceAddress,
            activationType,
            socketGroupId,
            localEndpoint,
            remoteEndpoint,
            oemField,
            isResuming);
    }

    if (activationType <= 0x01U)
    {
        return RoutingActivationCheckResult();
    }

    return RoutingActivationCheckResult()
        .setAction(Action::REJECT)
        .setResponseCode(DoIpConstants::RoutingResponseCodes::ROUTING_UNSUPPORTED_ACTIVATION_TYPE);
}

void DoIpServerTransportLayer::routingActive(DoIpServerConnectionHandler& handler)
{
    if (_callback != nullptr)
    {
        Logger::warn(DOIP, "DoIpServerTransportLayer::routingActive(%p)", &handler);
        // This class is derived from
        // DoIpServerConnectionHandler and thus it's guaranteed that we can cast safely
        auto const& connection = static_cast<DoIpServerTransportConnection&>(handler);

        auto const sourceAddress         = handler.getSourceAddress();
        auto const internalSourceAddress = handler.getInternalSourceAddress();
        auto const localEndpoint         = handler.getLocalEndpoint();
        auto const remoteEndpoint        = handler.getRemoteEndpoint();
        auto const connectionType        = connection.type();
        _callback->routingActive(
            sourceAddress, internalSourceAddress, localEndpoint, remoteEndpoint, connectionType);
    }
}

void DoIpServerTransportLayer::handleRoutingActivationRequest(
    DoIpServerConnectionHandler& /* handler */)
{
    startAliveCheck();
}

void DoIpServerTransportLayer::aliveCheckResponseReceived(
    DoIpServerConnectionHandler& handler, bool const isAlive)
{
    AliveCheckHelper* const aliveCheckHelper
        = findAliveCheckHelperBySocketGroupId(handler.getSocketGroupId());
    if (aliveCheckHelper != nullptr)
    {
        endAliveCheck(*aliveCheckHelper, isAlive);
    }
    startAliveCheck();
}

void DoIpServerTransportLayer::connectionClosed(DoIpServerConnectionHandler& handler)
{
    // This class is derived from DoIpServerConnectionHandler
    // and thus it's guaranteed that we can cast safely
    DoIpServerTransportConnection& connection
        = static_cast<DoIpServerTransportConnection&>(handler);
    AliveCheckHelper* const helper
        = findAliveCheckHelperBySocketGroupId(handler.getSocketGroupId());
    Logger::warn(
        DOIP,
        "DoIpServerTransportLayer::connectionClosed(%p): %p, %d",
        &handler,
        helper,
        connection.isOrWasRouting());
    if ((helper != nullptr) && (helper->getRoutingActivationHandler() == &handler))
    {
        helper->releaseRoutingActivationHandler();
    }
    if (_callback != nullptr)
    {
        if (connection.isOrWasRouting())
        {
            _callback->connectionClosed(connection.getSourceAddress());
        }
    }
    {
        // RAII mutex
        DoIpLock const lock;
        _connections.remove(connection);
        _connectionsToRelease.push_front(connection);
    }
    (void)::async::execute(_connectionConfig.getContext(), *this);
}

void DoIpServerTransportLayer::startAliveCheck()
{
    Logger::warn(DOIP, "DoIpServerTransportLayer::startAliveCheck()");
    while (!_aliveCheckHelperPool.empty())
    {
        DoIpServerTransportConnection* const routingActivationConnection
            = findActivatingConnection();
        if (routingActivationConnection == nullptr)
        {
            return;
        }
        DoIpServerTransportConnection* const connectionToCheck
            = findRoutingConnectionBySourceAddress(routingActivationConnection->getSourceAddress());
        if (connectionToCheck != nullptr)
        {
            startAliveCheck(*routingActivationConnection, connectionToCheck);
        }
        else
        {
            uint8_t const socketGroupId = routingActivationConnection->getSocketGroupId();
            // no side effects in getters
            // sequence is irrelevant
            if ((_callback == nullptr)
                || (getSocketGroupConnectionCount(socketGroupId, true)
                    < _callback->getMaxConnectionCount(socketGroupId)))
            {
                routingActivationConnection->routingActivationCompleted(
                    true, DoIpConstants::RoutingResponseCodes::ROUTING_SUCCESS);
            }
            else
            {
                startAliveCheck(*routingActivationConnection, nullptr);
            }
        }
    }
}

void DoIpServerTransportLayer::startAliveCheck(
    DoIpServerTransportConnection& routingActivationConnection,
    DoIpServerTransportConnection* const connectionToCheck)
{
    Logger::debug(
        DOIP,
        "DoIpServerTransportLayer::startAliveCheck1(%p, %p)",
        &routingActivationConnection,
        connectionToCheck);
    AliveCheckHelper& aliveCheckHelper = _aliveCheckHelperPool.allocate().construct(
        routingActivationConnection,
        static_cast<uint8_t>(
            (connectionToCheck != nullptr)
                ? DoIpConstants::RoutingResponseCodes::ROUTING_SOURCE_ALREADY_REGISTERED
                : DoIpConstants::RoutingResponseCodes::ROUTING_NO_FREE_SOCKET));
    _aliveCheckHelpers.push_front(aliveCheckHelper);
    if (connectionToCheck != nullptr)
    {
        startAliveCheck(aliveCheckHelper, *connectionToCheck);
    }
    else
    {
        uint8_t const socketGroupId = routingActivationConnection.getSocketGroupId();
        ConnectionList::iterator it;
        {
            // RAII mutex
            DoIpLock const lock;
            it = _connections.begin();
        }
        while (it != _connections.end())
        {
            if ((it->getSocketGroupId() == socketGroupId) && it->isRouting())
            {
                startAliveCheck(aliveCheckHelper, *it);
            }
            ++it;
        }
    }
    endAliveCheck(aliveCheckHelper, true);
}

void DoIpServerTransportLayer::startAliveCheck(
    AliveCheckHelper& aliveCheckHelper, DoIpServerConnectionHandler& handler)
{
    Logger::debug(
        DOIP, "DoIpServerTransportLayer::startAliveCheck2(%p, %p)", &aliveCheckHelper, &handler);
    aliveCheckHelper.startAliveCheck();
    handler.startAliveCheck();
}

void DoIpServerTransportLayer::endAliveCheck(AliveCheckHelper& aliveCheckHelper, bool const alive)
{
    DoIpServerConnectionHandler* const routingActivationHandler
        = aliveCheckHelper.getRoutingActivationHandler();
    if ((!alive) && (routingActivationHandler != nullptr))
    {
        aliveCheckHelper.releaseRoutingActivationHandler();
        routingActivationHandler->routingActivationCompleted(
            true, DoIpConstants::RoutingResponseCodes::ROUTING_SUCCESS);
    }
    if (aliveCheckHelper.endAliveCheck())
    {
        uint8_t const responseCode = aliveCheckHelper.getNegativeResponseCode();
        _aliveCheckHelpers.remove(aliveCheckHelper);
        _aliveCheckHelperPool.release(aliveCheckHelper);
        if (routingActivationHandler != nullptr)
        {
            routingActivationHandler->routingActivationCompleted(false, responseCode);
        }
    }
}

void DoIpServerTransportLayer::releaseConnections()
{
    Logger::debug(DOIP, "DoIpServerTransportLayer::releaseConnections()");
    ConnectionList connectionsToRelease;
    {
        // RAII mutex
        DoIpLock const lock;
        if (_removeLockCount != 0U)
        {
            return;
        }
        _connectionsToRelease.swap(connectionsToRelease);
    }
    while (!connectionsToRelease.empty())
    {
        DoIpServerTransportConnection& connection = connectionsToRelease.front();
        connectionsToRelease.pop_front();
        if ((_callback != nullptr) && connection.isOrWasRouting()
            && (findRoutingConnectionByInternalSourceAddress(connection.getInternalSourceAddress())
                == nullptr))
        {
            _callback->routingInactive(connection.getInternalSourceAddress());
        }
        _connectionProvider.releaseConnection(connection);
    }
}

DoIpServerTransportConnection*
DoIpServerTransportLayer::findRoutingConnectionBySourceAddress(uint16_t const sourceAddress)
{
    // RAII mutex
    DoIpLock const lock;
    for (auto& conn : _connections)
    {
        if (conn.isRouting() && (conn.getSourceAddress() == sourceAddress))
        {
            return &conn;
        }
    }
    return nullptr;
}

DoIpServerTransportConnection*
DoIpServerTransportLayer::findRoutingConnectionByInternalSourceAddress(
    uint16_t const internalSourceAddress)
{
    // RAII mutex
    DoIpLock const lock;
    for (auto& conn : _connections)
    {
        if (conn.isRouting() && (conn.getInternalSourceAddress() == internalSourceAddress))
        {
            return &conn;
        }
    }
    return nullptr;
}

DoIpServerTransportConnection* DoIpServerTransportLayer::findActivatingConnection()
{
    // RAII mutex
    DoIpLock const lock;
    for (auto& conn : _connections)
    {
        if (conn.isActivating()
            && (findAliveCheckHelperBySocketGroupId(conn.getSocketGroupId()) == nullptr))
        {
            return &conn;
        }
    }
    return nullptr;
}

uint8_t DoIpServerTransportLayer::getSocketGroupConnectionCount(
    uint8_t const socketGroupId, bool const active) const
{
    uint8_t connectionCount = 0U;
    // RAII mutex
    DoIpLock const lock;
    for (auto const& conn : _connections)
    {
        if ((conn.getSocketGroupId() == socketGroupId) && ((!active) || conn.isRouting()))
        {
            ++connectionCount;
        }
    }
    return connectionCount;
}

DoIpServerTransportLayer::AliveCheckHelper*
DoIpServerTransportLayer::findAliveCheckHelperBySocketGroupId(uint8_t const socketGroupId)
{
    for (auto& aliveCheckHelper : _aliveCheckHelpers)
    {
        if (aliveCheckHelper.getSocketGroupId() == socketGroupId)
        {
            return &aliveCheckHelper;
        }
    }
    return nullptr;
}

void DoIpServerTransportLayer::setRemoveLock()
{
    // RAII mutex
    DoIpLock const lock;
    ++_removeLockCount;
}

void DoIpServerTransportLayer::releaseRemoveLock()
{
    bool fireMessage;
    {
        // RAII mutex
        DoIpLock const lock;

        --_removeLockCount;
        fireMessage = (_removeLockCount == 0U);
    }
    if (fireMessage)
    {
        (void)::async::execute(_connectionConfig.getContext(), *this);
    }
}

} // namespace doip
