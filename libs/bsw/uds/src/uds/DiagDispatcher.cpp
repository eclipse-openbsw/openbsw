// Copyright 2024 Accenture.

#include "uds/DiagDispatcher.h"

#include "platform/config.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/TransportConfiguration.h"
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
using ::transport::TransportConfiguration;
using ::transport::TransportMessage;

using ::util::logger::GLOBAL;
using ::util::logger::Logger;
using ::util::logger::UDS;

namespace
{

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
        sendJob.transportMessage = &transportMessage;
        if (pNotificationListener != nullptr)
        {
            sendJob.processedListener = pNotificationListener;
        }
        else
        {
            sendJob.processedListener = &defaultProcessedListener;
        }
        return AbstractTransportLayer::ErrorCode::TP_OK;
    }

    Logger::warn(UDS, "SendJobQueue full.");
    return AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL;
}

IncomingDiagConnection* requestIncomingConnection(
    ::etl::ipool& incomingDiagConnectionPool,
    DiagnosisConfiguration& configuration,
    IDiagSessionManager& sessionManager,
    transport::AbstractTransportLayer& messageSender,
    TransportJob& job)
{
    IncomingDiagConnection* pConnection = nullptr;
    {
        ::async::LockType const lock;
        pConnection
            = acquireIncomingDiagConnection(incomingDiagConnectionPool, configuration.Context);
    }

    if (pConnection != nullptr)
    {
        auto const targetAddress = job.transportMessage->getTargetId();

        pConnection->messageSender      = &messageSender;
        pConnection->diagSessionManager = &sessionManager;
        pConnection->sourceAddress      = job.transportMessage->getSourceId();
        pConnection->targetAddress      = targetAddress;
        pConnection->responseSourceAddress
            = (TransportConfiguration::isFunctionalAddress(targetAddress))
                  ? configuration.DiagAddress
                  : targetAddress;
        pConnection->serviceId = job.transportMessage->getServiceId();
        pConnection->open(configuration.ActivateOutgoingPending);
        pConnection->requestMessage              = job.transportMessage;
        pConnection->responseMessage             = nullptr;
        pConnection->requestNotificationListener = job.processedListener;

        Logger::debug(
            UDS,
            "Opening incoming connection 0x%x --> 0x%x, service 0x%x",
            pConnection->sourceAddress,
            pConnection->targetAddress,
            pConnection->serviceId);

        return pConnection;
    }

    Logger::warn(
        UDS,
        "No incoming diag connection available for 0x%x --> 0x%x, service 0x%x",
        job.transportMessage->getSourceId(),
        job.transportMessage->getTargetId(),
        job.transportMessage->getServiceId());

    Logger::critical(GLOBAL, "!!!! Busy because no incoming connection!");

    return nullptr;
}

static bool isFromValidSender(TransportMessage const& transportMessage)
{
    return TransportConfiguration::isFromTester(transportMessage);
}

