// Copyright 2024 Accenture.

#include "uds/DiagDispatcher.h"

#include "platform/config.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportJob.h"
#include "uds/DiagCodes.h"
#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/delegate.h>
#include <etl/queue.h>

DECLARE_LOGGER_COMPONENT(GLOBAL)

namespace uds
{
using ::transport::AbstractTransportLayer;
using ::transport::ITransportMessageListener;
using ::transport::ITransportMessageProvider;
using ::transport::ITransportMessageProvidingListener;
using ::transport::TransportConfiguration;
using ::transport::TransportJob;
using ::transport::TransportMessage;

using ::util::logger::GLOBAL;
using ::util::logger::Logger;
using ::util::logger::UDS;

AbstractTransportLayer::ErrorCode enqueueMessage(
    ::etl::iqueue<TransportJob>& sendJobQueue,
    TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const pNotificationListener,
    ::transport::ITransportMessageProcessedListener& defaultProcessedListener)
{
    if (!transportMessage.isComplete())
    {
        return AbstractTransportLayer::ErrorCode::TP_MESSAGE_INCOMPLETE;
    }

    ::async::ModifiableLockType lock;
    if (!sendJobQueue.full())
    {
        sendJobQueue.emplace();
        TransportJob& sendJob = sendJobQueue.back();
        lock.unlock();
        sendJob.setTransportMessage(transportMessage);
        if (pNotificationListener != nullptr)
        {
            sendJob.setProcessedListener(pNotificationListener);
        }
        else
        {
            sendJob.setProcessedListener(&defaultProcessedListener);
        }
        return AbstractTransportLayer::ErrorCode::TP_OK;
    }

    Logger::warn(UDS, "SendJobQueue full.");
    return AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL;
}

DiagDispatcher::DiagDispatcher(
    ::etl::ipool& incomingDiagConnectionPool,
    ::etl::iqueue<transport::TransportJob>& sendJobQueue,
    DiagnosisConfiguration& configuration,
    IDiagSessionManager& sessionManager,
    DiagJobRoot& jobRoot)
: IDiagDispatcher(sessionManager)
, AbstractTransportLayer(configuration.DiagBusId)
, incomingDiagConnectionPool(incomingDiagConnectionPool)
, sendJobQueue(sendJobQueue)
, fConfiguration(configuration)
, fConnectionShutdownDelegate()
, fConnectionShutdownRequested(false)
, fShutdownDelegate()
, fDefaultTransportMessageProcessedListener()
, fBusyMessage()
, fBusyMessageBuffer()
, fAsyncProcessQueue(
      ::async::Function::CallType::create<DiagDispatcher, &DiagDispatcher::processQueue>(*this))
, fDiagJobRoot(jobRoot)
{
    fBusyMessage.init(
        &fBusyMessageBuffer[0], BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES);
    fBusyMessage.resetValidBytes();
    (void)fBusyMessage.append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)fBusyMessage.append(0x00U);
    (void)fBusyMessage.append(static_cast<uint8_t>(DiagReturnCode::ISO_BUSY_REPEAT_REQUEST));
    fBusyMessage.setPayloadLength(BUSY_MESSAGE_LENGTH);
}

