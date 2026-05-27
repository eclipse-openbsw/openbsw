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

#include "doip/common/DoIpHeader.h"
#include "doip/common/IDoIpConnection.h"
#include "doip/common/IDoIpConnectionHandler.h"
#include "doip/common/IDoIpSendJob.h"

#include <async/Types.h>
#include <udp/IDataListener.h>
#include <udp/IDataSentListener.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <etl/intrusive_links.h>
#include <etl/intrusive_list.h>

namespace doip
{
/**
 * This class is dedicated for transmission and reception of arbitrary DoIP messages
 * over a UDP socket. It is not in the scope of the DoIpUdpConnection to initialize the socket.
 * For UDP connections there are some restrictions:
 * - Received messages are not asynchronously available: So if the method receivePayload() isn't
 *   called from within IDoIpConnectionHandler::headerReceived() or a subsequent callback set
 *   by receivePayload() the message data will be lost.
 * - All send jobs need to have an attached destination endpoint to allow correct addressing
 *   for the datagram
 * - Whenever a message is received both sourceAddress and destinationAddress of the function
 *   IDoIpConnectionHandler::headerReceived() will be set
 * - The used write buffer should be big enough to hold the maximum DoIP datagram to send. If a
 *   send job consists of only a single buffer a copy to the write buffer can be avoided. Thus the
 * rule regarding the size of the write buffer can be stated more precisely: The used write buffer
 *   should be set to the size of the maximum send job consisting of more than one buffer.
 */
class DoIpUdpConnection
: public IDoIpConnection
, private ::udp::IDataListener
, private ::async::RunnableType
{
public:
    /**
     * Constructor.
     * \param context asynchronous execution context
     * \param socket reference to UDP socket to use
     * \param writeBuffer buffer that will typically used to hold a complete DoIP datagram
     */
    DoIpUdpConnection(
        ::async::ContextType context,
        ::udp::AbstractDatagramSocket& socket,
        ::etl::span<uint8_t> writeBuffer);

    /**
     * Get access to the UDP socket.
     * \return reference to the socket
     */
    ::udp::AbstractDatagramSocket& getSocket();

    void init(IDoIpConnectionHandler& handler) override;

    ::ip::IPEndpoint getLocalEndpoint() const override;

    ::ip::IPEndpoint getRemoteEndpoint() const override;

    bool receivePayload(
        ::etl::span<uint8_t> payload, PayloadReceivedCallbackType payloadReceivedCallback) override;
    void endReceiveMessage(PayloadDiscardedCallbackType payloadDiscardedCallback) override;

    bool sendMessage(IDoIpSendJob& sendJob) override;

    void close() override;

    void suspendSending() override;

    void resumeSending() override;

private:
    using SendJobList = ::etl::intrusive_list<IDoIpSendJob, ::etl::bidirectional_link<0>>;

    void dataReceived(
        ::udp::AbstractDatagramSocket& socket,
        ::ip::IPAddress sourceAddress,
        uint16_t sourcePort,
        ::ip::IPAddress destinationAddress,
        uint16_t length) override;

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

    void trySend();
    void tryReceive();
    ::etl::span<uint8_t const> prepareSendBuffer(IDoIpSendJob& job);
    ::etl::span<uint8_t const> fillWriteBuffer(IDoIpSendJob& job);
    void closeConnection(ConnectionState connectionState);

    static void releaseSendJobs(SendJobList& sendJobs);

    IDoIpSendJob* popSendJob();

    ::async::TimeoutType _sendTimeout;
    ::udp::AbstractDatagramSocket& _socket;
    ::etl::span<uint8_t> _currentReadBuffer;
    ::etl::span<uint8_t> _writeBuffer;
    IDoIpConnectionHandler* _handler;
    ::ip::IPEndpoint _localEndpoint;
    ::ip::IPEndpoint _remoteEndpoint;
    PayloadReceivedCallbackType _payloadReceivedCallback;
    SendJobList _pendingSendJobs;
    uint16_t _availableReadDataLength;
    uint16_t _readPayloadLength;
    ::async::ContextType const _context;
    uint8_t _headerBuffer[DoIpConstants::DOIP_HEADER_LENGTH];
    ConnectionState _connectionState;
    ReadState _readState;
    uint8_t _suspendCallCounter;
};

/**
 * Inline implementations.
 */

inline ::udp::AbstractDatagramSocket& DoIpUdpConnection::getSocket() { return _socket; }

inline ::ip::IPEndpoint DoIpUdpConnection::getLocalEndpoint() const { return _localEndpoint; }

inline ::ip::IPEndpoint DoIpUdpConnection::getRemoteEndpoint() const { return _remoteEndpoint; }

} // namespace doip