static TransportMessage* copyFunctionalRequest(
    TransportMessage& request,
    ::transport::ITransportMessageProvidingListener& providingListener,
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

void sendBusyResponse(
    transport::TransportMessage const* const message,
    DiagnosisConfiguration& configuration,
    ::transport::ITransportMessageProvidingListener& providingListenerHelper,
    transport::TransportMessage& busyMessage)
{
    Logger::error(UDS, "No incoming connection available -> request discarded --> BUSY");

    busyMessage.setSourceAddress(configuration.DiagAddress);
    busyMessage.setTargetAddress(message->getSourceId());
    busyMessage.getPayload()[1] = message->getServiceId();

    ITransportMessageListener::ReceiveResult const status
        = providingListenerHelper.messageReceived(configuration.DiagBusId, busyMessage, nullptr);

    if (status != ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
    {
        Logger::error(UDS, "Could not send BUSY_REPEAT_REQUEST!");
    }
}

enum class PrecheckResult
{
    Abort,
    Busy,
    Ready
};

PrecheckResult precheckRequest(
    TransportJob& job,
    DiagnosisConfiguration& configuration,
    ::transport::ITransportMessageProvidingListener& providingListener,
    ::transport::ITransportMessageProcessedListener* const dispatcherProcessedListener)
{
    bool const isResuming
        = job.transportMessage->getTargetId() == TransportMessage::INVALID_ADDRESS;
    if (isResuming)
    {
        job.transportMessage->setTargetAddress(configuration.DiagAddress);
    }
    if (!configuration.AcceptAllRequests)
    { // check if source is a tester or functional request
        if (!isFromValidSender(*job.transportMessage))
        {
            Logger::warn(
                UDS,
                "Request from invalid source 0x%x discarded",
                job.transportMessage->getSourceId());
            job.processedListener->transportMessageProcessed(
                *job.transportMessage,
                ::transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
            return PrecheckResult::Abort;
        }
    }
    TransportMessage& transportMessage = *job.transportMessage;
    TransportMessage* pRequest         = &transportMessage;
    if (configuration.CopyFunctionalRequests
        && TransportConfiguration::isFunctionallyAddressed(transportMessage))
    {
        pRequest = copyFunctionalRequest(transportMessage, providingListener, configuration);
        if (pRequest != nullptr)
        {
            if (job.processedListener != nullptr)
            {
                job.processedListener->transportMessageProcessed(
                    transportMessage,
                    ::transport::ITransportMessageProcessedListener::ProcessingResult::
                        PROCESSED_NO_ERROR);
            }
            if (dispatcherProcessedListener != nullptr)
            {
                job.processedListener = dispatcherProcessedListener;
            }
            job.transportMessage = pRequest;
        }
        else
        {
            Logger::critical(GLOBAL, "!!!! Busy because no functional buffer!");
            return PrecheckResult::Busy;
        }
    }

    return PrecheckResult::Ready;
}

bool dispatchIncomingRequest(
    TransportJob& job,
    DiagnosisConfiguration& configuration,
    ::etl::ipool& incomingDiagConnectionPool,
    DiagDispatcher& dispatcher,
    DiagJobRoot& diagJobRoot)
{
    IncomingDiagConnection* const pConnection = requestIncomingConnection(
        incomingDiagConnectionPool, configuration, dispatcher.fSessionManager, dispatcher, job);
    if (pConnection == nullptr)
    {
        return true;
    }
    pConnection->diagDispatcher       = &dispatcher;
    DiagReturnCode::Type const result = diagJobRoot.execute(
        *pConnection, job.transportMessage->getPayload(), job.transportMessage->getPayloadLength());
    if (result != DiagReturnCode::OK)
    {
        (void)pConnection->sendNegativeResponse(static_cast<uint8_t>(result), diagJobRoot);
        pConnection->terminate();
    }
    return false;
}

} // namespace

DiagDispatcher::DiagDispatcher(
    ::etl::ipool& incomingDiagConnectionPool,
    ::etl::iqueue<TransportJob>& sendJobQueue,
    DiagnosisConfiguration& configuration,
    IDiagSessionManager& sessionManager,
    DiagJobRoot& jobRoot)
: IDiagDispatcher(sessionManager)
, AbstractTransportLayer(configuration.DiagBusId)
, _incomingDiagConnectionPool(incomingDiagConnectionPool)
, _sendJobQueue(sendJobQueue)
, _configuration(configuration)
, _busyMessageBuffer()
, _asyncProcessQueue(
      ::async::Function::CallType::create<DiagDispatcher, &DiagDispatcher::processQueue>(*this))
, _diagJobRoot(jobRoot)
{
    _busyMessage.init(
        &_busyMessageBuffer[0], BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES);
    _busyMessage.resetValidBytes();
    (void)_busyMessage.append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)_busyMessage.append(0x00U);
    (void)_busyMessage.append(static_cast<uint8_t>(DiagReturnCode::ISO_BUSY_REPEAT_REQUEST));
    _busyMessage.setPayloadLength(BUSY_MESSAGE_LENGTH);
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
        _incomingDiagConnectionPool.begin(),
        _incomingDiagConnectionPool.end(),
        [pNotificationListener](void* const conn) -> bool
        { return reinterpret_cast<void*>(pNotificationListener) == conn; });

    if (connection != _incomingDiagConnectionPool.end())
    {
        ITransportMessageListener::ReceiveResult const status
            = fProvidingListenerHelper.messageReceived(
                _configuration.DiagBusId, transportMessage, pNotificationListener);
        if (status == ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
        {
            return AbstractTransportLayer::ErrorCode::TP_OK;
        }
        else
        {
            return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
        }
    }
    if ((transportMessage.getTargetId() != _configuration.DiagAddress)
        && ((_configuration.BroadcastAddress != TransportMessage::INVALID_ADDRESS)
            && (transportMessage.getTargetId() != _configuration.BroadcastAddress)))
    {
        Logger::error(
            UDS,
            "DiagDispatcher::send(): invalid target 0x%x, expected 0x%x",
            transportMessage.getTargetId(),
            _configuration.DiagAddress);
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }

    auto const result = enqueueMessage(
        _sendJobQueue,
        transportMessage,
        pNotificationListener,
        _defaultTransportMessageProcessedListener);

    if (AbstractTransportLayer::ErrorCode::TP_OK == result)
    {
        ::async::execute(_configuration.Context, _asyncProcessQueue);
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
    if ((transportMessage.getTargetId() != _configuration.DiagAddress)
        && ((_configuration.BroadcastAddress == TransportMessage::INVALID_ADDRESS)
            || (transportMessage.getTargetId() != _configuration.BroadcastAddress)))
    {
        Logger::error(
            UDS,
            "DiagDispatcher::resume(): invalid target 0x%x, expected 0x%x",
            transportMessage.getTargetId(),
            _configuration.DiagAddress);
        return AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }
    else
    {
        transportMessage.setTargetAddress(TransportMessage::INVALID_ADDRESS);
    }

    auto const result = enqueueMessage(
        _sendJobQueue,
        transportMessage,
        pNotificationListener,
        _defaultTransportMessageProcessedListener);
    if (AbstractTransportLayer::ErrorCode::TP_OK == result)
    {
        ::async::execute(_configuration.Context, _asyncProcessQueue);
    }
    return result;
}

void DiagDispatcher::processQueue()
{
    ::async::ModifiableLockType lock;
    while (!_sendJobQueue.empty())
    {
        auto& sendJob = _sendJobQueue.front();
        _sendJobQueue.pop();
        lock.unlock();
        auto const precheckResult
            = precheckRequest(sendJob, _configuration, fProvidingListenerHelper, this);

        bool sendBusyNegativeResponse = false;
        if (precheckResult == PrecheckResult::Busy)
        {
            sendBusyNegativeResponse = true;
        }
        if ((precheckResult == PrecheckResult::Ready) && (!_connectionShutdownRequested))
        {
            sendBusyNegativeResponse = dispatchIncomingRequest(
                sendJob, _configuration, _incomingDiagConnectionPool, *this, _diagJobRoot);
        }

        if (sendBusyNegativeResponse)
        {
            sendBusyResponse(
                sendJob.transportMessage, _configuration, fProvidingListenerHelper, _busyMessage);
            sendJob.processedListener->transportMessageProcessed(
                *sendJob.transportMessage,
                ::transport::ITransportMessageProcessedListener::ProcessingResult::
                    PROCESSED_NO_ERROR);
        }

        lock.lock();
    }
}

void DiagDispatcher::diagConnectionTerminated(IncomingDiagConnection& diagConnection)
{
    auto const requestMessage       = diagConnection.requestMessage;
    auto const notificationListener = diagConnection.requestNotificationListener;
    if ((notificationListener != nullptr) && (requestMessage != nullptr))
    {
        requestMessage->resetValidBytes();
        (void)requestMessage->increaseValidBytes(requestMessage->getPayloadLength());
        notificationListener->transportMessageProcessed(
            *requestMessage,
            ::transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }

    transport::TransportMessage* const responseMessage = diagConnection.responseMessage;
    if (responseMessage != nullptr)
    {
        fProvidingListenerHelper.releaseTransportMessage(*responseMessage);
    }

    {
        ::async::LockType const lock;
        _incomingDiagConnectionPool.destroy(&diagConnection);
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
    _connectionShutdownDelegate  = delegate;
    _connectionShutdownRequested = true;
    checkConnectionShutdownProgress();
}

void DiagDispatcher::checkConnectionShutdownProgress()
{
    if (!_connectionShutdownRequested)
    {
        return;
    }

    ::etl::ipool const& incomingDiagConnections = _incomingDiagConnectionPool;

    if (!incomingDiagConnections.empty())
    {
        Logger::error(
            UDS,
            "DiagDispatcher::problem at shutdown(in: %d/%d)",
            incomingDiagConnections.size(),
            incomingDiagConnections.max_size());
        _incomingDiagConnectionPool.release_all();
    }
    Logger::debug(UDS, "DiagDispatcher incoming connection shutdown complete");
    _connectionShutdownDelegate();
}

AbstractTransportLayer::ErrorCode DiagDispatcher::init()
{
    _connectionShutdownDelegate  = ::etl::delegate<void()>();
    _connectionShutdownRequested = false;
    fEnabled                     = true;
    return AbstractTransportLayer::ErrorCode::TP_OK;
}

ESR_NO_INLINE bool DiagDispatcher::shutdown(ShutdownDelegate const delegate)
{
    Logger::debug(UDS, "DiagDispatcher::shutdown()");
    fEnabled          = false;
    _shutdownDelegate = delegate;
    shutdownIncomingConnections(
        ::etl::delegate<void()>::
            create<DiagDispatcher, &DiagDispatcher::connectionManagerShutdownComplete>(*this));
    return false;
}

void DiagDispatcher::connectionManagerShutdownComplete()
{
    Logger::debug(UDS, "DiagDispatcher::connectionManagerShutdownComplete()");
    _shutdownDelegate(*this);
}

void DiagDispatcher::transportMessageProcessed(
    TransportMessage& transportMessage, ProcessingResult const /* result */)
{
    fProvidingListenerHelper.releaseTransportMessage(transportMessage);
}

} // namespace uds
