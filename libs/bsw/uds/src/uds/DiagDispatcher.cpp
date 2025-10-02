// Copyright 2024 Accenture.

#include "uds/DiagDispatcher.h"

#include "platform/config.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportJob.h"
#include "uds/DiagCodes.h"
#include "uds/UdsLogger.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/delegate.h>

DECLARE_LOGGER_COMPONENT(GLOBAL)

namespace uds
{
using ::transport::AbstractTransportLayer;
using ::transport::ITransportMessageListener;
using ::transport::ITransportMessageProvider;
using ::transport::TransportConfiguration;
using ::transport::TransportJob;
using ::transport::TransportMessage;

using ::util::logger::GLOBAL;
using ::util::logger::Logger;
using ::util::logger::UDS;

DiagDispatcher2::DiagDispatcher2(
    AbstractDiagnosisConfiguration& configuration,
    IDiagSessionManager& sessionManager,
    DiagJobRoot& jobRoot,
    ::async::ContextType context)
: IResumableDiagDispatcher(sessionManager, jobRoot)
, AbstractTransportLayer(configuration.DiagBusId)
, fConfiguration(configuration)
, fConnectionManager(fConfiguration, *this, getProvidingListenerHelper(), context, *this)
, fShutdownDelegate()
, fDefaultTransportMessageProcessedListener()
, fBusyMessage()
, fBusyMessageBuffer()
, fAsyncProcessQueue(
      ::async::Function::CallType::create<DiagDispatcher2, &DiagDispatcher2::processQueue>(*this))
{
    fBusyMessage.init(
        &fBusyMessageBuffer[0], BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES);
    fBusyMessage.resetValidBytes();
    (void)fBusyMessage.append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)fBusyMessage.append(0x00U);
    (void)fBusyMessage.append(static_cast<uint8_t>(DiagReturnCode::ISO_BUSY_REPEAT_REQUEST));
    fBusyMessage.setPayloadLength(BUSY_MESSAGE_LENGTH);
}

