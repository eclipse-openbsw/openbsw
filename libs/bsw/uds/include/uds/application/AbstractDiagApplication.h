// Copyright 2024 Accenture.

#ifndef GUARD_44D6432D_A8F2_4655_AD09_E114DD2F3080
#define GUARD_44D6432D_A8F2_4655_AD09_E114DD2F3080

#include "platform/estdint.h"
#include "uds/connection/IOutgoingDiagConnectionProvider.h"

namespace uds
{
class OutgoingDiagConnection;

/**
 * Base class for applications that send diag requests and receive responses
 *
 *
 */
class AbstractDiagApplication
{
public:
    /**
     * Indicator for result of asynchronous send result
     * @enum    RequestSendResult
     */
    enum RequestSendResult
    {
        /** The request has been successfully transmitted */
        REQUEST_SENT,
        /** An error occurred sending the request */
        REQUEST_SEND_FAILED
    };

    static uint32_t const DEFAULT_RESPONSE_TIMEOUT = 3000U; // ms

    /**
     * \param connectionProvider  IOutgoingDiagConnectionProvider that handles
     *                            requests of this application for OutgoingDiagConnections
     */
    explicit AbstractDiagApplication(IOutgoingDiagConnectionProvider& connectionProvider);

    /**
     * Callback that gets invoked when a response to a request that has been
     * sent through a OutgoingDiagConnections arrives.
     * \param connection        OutgoingDiagConnection to whose request the response
     *                          belongs to
     * \param sourceDiagAddress Diagnosisaddress of the responses sender
     * \param response          Array containing the response
     * \param responseLength    Length of response
     *
     * \note * The response stays valid until responseProcessed() is called upon the
     * OutgoingDiagConnection. Also no further responses will be received
     * until responseProcessed() is called. Incoming responses (e.g. to a
     * functional request) are queued inside the diagnosis stack.
     *
     * \note * sourceDiagAddress may differ from connection.getTargetId(), e.g. in case
     * of a functional request and its responses.
     *
     * \see OutgoingDiagConnection::responseProcessed()
     */
    virtual void responseReceived(
        OutgoingDiagConnection& connection,
        uint8_t sourceDiagAddress,
        uint8_t const response[],
        uint16_t responseLength)
        = 0;

    /**
     * Callback that gets invoked when the timeout specified on sending the
     * request has expired
     * \param connection  OutgoingDiagConnection whose timeout expired
     *
     * \note * This is the last chance to request a new timeout on the connection. If
     * this is not done, no more responses will be received when returning
     * from this callback!
     *
     * \see OutgoingDiagConnection::requestTimeout()
     */
    virtual void responseTimeout(OutgoingDiagConnection& connection) = 0;

    /**
     * Callback that gets invoked when the OutgoingDiagConnection on which
     * a request has been sent receives the result from its asynchronous
     * sender
     * \param connection  OutgoingDiagConnection on which a request has been
     * sent
     * \param result  REQUEST_SENT if the request has been successfully
     * sent, REQUEST_SEND_FAILED otherwise
     */
    virtual void requestSent(OutgoingDiagConnection& connection, RequestSendResult result) = 0;

protected:
    IOutgoingDiagConnectionProvider::ErrorCode getOutgoingDiagConnection(
        uint16_t const targetId,
        OutgoingDiagConnection*& connection,
        transport::TransportMessage* const pRequestMessage = nullptr)
    {
        return fConnectionProvider.getOutgoingDiagConnection(targetId, connection, pRequestMessage);
    }

private:
    IOutgoingDiagConnectionProvider& fConnectionProvider;
};

} // namespace uds

#endif // GUARD_44D6432D_A8F2_4655_AD09_E114DD2F3080
