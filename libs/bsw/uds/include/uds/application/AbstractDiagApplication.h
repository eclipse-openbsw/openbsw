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

#include <etl/span.h>

#include <cstdint>

#include "uds/connection/IOutgoingDiagConnectionProvider.h"

namespace transport
{
class TransportMessage;
}

namespace uds
{
class OutgoingDiagConnection;

/**
 * Base class for applications that send diag requests and receive responses
 */
class AbstractDiagApplication
{
public:
    enum RequestSendResult
    {
        REQUEST_SENT,
        REQUEST_SEND_FAILED
    };

    static uint32_t const DEFAULT_RESPONSE_TIMEOUT = 3000U; // ms

    explicit AbstractDiagApplication(IOutgoingDiagConnectionProvider& connectionProvider);

    /**
     * Called when a response for an outgoing request arrives.
     */
    virtual void responseReceived(
        OutgoingDiagConnection& connection,
        uint8_t sourceDiagAddress,
        ::etl::span<uint8_t const> response)
        = 0;

    /**
     * Called when the response timeout for an outgoing request expires.
     */
    virtual void responseTimeout(OutgoingDiagConnection& connection) = 0;

    /**
     * Called after the transport layer reports the send result for the request.
     */
    virtual void requestSent(OutgoingDiagConnection& connection, RequestSendResult result) = 0;

protected:
    IOutgoingDiagConnectionProvider::ErrorCode getOutgoingDiagConnection(
        uint16_t const targetAddress,
        OutgoingDiagConnection*& connection,
        transport::TransportMessage* const pRequestMessage = nullptr)
    {
        return _connectionProvider.getOutgoingDiagConnection(
            targetAddress, connection, pRequestMessage);
    }

private:
    IOutgoingDiagConnectionProvider& _connectionProvider;
};

} // namespace uds