ESR_NO_INLINE AbstractTransportLayer::ErrorCode DiagDispatcher2::send_local(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    for (DiagConnectionManager::ManagedOutgoingDiagConnectionList::iterator itr
         = fConnectionManager.getReleasedConnections().begin();
         itr != fConnectionManager.getReleasedConnections().end();
         ++itr)
    { // is this is a request sent by one of our outgoing connections?
        ITransportMessageProcessedListener* const pListener = itr.operator->();
        if (pListener == pNotificationListener)
        {
            ITransportMessageListener::ReceiveResult const status
                = getProvidingListenerHelper().messageReceived(
                    fConfiguration.DiagBusId, transportMessage, pNotificationListener);
            if (status == ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
            {
                return AbstractTransportLayer::ErrorCode::TP_OK;
            }
            else
            {
                return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
            }
        }
    }
    auto connection = fConfiguration.findIncomingDiagConnection(
        [pNotificationListener](void* const conn) -> bool
        { return reinterpret_cast<void*>(pNotificationListener) == conn; });
    if (connection != nullptr)
    {
        ITransportMessageListener::ReceiveResult const status
            = getProvidingListenerHelper().messageReceived(
                fConfiguration.DiagBusId, transportMessage, pNotificationListener);
        if (status == ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
        {
            return AbstractTransportLayer::ErrorCode::TP_OK;
        }
        else
        {
            return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
        }
    }
    if ((transportMessage.getTargetId() != fConfiguration.DiagAddress)
        && ((fConfiguration.BroadcastAddress != TransportMessage::INVALID_ADDRESS)
            && (transportMessage.getTargetId() != fConfiguration.BroadcastAddress)))
    {
        Logger::error(
            UDS,
            "DiagDispatcher::send(): invalid target 0x%x, expected 0x%x",
            transportMessage.getTargetId(),
            fConfiguration.DiagAddress);
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }

    return enqueueMessage(transportMessage, pNotificationListener);
}

AbstractTransportLayer::ErrorCode DiagDispatcher2::send(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    return send_local(transportMessage, pNotificationListener);
}

AbstractTransportLayer::ErrorCode DiagDispatcher2::resume(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if ((transportMessage.getTargetId() != fConfiguration.DiagAddress)
        && ((fConfiguration.BroadcastAddress == TransportMessage::INVALID_ADDRESS)
            || (transportMessage.getTargetId() != fConfiguration.BroadcastAddress)))
    {
        Logger::error(
            UDS,
            "DiagDispatcher::resume(): invalid target 0x%x, expected 0x%x",
            transportMessage.getTargetId(),
            fConfiguration.DiagAddress);
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }
    else
    {
        transportMessage.setTargetId(TransportMessage::INVALID_ADDRESS);
    }

    return enqueueMessage(transportMessage, pNotificationListener);
}

TransportMessage* DiagDispatcher2::copyFunctionalRequest(TransportMessage& request)
{
    TransportMessage* pRequest = nullptr;
    ITransportMessageProvider::ErrorCode const result
        = getProvidingListenerHelper().getTransportMessage(
            getBusId(),
            request.sourceAddress(),
            fConfiguration.DiagAddress,
            TransportConfiguration::DIAG_PAYLOAD_SIZE,
            {},
            pRequest);
    if ((ITransportMessageProvider::ErrorCode::TPMSG_OK == result) && (nullptr != pRequest))
    {
        pRequest->resetValidBytes();
        pRequest->setSourceId(request.getSourceId());
        pRequest->setTargetId(request.getTargetId());
        pRequest->setPayloadLength(request.getPayloadLength());

        (void)pRequest->append(request.getPayload(), request.getPayloadLength());
    }
    else
    {
        pRequest = nullptr;
    }
    return pRequest;
}

AbstractTransportLayer::ErrorCode DiagDispatcher2::enqueueMessage(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (!isEnabled())
    {
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }

    if (!transportMessage.isComplete())
    {
        return AbstractTransportLayer::ErrorCode::TP_MESSAGE_INCOMPLETE;
    }
    ::async::ModifiableLockType lock;
    if (!fConfiguration.SendJobQueue.full())
    {
        fConfiguration.SendJobQueue.emplace();
        TransportJob& sendJob = fConfiguration.SendJobQueue.back();
        lock.unlock();
        sendJob.setTransportMessage(transportMessage);
        if (pNotificationListener != nullptr)
        {
            sendJob.setProcessedListener(pNotificationListener);
        }
        else
        {
            sendJob.setProcessedListener(&fDefaultTransportMessageProcessedListener);
        }
        trigger();
        return AbstractTransportLayer::ErrorCode::TP_OK;
    }
    else
    {
        Logger::warn(UDS, "SendJobQueue full.");
        return AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL;
    }
}

void DiagDispatcher2::processQueue()
{
    {
        ::async::ModifiableLockType lock;
        while (!fConfiguration.SendJobQueue.empty())
        {
            TransportJob* const pSendJob = &fConfiguration.SendJobQueue.front();
            fConfiguration.SendJobQueue.pop();
            lock.unlock();
            TransportMessage* const pTransportMessage = pSendJob->getTransportMessage();

            ManagedOutgoingDiagConnection* const pOutgoingConnection
                = fConnectionManager.getExpectingConnection(*pTransportMessage);
            if (pOutgoingConnection != nullptr)
            {
                pOutgoingConnection->responseReceived(
                    *pTransportMessage, pSendJob->getProcessedListener());
            }
            else
            {
                dispatchIncomingRequest(*pSendJob);
            }
            lock.lock();
        }
    }
    fConnectionManager.processPendingResponses();
}

uint8_t DiagDispatcher2::dispatchTriggerEventRequest(TransportMessage& tmsg)
{
    if ((fConfiguration.SendJobQueue.empty()) && (isEnabled()))
    {
        /* check for TransportConfiguration::isFunctionallyAddressed
         * is obsolete because ALL triggerEvent are of this type  -
         * so exchange transport message against buffer and copy */
        TransportMessage* const pRequest = copyFunctionalRequest(tmsg);
        if (pRequest != nullptr)
        {
            IncomingDiagConnection* const pConnection
                = fConnectionManager.requestIncomingConnection(*pRequest);
            if (pConnection != nullptr)
            {
                Logger::debug(
                    UDS,
                    "Opening triggered connection 0x%x --> 0x%x, service 0x%x",
                    pConnection->fSourceId,
                    pConnection->fTargetId,
                    pConnection->fServiceId);
                // pConnection->setRequestNotificationListener(*job.getProcessedListener());
                DiagReturnCode::Type const result = fDiagJobRoot.execute(
                    *pConnection, pRequest->getPayload(), pRequest->getPayloadLength());
                if (result != DiagReturnCode::OK)
                {
                    (void)pConnection->sendNegativeResponse(
                        static_cast<uint8_t>(result), fDiagJobRoot);
                    pConnection->terminate();
                }
                /* release the transport message immediately */
                transportMessageProcessed(
                    *pRequest,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
                return 0U;
            }
            else
            {
                /* release the transport message immediately */
                transportMessageProcessed(
                    *pRequest,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
                return 1U;
            }
        }
        else
        {
            /* enable for debugging */
            sendBusyResponse(&tmsg);
        }
    }
    return 1U;
}

// METRIC STCYC 11 // The function is already in use as is
void DiagDispatcher2::dispatchIncomingRequest(TransportJob& job)
{
    bool const isResuming
        = job.getTransportMessage()->getTargetId() == TransportMessage::INVALID_ADDRESS;
    if (isResuming)
    {
        job.getTransportMessage()->setTargetId(fConfiguration.DiagAddress);
    }
    if (!fConfiguration.AcceptAllRequests)
    { // check if source is a tester or functional request
        if (!isFromValidSender(*job.getTransportMessage()))
        {
            Logger::warn(
                UDS,
                "Request from invalid source 0x%x discarded",
                job.getTransportMessage()->getSourceId());
            job.getProcessedListener()->transportMessageProcessed(
                *job.getTransportMessage(),
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
            return;
        }
    }
    bool sendBusyNegativeResponse      = false;
    TransportMessage& transportMessage = *job.getTransportMessage();
    TransportMessage* pRequest         = &transportMessage;
    if (fConfiguration.CopyFunctionalRequests
        && TransportConfiguration::isFunctionallyAddressed(transportMessage))
    {
        pRequest = copyFunctionalRequest(transportMessage);
        if (pRequest != nullptr)
        {
            if (job.getProcessedListener() != nullptr)
            {
                job.getProcessedListener()->transportMessageProcessed(
                    transportMessage,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
            }
            job.setProcessedListener(this);
            job.setTransportMessage(*pRequest);
        }
        else
        {
            Logger::critical(GLOBAL, "!!!! Busy because no functional buffer!");
            sendBusyNegativeResponse = true;
        }
    }

    if (!sendBusyNegativeResponse)
    {
        IncomingDiagConnection* const pConnection
            = fConnectionManager.requestIncomingConnection(*job.getTransportMessage());
        if (pConnection != nullptr)
        {
            Logger::debug(
                UDS,
                "Opening incoming connection 0x%x --> 0x%x, service 0x%x",
                pConnection->fSourceId,
                pConnection->fTargetId,
                pConnection->fServiceId);
            pConnection->setRequestNotificationListener(*job.getProcessedListener());
            DiagReturnCode::Type const result = fDiagJobRoot.execute(
                *pConnection,
                job.getTransportMessage()->getPayload(),
                job.getTransportMessage()->getPayloadLength());
            if (result != DiagReturnCode::OK)
            {
                (void)pConnection->sendNegativeResponse(static_cast<uint8_t>(result), fDiagJobRoot);
                pConnection->terminate();
            }
        }
        else
        {
            Logger::critical(GLOBAL, "!!!! Busy because no incoming connection!");
            sendBusyNegativeResponse = true;
        }
    }
    if (sendBusyNegativeResponse)
    {
        TransportMessage* const pMessage = job.getTransportMessage();
        sendBusyResponse(pMessage);
        job.getProcessedListener()->transportMessageProcessed(
            *pMessage, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }
}

void DiagDispatcher2::sendBusyResponse(TransportMessage const* const message)
{
    Logger::error(UDS, "No incoming connection available -> request discarded --> BUSY");

    fBusyMessage.setSourceId(fConfiguration.DiagAddress);
    fBusyMessage.setTargetId(message->getSourceId());
    fBusyMessage.getPayload()[1] = message->getServiceId();

    ITransportMessageListener::ReceiveResult const status
        = getProvidingListenerHelper().messageReceived(
            fConfiguration.DiagBusId, fBusyMessage, nullptr);

    if (status != ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
    {
        Logger::error(UDS, "Could not send BUSY_REPEAT_REQUEST!");
    }
}

bool DiagDispatcher2::isNegativeResponse(TransportMessage const& transportMessage)
{
    if (transportMessage.getPayloadLength() < 1U)
    {
        return false;
    }
    else
    {
        return (transportMessage.getServiceId() == DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    }
}

bool DiagDispatcher2::isFromValidSender(TransportMessage const& transportMessage)
{
    return TransportConfiguration::isFromTester(transportMessage);
}

IOutgoingDiagConnectionProvider::ErrorCode DiagDispatcher2::getOutgoingDiagConnection(
    uint16_t const targetId,
    OutgoingDiagConnection*& pConnection,
    TransportMessage* const pRequestMessage)
{
    if (!fEnabled)
    {
        return IOutgoingDiagConnectionProvider::NO_CONNECTION_AVAILABLE;
    }
    IOutgoingDiagConnectionProvider::ErrorCode const status
        = fConnectionManager.requestOutgoingConnection(targetId, pConnection, pRequestMessage);
    if (pConnection != nullptr)
    {
        Logger::debug(UDS, "Opened outgoing diag connection to target 0x%x", targetId);
    }
    return status;
}

void DiagDispatcher2::trigger() { ::async::execute(fConfiguration.Context, fAsyncProcessQueue); }

AbstractTransportLayer::ErrorCode DiagDispatcher2::init()
{
    fConnectionManager.init();
    enable();
    return AbstractTransportLayer::ErrorCode::TP_OK;
}

ESR_NO_INLINE bool DiagDispatcher2::shutdown_local(ShutdownDelegate const delegate)
{
    Logger::debug(UDS, "DiagDispatcher2::shutdown()");
    disable();
    fShutdownDelegate = delegate;
    fConnectionManager.shutdown(
        ::etl::delegate<void()>::
            create<DiagDispatcher2, &DiagDispatcher2::connectionManagerShutdownComplete>(*this));
    return false;
}

bool DiagDispatcher2::shutdown(ShutdownDelegate const delegate) { return shutdown_local(delegate); }

void DiagDispatcher2::connectionManagerShutdownComplete()
{
    Logger::debug(UDS, "DiagDispatcher2::connectionManagerShutdownComplete()");
    fShutdownDelegate(*this);
}

void DiagDispatcher2::transportMessageProcessed(
    TransportMessage& transportMessage, ProcessingResult const /* result */)
{
    getProvidingListenerHelper().releaseTransportMessage(transportMessage);
}

} // namespace uds
