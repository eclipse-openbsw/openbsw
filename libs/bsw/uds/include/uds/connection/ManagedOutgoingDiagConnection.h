// Copyright 2024 Accenture.

#pragma once

#include "estd/forward_list.h"
#include "estd/queue.h"
#include "transport/TransportJob.h"
#include "uds/connection/OutgoingDiagConnection.h"

namespace transport
{
class TransportMessage;
}

namespace uds
{
class AbstractDiagnosisConfiguration;

/**
 * A ManagedOutgoingDiagConnection is used by DiagConnectionManager.
 *
 *
 *
 *
 * This class exposes management functions that allow the DiagConnectionManager
 * to set parameters of the connection that are read-only for a
 * AbstractDiagApplication which is the reason why the reduced interface of
 * OutgoingDiagConnection is used there.
 *
 * \see OutgoingDiagConnection
 * \see DiagConnectionManager
 * \see AbstractDiagApplication
 */
class ManagedOutgoingDiagConnection
: public ::estd::forward_list_node<ManagedOutgoingDiagConnection>
, public OutgoingDiagConnection
{
public:
    ManagedOutgoingDiagConnection();

    void open()
    {
        fOpen                     = true;
        fSuppressIncomingPendings = true;
    }

    /**
     * Sets the source id of this ManagedOutgoingDiagConnection.
     * \param sourceId    Source diag address of this connection.
     */
    void setSourceId(uint16_t sourceId);

    /**
     * Sets the target id of this ManagedOutgoingDiagConnection.
     * \param targetId    Source diag address of this connection.
     */
    void setTargetId(uint16_t targetId);

    /**
     * Checks if a given TransportMessage is a response expected by this
     * connection, i.e. a response to a previously sent request.
     * \param transportMessage    TransportMessage to examine
     * \return *          - 0: Response is not expected
     *          - Otherwise number of bytes that match the request
     */
    uint16_t isExpectedResponse(transport::TransportMessage const& transportMessage);

    /**
     * Notifies the connection that a response has been received.
     * \param transportMessage    TransportMessage containing the response
     * \note * This method does not doublecheck if transportMessage is an expected
     * response of this connection, so call isExpectedResponse before!
     */
    void responseReceived(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener);

    void processResponseQueue();

    void terminate() override;

    void responseProcessed() override;

    void timeoutOccured() override;

private:
    using TransportJobQueue = ::estd::queue<transport::TransportJob>;
    friend class AbstractDiagnosisConfiguration;

    void setResponseQueue(TransportJobQueue& responseQueue) { fpPendingResponses = &responseQueue; }

    TransportJobQueue* fpPendingResponses;
    bool fProcessingResponse;
    bool fConnectionTerminationIsPending;
};

} // namespace uds
