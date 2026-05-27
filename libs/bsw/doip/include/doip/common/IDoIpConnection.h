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

#include <ip/IPEndpoint.h>

#include <etl/delegate.h>
#include <etl/span.h>

namespace doip
{
class IDoIpConnectionHandler;
class IDoIpSendJob;

/**
 * Interface for a DoIp connection.
 */
class IDoIpConnection
{
public:
    using PayloadReceivedCallbackType  = ::etl::delegate<void(::etl::span<uint8_t const>)>;
    using PayloadDiscardedCallbackType = ::etl::delegate<void()>;

    IDoIpConnection();

    /**
     * Initialize the DoIP with the handler (i.e. the callback interface).
     * \param handler reference to the handler
     */
    virtual void init(IDoIpConnectionHandler& handler) = 0;

    /**
     * Get the local endpoint that received the currently processed message.
     * \return the local endpoint of the message. This may be empty for pure packet based
     * connections
     */
    virtual ::ip::IPEndpoint getLocalEndpoint() const = 0;

    /**
     * Get the remote endpoint of the sender of the currently processed message.
     * \return remote endpoint of the message. This may be empty for pure packet based connections
     */
    virtual ::ip::IPEndpoint getRemoteEndpoint() const = 0;

    /**
     * This method will typically be called to receive parts of a message payload after reception of
     * a DoIP header over the connection. \param payload reference to the buffer to use for
     * receiving parts of the payload. The size of the buffer indicates the exact number of bytes to
     * receive. The received payload will be copied to this buffer \param payloadReceivedCallback
     * mandatory function that will be called if the specified number of bytes has been read
     * successfully \return true if the function call is valid and expected
     */
    virtual bool receivePayload(
        ::etl::span<uint8_t> payload, PayloadReceivedCallbackType payloadReceivedCallback)
        = 0;

    /**
     * This method can be called at any time during reception of a DoIP message.
     * Any pending rest of the message will be discarded silently.
     *
     * \param payloadDiscardedCallback optional callback upon discard of the remaining payload
     */
    virtual void endReceiveMessage(PayloadDiscardedCallbackType payloadDiscardedCallback) = 0;

    /**
     * Send a DoIP Message represented by a send job. The send jobs keeps all data and can be used
     * asynchronously. The message will be released once it has been sent successfully or the
     * connection closes before sending has been done. Check success of sending with
     * DoIpSendJob::isDone(). \param sendJob asynchronous send job holding all data of the DoIP
     * message \return true if the function call is valid and expected
     */
    virtual bool sendMessage(IDoIpSendJob& sendJob) = 0;

    /**
     * Close the underlying IP connection and detach the handler from the DoIP connection.
     * \note all pending send jobs are released. Check success of sending of each send job with
     *       DoIpSendJob::isDone()
     */
    virtual void close() = 0;

    /**
     * Suspends sending of pending send jobs. Works with nested calls.
     */
    virtual void suspendSending() = 0;

    /**
     * Resumes sending.
     */
    virtual void resumeSending() = 0;

protected:
    ~IDoIpConnection()                                 = default;
    IDoIpConnection& operator=(IDoIpConnection const&) = delete;
};

/**
 * Inline implementation.
 */
inline IDoIpConnection::IDoIpConnection() {}

} // namespace doip
