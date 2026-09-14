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

#include <async/Async.h>
#include <etl/span.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <uds/connection/ErrorCode.h>

#include "uds/application/AbstractDiagApplication.h"

namespace transport
{
class TransportMessage;
class AbstractTransportLayer;
} // namespace transport

namespace uds
{
class OutgoingDiagConnectionManager;

/**
 * Interface for AbstractDiagApplication to send a diagnosis request and receive
 * responses
 */
class OutgoingDiagConnection : public transport::ITransportMessageProcessedListener
{
public:
    transport::TransportMessage* _requestMessage          = nullptr;
    transport::TransportMessage* _responseMessage         = nullptr;
    transport::AbstractTransportLayer* _messageSender     = nullptr;
    OutgoingDiagConnectionManager* _diagConnectionManager = nullptr;
    ::async::ContextType _context;
    bool _open = false;

    uint16_t _sourceAddress = static_cast<uint16_t>(0xFFU);
    uint16_t _targetAddress = static_cast<uint16_t>(0xFFU);
    uint8_t _serviceId      = 0xFFU;

public:
    static uint32_t const INFINITE_RESPONSE_TIMEOUT = 0xFFFFFFFFU;

    OutgoingDiagConnection() : _responseTimeout(*this), _responsePendingTimeout(*this) {}

    /**
     * Called once the current response has been processed by the application.
     *
     * \note * As an AbstractDiagApplication you will only receive one response at a
     * time. Further responses are stored in a queue and reported to the
     * AbstractDiagApplication one by one after it calls responseProcessed.
     */
    virtual void responseProcessed() = 0;

    virtual void terminate() = 0;

    ::etl::span<uint8_t> getRequestBuffer();

    /**
     * Sends the request currently stored in the connection buffer.
     */
    ErrorCode sendDiagRequest(
        uint16_t length,
        AbstractDiagApplication& sender,
        uint32_t timeout               = AbstractDiagApplication::DEFAULT_RESPONSE_TIMEOUT,
        uint16_t matchingResponseBytes = 0U,
        bool acceptNegativeResponse    = true,
        bool suppressSend              = false,
        bool suppressIncomingPendings  = true);

    /**
     * Resends the last request while the connection remains open.
     */
    ErrorCode resend(uint32_t timeout = AbstractDiagApplication::DEFAULT_RESPONSE_TIMEOUT);

    void keepAlive() { _keepAlive = true; }

    /**
     * Returns the number of response bytes matching this request.
     */
    uint16_t verifyResponse(transport::TransportMessage const& response);

    /**
     * Checks if a connection conflicts with this OutgoingDiagConnection.
     *
     * A conflict arises if both connections have the same serviceId and both
     * may receive negative responses because a negative response contains only
     * the serviceId and this must be used to dispatch a incoming response
     * to a distinct OutgoingDiagConnection (which is impossible with more than
     * one connection having the same serviceId).
     */
    bool isConflicting(OutgoingDiagConnection const& connection);

    void transportMessageProcessed(
        transport::TransportMessage& transportMessage, ProcessingResult result) override;

    void execute(::async::RunnableType const& timeout);

protected:
    static uint32_t const RESPONSE_PENDING_TIMEOUT = 5000U; // ms

    class Timeout : public ::async::RunnableType
    {
    public:
        explicit Timeout(OutgoingDiagConnection& connection)
        : _isActive(false), _connection(connection)
        {}

        void execute() override { _connection.execute(*this); }

        ::async::TimeoutType _asyncTimeout;
        bool _isActive = false;

    private:
        OutgoingDiagConnection& _connection;
    };

    /**
     * Implemented in ManagedOutgoingDiagConnection
     * Notifies the ManagedOutgoingDiagConnection of the connection timeout
     */
    virtual void timeoutOccurred() = 0;

    /**
     * \return  CONNECTION_NOT_OPEN: Impossible to request timeout on a non
     *          busy connection.
     */
    ErrorCode setTimeout(uint32_t newTimeout);

    AbstractDiagApplication* _sender = nullptr;
    Timeout _responseTimeout;
    Timeout _responsePendingTimeout;
    bool _keepAlive                 = false;
    bool _sendInProgress            = false;
    bool _acceptNegativeResponse    = true;
    bool _suppressIncomingPendings  = false;
    bool _infiniteTimeout           = false;
    uint16_t _matchingResponseBytes = 0U;
    uint32_t _timeout               = 0U;
};

} // namespace uds
