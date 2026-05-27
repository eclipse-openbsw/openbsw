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
#include "doip/common/IDoIpConnectionHandler.h"
#include "doip/common/IDoIpSendJob.h"
#include "doip/common/IDoIpTcpConnection.h"

#include <async/Types.h>
#include <tcp/IDataListener.h>
#include <tcp/IDataSendNotificationListener.h>
#include <tcp/socket/AbstractSocket.h>

#include <etl/intrusive_links.h>
#include <etl/intrusive_list.h>
#include <etl/span.h>

namespace doip
{
/**
 * This class is dedicated for transmission and reception of arbitrary DoIP messages
 * over a TCP socket once a connection has been established. It is not in the scope
 * of the DoIpTcpConnection to establish a connection. For the connection the following
 * constraints are valid:
 * - Whenever a message is received both sourceAddress and destinationAddress are nullptr.
 * - A call to receivePayload() or endReceiveMessage() may be done asynchronously after
 *   reception of the header.
 */
class DoIpTcpConnection
: public IDoIpTcpConnection
, private ::tcp::IDataListener
, private ::tcp::IDataSendNotificationListener
, private ::async::RunnableType
{
public:
    enum class ConnectionType : uint8_t
    {
        PLAIN,
        TLS,
    };

    /**
     * Constructor.
     * \param context asynchronous execution context
     * \param socket reference to TCP socket to use
     * \param writeBuffer buffer that is available for writing (>= 8 bytes)
     */
    DoIpTcpConnection(
        ::async::ContextType context,
        ::tcp::AbstractSocket& socket,
        ::etl::span<uint8_t> writeBuffer);
    /**
     * Get access to the socket.
     * \return reference to the socket
     */
    ::tcp::AbstractSocket& getSocket();

    void init(IDoIpConnectionHandler& handler) override;

    ::ip::IPEndpoint getLocalEndpoint() const override;

    ::ip::IPEndpoint getRemoteEndpoint() const override;

    bool receivePayload(
        ::etl::span<uint8_t> payload, PayloadReceivedCallbackType payloadReceivedCallback) override;
    void endReceiveMessage(PayloadDiscardedCallbackType payloadDiscardedCallback) override;

    bool sendMessage(IDoIpSendJob& sendJob) override;

    void close() override;

    void setCloseMode(CloseMode closeMode) override;

    void detach(DetachCallbackType detachCallback) override;

    void suspendSending() override;

    void resumeSending() override;

    /**
     * Cancel the timeout.
     */
    void shutdown();

private:
    using SendJobList = ::etl::intrusive_list<IDoIpSendJob, ::etl::bidirectional_link<0>>;

    void dataReceived(uint16_t length) override;
    void connectionClosed(ErrorCode status) override;

    void dataSent(uint16_t length, SendResult result) override;

    bool processCurrentSendBuffer(IDoIpSendJob& sendJob);

    void selectNextSendJob(IDoIpSendJob& currentSendJob);

    void execute() override;

    enum class ConnectionState : uint8_t
    {
        INIT,
        ACTIVE,
        INACTIVE,
        ERROR
    };

    enum class ReadState : uint8_t
    {
        HEADER,
        PAYLOAD,
        DISCARD
    };

    void processNextReadChunk(size_t const bytesRead);
    void tryReceive();
    void handleDataSent();
    void
    closeConnection(ConnectionState connectionState, bool closedByRemotePeer, bool closeSocket);
    void setReadBuffer(::etl::span<uint8_t> const& readBuffer);
    void fireMessageIfNotSuspended();

    static void releaseSendJobs(SendJobList& sendJobs);

    ::tcp::AbstractSocket& _socket;
    ::etl::span<uint8_t> _currentReadBuffer;
    ::etl::span<uint8_t> _writeBuffer;
    IDoIpConnectionHandler* _handler;
    PayloadReceivedCallbackType _payloadReceivedCallback;
    PayloadDiscardedCallbackType _payloadDiscardedCallback;
    DetachCallbackType _detachCallback;
    SendJobList _pendingSendJobs;
    SendJobList _sentJobs;
    uint8_t _headerBuffer[DoIpConstants::DOIP_HEADER_LENGTH];
    size_t _readPayloadLength;
    size_t _receivedBufferLength;
    size_t _availableReadDataLength;
    size_t _sentDataLength;
    size_t _pendingSendDataLength;
    size_t _sentJobTotalLength;
    ::async::TimeoutType _sendTimeout;
    ::async::ContextType const _context;
    ConnectionState _connectionState;
    ReadState _readState;
    CloseMode _closeMode;
    uint8_t _sendBufferIndex;
    uint8_t _suspendCallCounter;
    bool _recurseRead;
    bool _recurseWrite;
};

/**
 * Inline implementations.
 */
inline ::tcp::AbstractSocket& DoIpTcpConnection::getSocket() { return _socket; }

inline void DoIpTcpConnection::setReadBuffer(::etl::span<uint8_t> const& readBuffer)
{
    _currentReadBuffer    = readBuffer;
    _receivedBufferLength = 0U;
}

} // namespace doip
