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

#include "doip/common/DoIpTransportMessageProvidingListenerHelper.h"
#include "doip/server/DoIpServerTransportConnection.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/DoIpServerTransportConnectionProvider.h"
#include "doip/server/IDoIpServerConnectionHandlerCallback.h"
#include "doip/server/IDoIpServerTransportConnectionProviderCallback.h"

#include <async/Types.h>
#include <async/util/Call.h>
#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>
#include <etl/optional.h>
#include <etl/pool.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProcessedListener.h>

namespace doip
{
class IDoIpServerTransportConnectionPool;
class IDoIpServerTransportLayerCallback;

/**
 * Class implementing a transport layer on a given bus.
 */
// multiple inheritance of interfaces is OK
class DoIpServerTransportLayer
: public ::transport::AbstractTransportLayer
, private IDoIpServerTransportConnectionProviderCallback
, private IDoIpServerConnectionHandlerCallback
, private ::transport::ITransportMessageProcessedListener
, private ::async::RunnableType
{
public:
    ErrorCode init() override;
    bool shutdown(ShutdownDelegate shutdownDelegate) override;
    ErrorCode send(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener) override;

    /**
     * Set a callback interface for checking creation of connections and routing activation.
     * \param callback reference to callback interface
     */
    void setCallback(IDoIpServerTransportLayerCallback& callback);

    /**
     * Set DoIP specific message providing listener
     * \param messageProvidingListener pointer to DoIP specific message providing listener interface
     */
    void setTransportMessageProvidingListener(
        IDoIpTransportMessageProvidingListener* messageProvidingListener);

    /**
     * Get number of active connections for a socket group
     * \param socketGroupId identifier of socket group to get number of active connections for
     * \return number of active connections
     */
    uint8_t getConnectionCount(uint8_t socketGroupId) const;

    /**
     * Close a specific connection if existing.
     * \param internalSourceAddress internal address of the connection (as returned from routing
     * activation) \param closeMode mode to use for closing the connection
     */
    void closeConnection(uint16_t internalSourceAddress, IDoIpTcpConnection::CloseMode closeMode);

    /**
     * Close all connections of a socket group.
     * \param socketGroupId identifier of socket group
     * \param closeMode mode to use for closing the connections
     */
    void
    closeSocketGroupConnections(uint8_t socketGroupId, IDoIpTcpConnection::CloseMode closeMode);

    /**
     * Close all connections. In contrast to shutdown() new connections can be established
     * after call.
     * \param closeMode mode to use for closing the connections
     */
    void closeAllConnections(IDoIpTcpConnection::CloseMode closeMode);

protected:
    class AliveCheckHelper : public ::etl::forward_link<0>
    {
    public:
        AliveCheckHelper(
            DoIpServerConnectionHandler& routingActivationHandler, uint8_t negativeResponseCode);

        uint8_t getSocketGroupId() const;
        uint8_t getNegativeResponseCode() const;
        DoIpServerConnectionHandler* getRoutingActivationHandler() const;

        void startAliveCheck();
        bool endAliveCheck();

        void releaseRoutingActivationHandler();

    private:
        DoIpServerConnectionHandler* _routingActivationHandler;
        uint8_t _socketGroupId;
        uint8_t _negativeResponseCode;
        uint8_t _pendingAliveCheckCount;
    };

    using AliveCheckHelperList
        = ::etl::intrusive_forward_list<AliveCheckHelper, ::etl::forward_link<0>>;
    using AliveCheckHelperPool = ::etl::ipool;

    /**
     * Constructor.
     */
    DoIpServerTransportLayer(
        uint8_t busId,
        uint16_t logicalEntityAddress,
        ::async::ContextType context,
        IDoIpServerSocketHandler& socketHandler,
        IDoIpServerTransportConnectionPool& connectionPool,
        DoIpServerTransportLayerParameters const& parameters,
        AliveCheckHelperPool& aliveCheckHelperPool);

    ~DoIpServerTransportLayer() = default;

private:
    using ConnectionList
        = ::etl::intrusive_forward_list<DoIpServerTransportConnection, ::etl::forward_link<0>>;

    static uint16_t const EVENT_CLOSE   = 0U;
    static uint16_t const EVENT_RELEASE = 1U;

    class RemoveGuard final
    {
    public:
        explicit RemoveGuard(DoIpServerTransportLayer& layer);
        ~RemoveGuard();

    private:
        DoIpServerTransportLayer& _layer;
    };
    friend class RemoveGuard;

    uint8_t
    getSocketGroupId(uint8_t serverSocketId, ::ip::IPEndpoint const& remoteEndpoint) const override;
    bool filterConnection(uint8_t socketGroupId, ::ip::IPEndpoint const& remoteEndpoint) override;
    void connectionAccepted(DoIpServerTransportConnection& connection) override;

    RoutingActivationCheckResult checkRoutingActivation(
        uint16_t sourceAddress,
        uint8_t activationType,
        uint8_t socketGroupId,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint,
        ::etl::optional<uint32_t> const oemField,
        bool isResuming) override;
    void routingActive(DoIpServerConnectionHandler& handler) override;
    void handleRoutingActivationRequest(DoIpServerConnectionHandler& handler) override;
    void aliveCheckResponseReceived(DoIpServerConnectionHandler& handler, bool isAlive) override;
    void connectionClosed(DoIpServerConnectionHandler& handler) override;

    void transportMessageProcessed(
        ::transport::TransportMessage& transportMessage, ProcessingResult result) override;

    void execute() override;
    void executeEventClose();

    void startAliveCheck();
    void startAliveCheck(
        DoIpServerTransportConnection& routingActivationConnection,
        DoIpServerTransportConnection* connectionToCheck);
    void endAliveCheck(AliveCheckHelper& aliveCheckHelper, bool alive);

    static void
    startAliveCheck(AliveCheckHelper& aliveCheckHelper, DoIpServerConnectionHandler& handler);

    void releaseConnections();

    void setRemoveLock();
    void releaseRemoveLock();

    DoIpServerTransportConnection* findRoutingConnectionBySourceAddress(uint16_t sourceAddress);
    DoIpServerTransportConnection*
    findRoutingConnectionByInternalSourceAddress(uint16_t internalSourceAddress);
    DoIpServerTransportConnection* findActivatingConnection();
    AliveCheckHelper* findAliveCheckHelperBySocketGroupId(uint8_t socketGroupId);

    uint8_t getSocketGroupConnectionCount(uint8_t socketGroupId, bool active) const;

    DoIpTransportMessageProvidingListenerHelper _messageProvidingListenerHelper;
    DoIpServerTransportConnectionConfig _connectionConfig;
    DoIpServerTransportConnectionProvider _connectionProvider;
    ConnectionList _connections;
    ConnectionList _connectionsToRelease;
    ::async::Function _functionEventClose;
    IDoIpServerTransportLayerCallback* _callback;
    AliveCheckHelperPool& _aliveCheckHelperPool;
    AliveCheckHelperList _aliveCheckHelpers;
    uint8_t _removeLockCount;
};

namespace declare
{
/**
 * Class implementing a transport layer on a given bus supporting a number of socket groups.
 * Socket groups are used to create partitions of connections that are independent regarding
 * alive checks
 * \tparam NUM_SOCKET_GROUPS number of socket groups to support
 */
template<size_t NUM_SOCKET_GROUPS>
class DoIpServerTransportLayer : public ::doip::DoIpServerTransportLayer
{
public:
    /**
     * Constructor.
     * \param busId bus identifier of transport layer
     * \param logicalEntityAddress address of represented ECU
     * \param asynchronous execution context
     * \param socketHandler reference to socket handler interface
     * \param connectionPool
     */
    DoIpServerTransportLayer(
        uint8_t busId,
        uint16_t logicalEntityAddress,
        ::async::ContextType context,
        IDoIpServerSocketHandler& socketHandler,
        IDoIpServerTransportConnectionPool& connectionPool,
        DoIpServerTransportLayerParameters const& parameters);

private:
    ::etl::pool<AliveCheckHelper, NUM_SOCKET_GROUPS> _aliveCheckHelperPool;
};

} // namespace declare

/**
 * Inline implementations.
 */
inline DoIpServerTransportLayer::AliveCheckHelper::AliveCheckHelper(
    DoIpServerConnectionHandler& routingActivationHandler, uint8_t const negativeResponseCode)
: _routingActivationHandler(&routingActivationHandler)
, _socketGroupId(routingActivationHandler.getSocketGroupId())
, _negativeResponseCode(negativeResponseCode)
, _pendingAliveCheckCount(1U)
{}

inline uint8_t DoIpServerTransportLayer::AliveCheckHelper::getSocketGroupId() const
{
    return _socketGroupId;
}

inline uint8_t DoIpServerTransportLayer::AliveCheckHelper::getNegativeResponseCode() const
{
    return _negativeResponseCode;
}

// handler is managed from the outside
inline DoIpServerConnectionHandler*
DoIpServerTransportLayer::AliveCheckHelper::getRoutingActivationHandler() const
{
    return _routingActivationHandler;
}

inline void DoIpServerTransportLayer::AliveCheckHelper::startAliveCheck()
{
    ++_pendingAliveCheckCount;
}

inline bool DoIpServerTransportLayer::AliveCheckHelper::endAliveCheck()
{
    --_pendingAliveCheckCount;
    return _pendingAliveCheckCount == 0U;
}

inline void DoIpServerTransportLayer::AliveCheckHelper::releaseRoutingActivationHandler()
{
    _routingActivationHandler = nullptr;
}

inline DoIpServerTransportLayer::RemoveGuard::RemoveGuard(DoIpServerTransportLayer& layer)
: _layer(layer)
{
    _layer.setRemoveLock();
}

inline DoIpServerTransportLayer::RemoveGuard::~RemoveGuard() { _layer.releaseRemoveLock(); }

namespace declare
{
template<size_t NUM_SOCKET_GROUPS>
DoIpServerTransportLayer<NUM_SOCKET_GROUPS>::DoIpServerTransportLayer(
    uint8_t const busId,
    uint16_t const logicalEntityAddress,
    ::async::ContextType const context,
    IDoIpServerSocketHandler& socketHandler,
    IDoIpServerTransportConnectionPool& connectionPool,
    DoIpServerTransportLayerParameters const& parameters)
: ::doip::DoIpServerTransportLayer(
    busId,
    logicalEntityAddress,
    context,
    socketHandler,
    connectionPool,
    parameters,
    _aliveCheckHelperPool)
{}

} // namespace declare

} // namespace doip