ESR_NO_INLINE AbstractTransportLayer::ErrorCode DiagDispatcher::send(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (!fEnabled)
    {
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }

    // FIXME: This reinterpret_cast works for now but is somewhat fragile
    auto connection = etl::find_if(
        incomingDiagConnectionPool.begin(),
        incomingDiagConnectionPool.end(),
        [pNotificationListener](void* const conn) -> bool
        { return reinterpret_cast<void*>(pNotificationListener) == conn; });

    if (connection != incomingDiagConnectionPool.end())
    {
        ITransportMessageListener::ReceiveResult const status
            = fProvidingListenerHelper.messageReceived(
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

    auto const result = enqueueMessage(
        sendJobQueue,
        transportMessage,
        pNotificationListener,
        fDefaultTransportMessageProcessedListener);

    if (AbstractTransportLayer::ErrorCode::TP_OK == result)
    {
        ::async::execute(fConfiguration.Context, fAsyncProcessQueue);
    }
    return result;
}

AbstractTransportLayer::ErrorCode DiagDispatcher::resume(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (!fEnabled)
    {
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }
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
        transportMessage.setTargetAddress(TransportMessage::INVALID_ADDRESS);
    }

    auto const result = enqueueMessage(
        sendJobQueue,
        transportMessage,
        pNotificationListener,
        fDefaultTransportMessageProcessedListener);
    if (AbstractTransportLayer::ErrorCode::TP_OK == result)
    {
        ::async::execute(fConfiguration.Context, fAsyncProcessQueue);
    }
    return result;
}

TransportMessage* DiagDispatcher::copyFunctionalRequest(
    TransportMessage& request,
    ITransportMessageProvidingListener& providingListener,
    DiagnosisConfiguration& configuration)
{
    TransportMessage* pRequest                        = nullptr;
    ITransportMessageProvider::ErrorCode const result = providingListener.getTransportMessage(
        configuration.DiagBusId,
        request.sourceAddress(),
        configuration.DiagAddress,
        TransportConfiguration::DIAG_PAYLOAD_SIZE,
        {},
        pRequest);
    if ((ITransportMessageProvider::ErrorCode::TPMSG_OK == result) && (nullptr != pRequest))
    {
        pRequest->resetValidBytes();
        pRequest->setSourceAddress(request.getSourceId());
        pRequest->setTargetAddress(request.getTargetId());
        pRequest->setPayloadLength(request.getPayloadLength());

        (void)pRequest->append(request.getPayload(), request.getPayloadLength());
    }
    else
    {
        pRequest = nullptr;
    }
    return pRequest;
}

void DiagDispatcher::processQueue()
{
    {
        ::async::ModifiableLockType lock;
        while (!sendJobQueue.empty())
        {
            TransportJob* const pSendJob = &sendJobQueue.front();
            sendJobQueue.pop();
            lock.unlock();
            dispatchIncomingRequest(
                *pSendJob,
                fConfiguration,
                *this,
                fDiagJobRoot,
                fProvidingListenerHelper,
                this,
                SendBusyResponseCallback::create<DiagDispatcher, &DiagDispatcher::sendBusyResponse>(
                    *this));
            lock.lock();
        }
    }
}

// METRIC STCYC 11 // The function is already in use as is
void DiagDispatcher::dispatchIncomingRequest(
    TransportJob& job,
    DiagnosisConfiguration& configuration,
    DiagDispatcher& dispatcher,
    DiagJobRoot& diagJobRoot,
    ITransportMessageProvidingListener& providingListener,
    ITransportMessageProcessedListener* const dispatcherProcessedListener,
    SendBusyResponseCallback sendBusyResponse)
{
    bool const isResuming
        = job.getTransportMessage()->getTargetId() == TransportMessage::INVALID_ADDRESS;
    if (isResuming)
    {
        job.getTransportMessage()->setTargetAddress(configuration.DiagAddress);
    }
    if (!configuration.AcceptAllRequests)
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
    if (configuration.CopyFunctionalRequests
        && TransportConfiguration::isFunctionallyAddressed(transportMessage))
    {
        pRequest = copyFunctionalRequest(transportMessage, providingListener, configuration);
        if (pRequest != nullptr)
        {
            if (job.getProcessedListener() != nullptr)
            {
                job.getProcessedListener()->transportMessageProcessed(
                    transportMessage,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
            }
            if (dispatcherProcessedListener != nullptr)
            {
                job.setProcessedListener(dispatcherProcessedListener);
            }
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
            = dispatcher.requestIncomingConnection(*job.getTransportMessage());
        if (pConnection != nullptr)
        {
            Logger::debug(
                UDS,
                "Opening incoming connection 0x%x --> 0x%x, service 0x%x",
                pConnection->sourceAddress,
                pConnection->targetAddress,
                pConnection->serviceId);
            pConnection->requestNotificationListener = job.getProcessedListener();
            DiagReturnCode::Type const result        = diagJobRoot.execute(
                *pConnection,
                job.getTransportMessage()->getPayload(),
                job.getTransportMessage()->getPayloadLength());
            if (result != DiagReturnCode::OK)
            {
                (void)pConnection->sendNegativeResponse(static_cast<uint8_t>(result), diagJobRoot);
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
        if (sendBusyResponse.is_valid())
        {
            sendBusyResponse(pMessage);
        }
        job.getProcessedListener()->transportMessageProcessed(
            *pMessage, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }
}

IncomingDiagConnection* DiagDispatcher::requestIncomingConnection(TransportMessage& requestMessage)
{
    if (fConnectionShutdownRequested)
    {
        return nullptr;
    }
    IncomingDiagConnection* pConnection = nullptr;
    {
        ::async::LockType const lock;
        pConnection
            = acquireIncomingDiagConnection(incomingDiagConnectionPool, fConfiguration.Context);
    }
    if (pConnection != nullptr)
    {
        auto const targetAddress = requestMessage.getTargetId();

        pConnection->diagDispatcher     = this;
        pConnection->messageSender      = this;
        pConnection->diagSessionManager = &fSessionManager;
        pConnection->sourceAddress      = requestMessage.getSourceId();
        pConnection->targetAddress      = targetAddress;
        pConnection->responseSourceAddress
            = (TransportConfiguration::isFunctionalAddress(targetAddress))
                  ? fConfiguration.DiagAddress
                  : targetAddress;
        pConnection->serviceId = requestMessage.getServiceId();
        pConnection->open(fConfiguration.ActivateOutgoingPending);
        pConnection->requestMessage  = &requestMessage;
        pConnection->responseMessage = nullptr;
        return pConnection;
    }
    Logger::warn(
        UDS,
        "No incoming diag connection available for 0x%x --> 0x%x, service 0x%x",
        requestMessage.getSourceId(),
        requestMessage.getTargetId(),
        requestMessage.getServiceId());
    return nullptr;
}

void DiagDispatcher::diagConnectionTerminated(IncomingDiagConnection& diagConnection)
{
    transport::TransportMessage* const requestMessage = diagConnection.requestMessage;
    transport::ITransportMessageProcessedListener* const notificationListener
        = diagConnection.requestNotificationListener;
    if ((notificationListener != nullptr) && (requestMessage != nullptr))
    {
        requestMessage->resetValidBytes();
        (void)requestMessage->increaseValidBytes(requestMessage->getPayloadLength());
        notificationListener->transportMessageProcessed(
            *requestMessage,
            transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }

    transport::TransportMessage* const responseMessage = diagConnection.responseMessage;
    if (responseMessage != nullptr)
    {
        fProvidingListenerHelper.releaseTransportMessage(*responseMessage);
    }

    {
        ::async::LockType const lock;
        incomingDiagConnectionPool.destroy(&diagConnection);
    }

    diagConnection.requestMessage              = nullptr;
    diagConnection.responseMessage             = nullptr;
    diagConnection.diagDispatcher              = nullptr;
    diagConnection.messageSender               = nullptr;
    diagConnection.requestNotificationListener = nullptr;

    checkConnectionShutdownProgress();
}

void DiagDispatcher::shutdownIncomingConnections(::etl::delegate<void()> delegate)
{
    fConnectionShutdownDelegate  = delegate;
    fConnectionShutdownRequested = true;
    checkConnectionShutdownProgress();
}

void DiagDispatcher::checkConnectionShutdownProgress()
{
    if (!fConnectionShutdownRequested)
    {
        return;
    }

    ::etl::ipool const& incomingDiagConnections = incomingDiagConnectionPool;

    if (!incomingDiagConnections.empty())
    {
        Logger::error(
            UDS,
            "DiagDispatcher::problem at shutdown(in: %d/%d)",
            incomingDiagConnections.size(),
            incomingDiagConnections.max_size());
        incomingDiagConnectionPool.release_all();
    }
    Logger::debug(UDS, "DiagDispatcher incoming connection shutdown complete");
    fConnectionShutdownDelegate();
}

void DiagDispatcher::sendBusyResponse(TransportMessage const* const message)
{
    Logger::error(UDS, "No incoming connection available -> request discarded --> BUSY");

    fBusyMessage.setSourceAddress(fConfiguration.DiagAddress);
    fBusyMessage.setTargetAddress(message->getSourceId());
    fBusyMessage.getPayload()[1] = message->getServiceId();

    ITransportMessageListener::ReceiveResult const status
        = fProvidingListenerHelper.messageReceived(fConfiguration.DiagBusId, fBusyMessage, nullptr);

    if (status != ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
    {
        Logger::error(UDS, "Could not send BUSY_REPEAT_REQUEST!");
    }
}

bool DiagDispatcher::isNegativeResponse(TransportMessage const& transportMessage)
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

bool DiagDispatcher::isFromValidSender(TransportMessage const& transportMessage)
{
    return TransportConfiguration::isFromTester(transportMessage);
}

AbstractTransportLayer::ErrorCode DiagDispatcher::init()
{
    fConnectionShutdownDelegate  = ::etl::delegate<void()>();
    fConnectionShutdownRequested = false;
    fEnabled                     = true;
    return AbstractTransportLayer::ErrorCode::TP_OK;
}

ESR_NO_INLINE bool DiagDispatcher::shutdown(ShutdownDelegate const delegate)
{
    Logger::debug(UDS, "DiagDispatcher::shutdown()");
    fEnabled          = false;
    fShutdownDelegate = delegate;
    shutdownIncomingConnections(
        ::etl::delegate<void()>::
            create<DiagDispatcher, &DiagDispatcher::connectionManagerShutdownComplete>(*this));
    return false;
}

void DiagDispatcher::connectionManagerShutdownComplete()
{
    Logger::debug(UDS, "DiagDispatcher::connectionManagerShutdownComplete()");
    fShutdownDelegate(*this);
}

void DiagDispatcher::transportMessageProcessed(
    TransportMessage& transportMessage, ProcessingResult const /* result */)
{
    fProvidingListenerHelper.releaseTransportMessage(transportMessage);
}

} // namespace uds
