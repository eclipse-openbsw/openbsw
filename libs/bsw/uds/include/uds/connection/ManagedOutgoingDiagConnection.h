/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/intrusive_list.h>
#include <etl/queue.h>

#include "uds/connection/OutgoingDiagConnection.h"

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;
} // namespace transport

namespace uds
{
struct TransportJob;

/**
 * A ManagedOutgoingDiagConnection is used by OutgoingDiagConnectionManager.
 *
 * This class exposes management functions that allow the OutgoingDiagConnectionManager
 * to set parameters of the connection that are read-only for a
 * AbstractDiagApplication which is the reason why the reduced interface of
 * OutgoingDiagConnection is used there.
 */
class ManagedOutgoingDiagConnection
: public ::etl::bidirectional_link<0>
, public OutgoingDiagConnection
{
public:
    ManagedOutgoingDiagConnection();

    void open()
    {
        _open                     = true;
        _suppressIncomingPendings = true;
    }

    /**
     * Sets the source address for this connection.
     */
    void setSourceAddress(uint16_t sourceAddress);

    /**
     * Sets the target address for this connection.
     */
    void setTargetAddress(uint16_t targetAddress);

    /**
     * Returns the number of bytes matching a received response.
     */
    uint16_t isExpectedResponse(transport::TransportMessage const& transportMessage);

    /**
     * Notifies the connection that a response has been received.
     * \note * This method does not doublecheck if transportMessage is an expected
     * response of this connection, so call isExpectedResponse before!
     */
    void responseReceived(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener);

    void processResponseQueue();

    void terminate() override;

    void responseProcessed() override;

    void timeoutOccurred() override;

private:
    using TransportJobQueue = ::etl::iqueue<TransportJob>;
    friend class OutgoingDiagConnectionManager;

    void setResponseQueue(TransportJobQueue& responseQueue) { _pendingResponses = &responseQueue; }

    TransportJobQueue* _pendingResponses;
    bool _processingResponse;
    bool _connectionTerminationIsPending;
};

} // namespace uds
