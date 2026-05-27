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
#include "doip/common/DoIpStaticPayloadSendJob.h"
#include "doip/common/IDoIpConnectionHandler.h"
#include "doip/common/IDoIpTcpConnection.h"
#include "doip/server/IDoIpServerConnection.h"
#include "doip/server/IDoIpServerMessageHandler.h"

#include <async/Types.h>
#include <etl/intrusive_links.h>
#include <etl/intrusive_list.h>
#include <etl/pool.h>

namespace doip
{
class DoIpServerTransportLayerParameters;
class IDoIpServerConnectionHandlerCallback;

/**
 * Class that represents a connection handler for a server connection.
 * It is responsible for the base connection handling with routing activation and socket handlings.
 */
// multiple inheritance of interfaces is OK
class DoIpServerConnectionHandler
: private IDoIpConnectionHandler
, private IDoIpServerConnection
, private ::async::RunnableType
{
public:
    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param socketGroupId identifier of socket group the connection belongs to
     * \param connection base connection to work on
     * \param context asynchronous execution context
     * \param logicalEntityAddress address of the server
     * \param parameters reference to transport layer parameters
     */
    DoIpServerConnectionHandler(
        DoIpConstants::ProtocolVersion protocolVersion,
        uint8_t socketGroupId,
        IDoIpTcpConnection& connection,
        ::async::ContextType context,
        uint16_t logicalEntityAddress,
        DoIpServerTransportLayerParameters const& parameters);

    /**
     * Get the socket group identifier for this connection.
     * \return identifier
     */
    uint8_t getSocketGroupId() const;

    /**
     * Check whether this connection has requested for routing activation
     * \return true if the connection is activating routing
     */
    bool isActivating() const;

    /**
     * Check whether this connection is routing.
     * \return true if the connection is routing
     */
    bool isRouting() const;

    /**
     * Check whether this connection is or was routing.
     * \return true if the connection is or was routing
     */
    bool isOrWasRouting() const;

    /**
     * Check whether this connection has been closed.
     * \return true if the connection has been closed
     */
    bool isClosed() const;

    /**
     * Get the local endpoint of this connection.
     * \return the local endpoint
     */
    ::ip::IPEndpoint getLocalEndpoint() const override;

    /**
     * Get the remote endpoint of this connection.
     * \return the local endpoint
     */
    ::ip::IPEndpoint getRemoteEndpoint() const override;

    /**
     * Get the source address attached to this connection.
     * \return source address if attached, INVALID_ADDRESS otherwise
     */
    uint16_t getSourceAddress() const override;

    /**
     * Get the internal source address to be used for routing.
     * \return internal source address if attached, INVALID_ADDRESS otherwise
     */
    uint16_t getInternalSourceAddress() const override;

    /**
     * Add a message handler.
     * \param messageHandler message handler
     */
    void addMessageHandler(IDoIpServerMessageHandler& messageHandler);

    /**
     * Resume the connection. This can be only done just after construction and before
     * initialization. \param sourceAddress active source address \return true if resume was
     * successful
     */
    bool resume(uint16_t sourceAddress);

    /**
     * Start the connection.
     * \param callback callback interface that handles this connection
     */
    void start(IDoIpServerConnectionHandlerCallback& callback);

    /**
     * Close the connection.
     */
    void close() override;

    /**
     * Start alive check.
     */
    void startAliveCheck();

    /**
     * Called to indicate result of routing activation.
     * \param success true if routing was successful and thus the connection is allowed to route
     * \param responseCode response code to send
     */
    void routingActivationCompleted(bool success, uint8_t responseCode);

    void suspendSending() override;

    void resumeSending() override;

    /**
     * Cancel the timeout.
     */
    void shutdown();

private:
    enum class State : uint8_t
    {
        INACTIVE,
        ACTIVATING,
        ACTIVE,
        CLOSING,
        SHUTDOWN
    };

    using StaticPayloadSendJobType = declare::DoIpStaticPayloadSendJob<9U>;
    using MessageHandlerList
        = ::etl::intrusive_list<IDoIpServerMessageHandler, ::etl::bidirectional_link<0>>;

    void connectionClosed(bool closedByRemotePeer) override;
    IDoIpConnectionHandler::HeaderReceivedContinuation
    headerReceivedRoutingActivationRequest(DoIpHeader const& header);
    IDoIpConnectionHandler::HeaderReceivedContinuation
    headerReceivedAliveCheckResponse(DoIpHeader const& header);
    IDoIpConnectionHandler::HeaderReceivedContinuation
    headerReceivedNegativeAck(DoIpHeader const& header);
    IDoIpConnectionHandler::HeaderReceivedContinuation
    headerReceivedDefault(DoIpHeader const& header);
    IDoIpConnectionHandler::HeaderReceivedContinuation
    headerReceived(DoIpHeader const& header) override;

    void execute() override;

    void init(IDoIpConnectionHandler& handler) override;
    bool receivePayload(
        ::etl::span<uint8_t> payload, PayloadReceivedCallbackType payloadReceivedCallback) override;
    void endReceiveMessage(
        IDoIpConnection::PayloadDiscardedCallbackType payloadDiscardedCallback) override;
    bool sendMessage(IDoIpSendJob& sendJob) override;
    void sendNack(uint8_t nackCode, bool closeAfterSend) override;

    void routingActivationRequestReceived(::etl::span<uint8_t const> payload);
    void aliveCheckResponseReceived(::etl::span<uint8_t const> payload);

    void sendRoutingActivationResponse(
        uint16_t sourceAddress, uint8_t responseCode, bool closeAfterSend);

    void sendConnectionOpened();
    void sendRoutingActive();

    void setTimer(uint32_t timeoutInMs);
    void timerExpired();

    bool sendOrReleaseMessage(DoIpStaticPayloadSendJob& job);

    StaticPayloadSendJobType*
    allocateSendJob(uint16_t payloadType, uint16_t payloadLength, bool closeAfterSend);

    void releaseSendJobAndClose(DoIpStaticPayloadSendJob& job, bool success);
    void releaseSendJob(DoIpStaticPayloadSendJob& job, bool success);

    IDoIpServerConnectionHandlerCallback* _callback;
    IDoIpTcpConnection& _connection;
    DoIpServerTransportLayerParameters const& _parameters;
    ::async::TimeoutType _timerTimeout;
    ::etl::pool<StaticPayloadSendJobType, 2U> _sendJobPool;
    MessageHandlerList _messageHandlers;
    uint16_t _logicalEntityAddress;
    uint16_t _sourceAddress;
    uint16_t _internalSourceAddress;
    DoIpConstants::ProtocolVersion _protocolVersion;
    ::async::ContextType const _context;
    State _state;
    uint8_t _socketGroupId;
    uint8_t _readBuffer[11];
    bool _aliveCheckPending;
    bool _isOrWasRouting;
};

/**
 * Inline implementations.
 */
// message destructor is called
inline void DoIpServerConnectionHandler::init(IDoIpConnectionHandler& /* handler */) {}

inline uint8_t DoIpServerConnectionHandler::getSocketGroupId() const { return _socketGroupId; }

inline bool DoIpServerConnectionHandler::isActivating() const
{
    return _state == State::ACTIVATING;
}

inline bool DoIpServerConnectionHandler::isRouting() const { return _state == State::ACTIVE; }

inline bool DoIpServerConnectionHandler::isOrWasRouting() const { return _isOrWasRouting; }

inline bool DoIpServerConnectionHandler::isClosed() const { return _state == State::SHUTDOWN; }

inline uint16_t DoIpServerConnectionHandler::getSourceAddress() const { return _sourceAddress; }

inline uint16_t DoIpServerConnectionHandler::getInternalSourceAddress() const
{
    return _internalSourceAddress;
}

inline ::ip::IPEndpoint DoIpServerConnectionHandler::getLocalEndpoint() const
{
    return _connection.getLocalEndpoint();
}

inline ::ip::IPEndpoint DoIpServerConnectionHandler::getRemoteEndpoint() const
{
    return _connection.getRemoteEndpoint();
}

inline bool DoIpServerConnectionHandler::receivePayload(
    ::etl::span<uint8_t> payload, PayloadReceivedCallbackType const payloadReceivedCallback)
{
    return _connection.receivePayload(payload, payloadReceivedCallback);
}

inline void DoIpServerConnectionHandler::endReceiveMessage(
    IDoIpConnection::PayloadDiscardedCallbackType const payloadDiscardedCallback)
{
    _connection.endReceiveMessage(payloadDiscardedCallback);
}

inline bool DoIpServerConnectionHandler::sendMessage(IDoIpSendJob& sendJob)
{
    return _connection.sendMessage(sendJob);
}

} // namespace doip
