// Copyright 2024 Accenture.

#include "uds/connection/DiagConnectionManager.h"

#include "transport/AbstractTransportLayer.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "uds/DiagDispatcher.h"
#include "uds/DiagnosisConfiguration.h"
#include "uds/UdsLogger.h"
#include "uds/connection/ErrorCode.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <async/Async.h>
#include <etl/pool.h>

using ::transport::AbstractTransportLayer;
using ::transport::ITransportMessageProcessedListener;
using ::transport::ITransportMessageProvider;
using ::transport::ITransportMessageProvidingListener;
using ::transport::TransportConfiguration;
using ::transport::TransportMessage;
using ::util::logger::Logger;
using ::util::logger::UDS;

namespace uds
{
DiagConnectionManager::DiagConnectionManager(
    AbstractDiagnosisConfiguration& configuration,
    AbstractTransportLayer& outgoingSender,
    ITransportMessageProvider& outgoingProvider,
    ::async::ContextType context,
    DiagDispatcher2& diagDispatcher)
: fConfiguration(configuration)
, fOutgoingTransportMessageSender(outgoingSender)
, fOutgoingTransportMessageProvider(outgoingProvider)
, fContext(context)
, fDiagDispatcher(diagDispatcher)
, fShutdownRequested(false)
{}

void DiagConnectionManager::diagConnectionTerminated(IncomingDiagConnection& diagConnection)
{
    TransportMessage* const requestMessage = diagConnection.fpRequestMessage;
    ITransportMessageProcessedListener* const notificationListener
        = diagConnection.getRequestNotificationListener();
    if ((nullptr != notificationListener) && (nullptr != requestMessage))
    {
        requestMessage->resetValidBytes();
        (void)requestMessage->increaseValidBytes(requestMessage->getPayloadLength());
        notificationListener->transportMessageProcessed(
            *requestMessage,
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }

    TransportMessage* const responseMessage = diagConnection.fpResponseMessage;
    if (responseMessage != nullptr)
    {
        fOutgoingTransportMessageProvider.releaseTransportMessage(*responseMessage);
    }

    {
        ::async::LockType const lock;
        fConfiguration.releaseIncomingDiagConnection(diagConnection);
    }

    checkShutdownProgress();
}

IncomingDiagConnection*
DiagConnectionManager::requestIncomingConnection(TransportMessage& requestMessage)
{
    if (fShutdownRequested)
    {
        return nullptr;
    }
    IncomingDiagConnection* pConnection = nullptr;
    {
        ::async::LockType const lock;
        pConnection = fConfiguration.acquireIncomingDiagConnection();
    }
    if (pConnection != nullptr)
    {
        pConnection->fpDiagConnectionManager = this;
        pConnection->fpMessageSender         = &fOutgoingTransportMessageSender;
        pConnection->setDiagSessionManager(fDiagDispatcher.getDiagSessionManager());
        pConnection->fSourceId  = requestMessage.getSourceId();
        pConnection->fTargetId  = requestMessage.getTargetId();
        pConnection->fServiceId = requestMessage.getServiceId();
        pConnection->open(fConfiguration.ActivateOutgoingPending);
        pConnection->fpRequestMessage  = &requestMessage;
        pConnection->fpResponseMessage = nullptr;
        return pConnection;
    }
    else
    {
        Logger::warn(
            UDS,
            "No incoming diag connection available for 0x%x --> 0x%x, service 0x%x",
            requestMessage.getSourceId(),
            requestMessage.getTargetId(),
            requestMessage.getServiceId());
        return nullptr;
    }
}

void DiagConnectionManager::triggerResponseProcessing() { fDiagDispatcher.trigger(); }

bool DiagConnectionManager::isPendingActivated() const
{
    return fConfiguration.ActivateOutgoingPending;
}

uint16_t DiagConnectionManager::getSourceDiagId() const { return fConfiguration.DiagAddress; }

uint8_t DiagConnectionManager::getBusId() const { return fConfiguration.DiagBusId; }

void DiagConnectionManager::shutdown(::etl::delegate<void()> const delegate)
{
    fShutdownDelegate  = delegate;
    fShutdownRequested = true;
    checkShutdownProgress();
}

void DiagConnectionManager::checkShutdownProgress()
{
    if (fShutdownRequested)
    {
        ::etl::ipool const& incomingDiagConnections = fConfiguration.incomingDiagConnectionPool();

        if ((!incomingDiagConnections.full()))
        {
            Logger::error(
                UDS,
                "DiagConnectionManager::problem at shutdown(in: %d/%d)",
                incomingDiagConnections.size(),
                incomingDiagConnections.max_size());
            fConfiguration.clearIncomingDiagConnections();
        }
        Logger::debug(UDS, "DiagConnectionManager shutdown complete");
        fShutdownDelegate();
    }
}

} // namespace uds
