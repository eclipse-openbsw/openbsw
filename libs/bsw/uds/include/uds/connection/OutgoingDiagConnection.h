// Copyright 2024 Accenture.

#pragma once

#include "transport/ITransportMessageProcessedListener.h"
#include "uds/application/AbstractDiagApplication.h"
#include "uds/connection/ErrorCode.h"

#include <async/Async.h>

namespace transport
{
class AbstractTransportLayer;
}

namespace uds
{
class DiagConnectionManager;

/**
 * Interface for AbstractDiagApplication to send a diagnosis request and receive
 * responses
 *
 */
class OutgoingDiagConnection : public transport::ITransportMessageProcessedListener
{
public:
    transport::TransportMessage* fpRequestMessage      = nullptr;
    transport::TransportMessage* fpResponseMessage     = nullptr;
    transport::AbstractTransportLayer* fpMessageSender = nullptr;
    DiagConnectionManager* fpDiagConnectionManager     = nullptr;
    ::async::ContextType fContext;
    bool fOpen = false;

    uint16_t fSourceId = static_cast<uint16_t>(0xFFU);
    uint16_t fTargetId = static_cast<uint16_t>(0XFFU);
    uint8_t fServiceId = 0xFFU;

public:
    static uint32_t const INFINITE_RESPONSE_TIMEOUT = 0xFFFFFFFFU;

    OutgoingDiagConnection() : fResponseTimeout(*this), fResponsePendingTimeout(*this) {}

    /**
     * Call this function once the current response has been processed
     *
     * \note * As an AbstractDiagApplication you will only receive one response at a
     * time. Further responses are stored in a queue and reported to the
     * AbstractDiagApplication one by one after it calls responseProcessed.
     */
    virtual void responseProcessed() = 0;

    virtual void terminate() = 0;

    /**
     * Returns the request buffer and its length
     * \param pBuffer Pointer which will refer to the request buffer
     * \param length  Variable that will hold the length of the request buffer
     */
    void getRequestBuffer(uint8_t*& pBuffer, uint16_t& length);

    /**
     * Sends a diagnosis request that is contained in the buffer that has been
     * requested via getRequestBuffer() before
     * \param length          Length of the payload to send
     * \param sender          Indicates who is sending this request
     * (usually "this")
     * \param timeout         Timeout to wait for responses
     * \param matchingResponseBytes   Number of bytes following the service
     * identifier that the connection considers in its verify() method when a
     * response arrives
     * \param suppressIncomingPendings    Flag to enable filtering of response
     * pending respones. They will then not be given to any listening application.
     * \return ErrorCode indication result of the send operation
     *          - OK: Successfully sent request
     *          - SEND_FAILED: An error occurred sending the request
     *
     * \see verify()
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
     * Resends the request that has been sent via sendDiagRequest() before.
     * This can be done after sending a request as long as the connection has not been closed.
     * \return ErrorCode indication result of the send operation
     *          - OK: Successfully sent request
     *          - SEND_FAILED: An error occurred sending the request
     */
    ErrorCode resend(uint32_t timeout = AbstractDiagApplication::DEFAULT_RESPONSE_TIMEOUT);

    void keepAlive() { fKeepAlive = true; }

    /**
     * Verifies how many bytes of a given response match the request that has
     * been sent by this connection
     * \param response  The response message
     * \return Number of bytes that match the request of this connection
     */
    uint16_t verifyResponse(transport::TransportMessage const& response);

    /**
     * Checks is a connection conflicts with this OutgoingDiagConnection.
     * \param connection  OutgoingDiagConnection to check for conflicts
     *
     * A conflict arises if both connections have the same serviceId and both
     * may receive negative responses because a negative response contains only
     * the serviceId and this must be used to dispatch a incoming response
     * to a distinct OutgoingDiagConnection (which is impossible with more than
     * one connection having the same serviceId).
     *
     * \see sendDiagRequest()
     */
    bool isConflicting(OutgoingDiagConnection const& connection);

    /**
     * \see transport::ITransportMessageSendNotificationListener()
     */
    void transportMessageProcessed(
        transport::TransportMessage& transportMessage, ProcessingResult result) override;

    void execute(::async::RunnableType const& timeout);

protected:
    static uint32_t const RESPONSE_PENDING_TIMEOUT = 5000U; // ms

    class Timeout : public ::async::RunnableType
    {
    public:
        explicit Timeout(OutgoingDiagConnection& connection) : fConnection(connection) {}

        void execute() override { fConnection.execute(*this); }

        ::async::TimeoutType _asyncTimeout;
        bool _isActive;

    private:
        OutgoingDiagConnection& fConnection;
    };

    /**
     * Implemented in ManageOutgoingDiagConnection
     * Notifies the ManageOutgoingDiagConnection of the connection timeout
     */
    virtual void timeoutOccured() = 0;

    /**
     * \return  CONNECTION_NOT_OPEN: Impossible to request timeout on a non
     *          busy connection.
     */
    ErrorCode setTimeout(uint32_t newTimeout);

    AbstractDiagApplication* fpSender = nullptr;
    Timeout fResponseTimeout;
    Timeout fResponsePendingTimeout;
    bool fKeepAlive                 = false;
    bool fSendInProgress            = false;
    bool fAcceptNegativeResponse    = true;
    bool fSuppressIncomingPendings  = false;
    bool fInfiniteTimeout           = false;
    uint16_t fMatchingResponseBytes = 0U;
    uint32_t fTimeout               = 0U;
};

} // namespace uds
