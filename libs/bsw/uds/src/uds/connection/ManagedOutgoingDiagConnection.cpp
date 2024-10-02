// Copyright 2024 Accenture.

#include "uds/connection/ManagedOutgoingDiagConnection.h"

#include "estd/assert.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "uds/DiagCodes.h"
#include "uds/DiagReturnCode.h"
#include "uds/UdsLogger.h"
#include "uds/application/AbstractDiagApplication.h"
#include "uds/connection/DiagConnectionManager.h"

#include <async/Async.h>

namespace uds
{
using ::transport::TransportConfiguration;
using ::transport::TransportJob;
using ::transport::TransportMessage;
using ::util::logger::Logger;
using ::util::logger::UDS;

ManagedOutgoingDiagConnection::ManagedOutgoingDiagConnection()
: OutgoingDiagConnection()
, fpPendingResponses(nullptr)
, fProcessingResponse(false)
, fConnectionTerminationIsPending(false)
{}

void ManagedOutgoingDiagConnection::setSourceId(uint16_t const sourceId)
{
    if (nullptr == fpRequestMessage)
    {
        Logger::critical(
            UDS,
            "ManagedOutgoingDiagConnection::setSourceId() called with fpRequestMessage == NULL!");
        estd_assert(fpRequestMessage);
    }
    fSourceId = sourceId;
    fpRequestMessage->setSourceId(sourceId);
}

void ManagedOutgoingDiagConnection::setTargetId(uint16_t const targetId)
{
    if (nullptr == fpRequestMessage)
    {
        Logger::critical(
            UDS,
            "ManagedOutgoingDiagConnection::setTargetId() called with fpRequestMessage == NULL!");
        estd_assert(fpRequestMessage);
    }
    fTargetId = targetId;
    fpRequestMessage->setTargetId(targetId);
}

uint16_t ManagedOutgoingDiagConnection::isExpectedResponse(TransportMessage const& transportMessage)
{
    if (!fOpen)
    {
        return 0U;
    }

    // check Sender
    if ((transportMessage.getSourceId() != fTargetId)
        && (fTargetId != DiagCodes::FUNCTIONAL_ID_ALL_ISO14229_ECUS)
        && (fTargetId != DiagCodes::FUNCTIONAL_ID_ALL_KWP2000_ECUS))
    {
        Logger::debug(
            UDS,
            "Incorrect Sender 0x%x, Target was 0x%x",
            transportMessage.getSourceId(),
            fTargetId);
        return 0U;
    }

    return verifyResponse(transportMessage);
}

void ManagedOutgoingDiagConnection::processResponseQueue()
{
    if (!fProcessingResponse)
    {
        if ((nullptr != fpPendingResponses) && (!fpPendingResponses->empty()))
        {
            if (fpSender != nullptr)
            {
                fpResponseMessage   = fpPendingResponses->front().getTransportMessage();
                fProcessingResponse = true;
                fpSender->responseReceived(
                    *this,
                    static_cast<uint8_t>(fpResponseMessage->getSourceId()),
                    fpResponseMessage->getPayload(),
                    fpResponseMessage->getPayloadLength());
            }
            else
            {
                Logger::error(
                    UDS,
                    "ManagedOutgoingDiagConnection::processResponseQueue(), fSender == NULL --> "
                    "discarding pending responses");
                while (!fpPendingResponses->empty())
                {
                    fpPendingResponses->pop();
                }
            }
        }
    }
}

void ManagedOutgoingDiagConnection::responseReceived(
    transport::TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (fpSender != nullptr)
    { // this connection is still active
        bool isResponsePending  = false;
        ErrorCode timeoutStatus = ::uds::ErrorCode::OK;
        if ((transportMessage.getPayload()[0] == DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER)
            && (transportMessage.getPayload()[2]
                == static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING)))
        {
            timeoutStatus     = setTimeout(RESPONSE_PENDING_TIMEOUT);
            isResponsePending = true;
        }
        else if (TransportConfiguration::isFunctionalAddress(fTargetId))
        {
            timeoutStatus = setTimeout(fTimeout);
        }
        else
        {
            // nothing to do
        }
        if (timeoutStatus != ::uds::ErrorCode::OK)
        {
            Logger::error(
                UDS,
                "ManagedOutgoingDiagConnection::responseReceived(0x%x --> 0x%x) unable to request "
                "timeout",
                fSourceId,
                fTargetId);
        }
        if (isResponsePending && fSuppressIncomingPendings)
        {
            if (pNotificationListener != nullptr)
            {
                pNotificationListener->transportMessageProcessed(
                    transportMessage,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
            }
        }
        else if ((nullptr != fpPendingResponses) && (!fpPendingResponses->full()))
        {
            TransportJob& response = fpPendingResponses->push();
            response.setTransportMessage(transportMessage);
            response.setProcessedListener(pNotificationListener);
            processResponseQueue();
        }
        else
        {
            Logger::warn(
                UDS,
                "ManagedOutgoingDiagConnection::responseReceived(0x%x --> 0x%x) response queue "
                "full!!",
                fSourceId,
                fTargetId);
            if (pNotificationListener != nullptr)
            {
                pNotificationListener->transportMessageProcessed(
                    transportMessage,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
            }
        }
    }
    else
    {
        Logger::error(
            UDS,
            "ManagedOutgoingDiagConnection::responseReceived(0x%x --> 0x%x) with fpSender == NULL!",
            fSourceId,
            fTargetId);
        if (pNotificationListener != nullptr)
        {
            pNotificationListener->transportMessageProcessed(
                transportMessage,
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
        }
    }
}

void ManagedOutgoingDiagConnection::terminate()
{
    ::async::ModifiableLockType lock;
    if (fOpen || fConnectionTerminationIsPending)
    {
        fOpen                           = false;
        fConnectionTerminationIsPending = false;
        lock.unlock();
        Logger::debug(
            UDS,
            "ManagedOutgoingDiagConnection::terminate(): 0x%x --> 0x%x, "
            "service 0x%x, %d pending responses",
            fSourceId,
            fTargetId,
            fServiceId,
            (nullptr != fpPendingResponses) ? fpPendingResponses->size() : 0U);
        fResponseTimeout._asyncTimeout.cancel();
        fResponsePendingTimeout._asyncTimeout.cancel();

        fSendInProgress  = false;
        fInfiniteTimeout = false;
        fpSender         = nullptr;
        if (fpDiagConnectionManager != nullptr)
        {
            fpDiagConnectionManager->diagConnectionTerminated(*this);
        }
        if (fpPendingResponses != nullptr)
        {
            // clean up pending responses
            while (!fpPendingResponses->empty())
            {
                TransportJob& response = fpPendingResponses->front();
                fpPendingResponses->pop();
                if (response.getProcessedListener() != nullptr)
                {
                    if (fProcessingResponse)
                    {
                        // terminate is implicit responseProcessed --> no error
                        response.getProcessedListener()->transportMessageProcessed(
                            *response.getTransportMessage(),
                            ITransportMessageProcessedListener::ProcessingResult::
                                PROCESSED_NO_ERROR);
                        fProcessingResponse = false;
                    }
                    else
                    {
                        response.getProcessedListener()->transportMessageProcessed(
                            *response.getTransportMessage(),
                            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
                    }
                }
            }
        }
        // just in case fPendingResponses was empty
        fProcessingResponse = false;
    }
    else
    {
        // nothing to do
    }
}

void ManagedOutgoingDiagConnection::responseProcessed()
{
    fProcessingResponse = false;
    if ((nullptr != fpPendingResponses) && (!fpPendingResponses->empty()))
    {
        TransportJob& response = fpPendingResponses->front();
        fpPendingResponses->pop();
        if (response.getProcessedListener() != nullptr)
        {
            response.getProcessedListener()->transportMessageProcessed(
                *response.getTransportMessage(),
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        }
        if (fpDiagConnectionManager != nullptr)
        {
            fpDiagConnectionManager->triggerResponseProcessing();
        }
    }
    else
    {
        Logger::error(
            UDS,
            "ManagedOutgoingDiagConnection::responseProcessed() called "
            "with no pending response!");
    }

    if (fConnectionTerminationIsPending && (nullptr != fpPendingResponses)
        && (fpPendingResponses->empty()))
    {
        terminate();
    }
}

void ManagedOutgoingDiagConnection::timeoutOccured()
{
    if ((nullptr != fpPendingResponses) && (fpPendingResponses->empty()))
    {
        terminate();
    }
    else
    {
        fOpen                           = false;
        fConnectionTerminationIsPending = true;
    }
}

} // namespace uds
