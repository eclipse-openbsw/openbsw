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
#include "uds/connection/ManagedOutgoingDiagConnection.h"
#include "uds/connection/OutgoingDiagConnection.h"

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
, fOutgoingDiagConnections(configuration.OutgoingDiagConnections)
, fReleasedOutgoingDiagConnections()
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

void DiagConnectionManager::diagConnectionTerminated(ManagedOutgoingDiagConnection& diagConnection)
{
    TransportMessage* const pRequestMessage = diagConnection.fpRequestMessage;
    if (pRequestMessage != nullptr)
    {
        fOutgoingTransportMessageProvider.releaseTransportMessage(*pRequestMessage);
    }

    {
        ::async::LockType const lock;
        fReleasedOutgoingDiagConnections.erase(diagConnection);
        fOutgoingDiagConnections.push_back(diagConnection);
    }
    checkShutdownProgress();
}

IOutgoingDiagConnectionProvider::ErrorCode DiagConnectionManager::requestOutgoingConnection(
    uint16_t const targetId,
    OutgoingDiagConnection*& pOutgoingConnection,
    TransportMessage* pRequestMessage)
{
    ::async::ModifiableLockType lock;
    pOutgoingConnection = nullptr;
    if ((!fOutgoingDiagConnections.empty()) && (!fShutdownRequested))
    {
        ManagedOutgoingDiagConnection* const pConnection = &fOutgoingDiagConnections.front();
        fOutgoingDiagConnections.pop_front();
        lock.unlock();
        uint16_t payloadSize;
        if (TransportConfiguration::isFunctionalAddress(targetId))
        {
            payloadSize = TransportConfiguration::MAX_FUNCTIONAL_MESSAGE_PAYLOAD_SIZE;
        }
        else
        {
            payloadSize = TransportConfiguration::DIAG_PAYLOAD_SIZE;
        }
        ITransportMessageProvider::ErrorCode result
            = ITransportMessageProvider::ErrorCode::TPMSG_OK;
        if (nullptr == pRequestMessage)
        {
            result = fOutgoingTransportMessageProvider.getTransportMessage(
                fConfiguration.DiagBusId,
                fConfiguration.DiagAddress,
                targetId,
                payloadSize,
                {},
                pRequestMessage);
        }
        else
        {
            if (payloadSize > pRequestMessage->getMaxPayloadLength())
            {
                pRequestMessage = nullptr;
                result          = ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE;
            }
        }
        if ((result == ITransportMessageProvidingListener::ErrorCode::TPMSG_OK)
            && (pRequestMessage != nullptr))
        {
            pConnection->fpDiagConnectionManager = this;
            pConnection->fpMessageSender         = &fOutgoingTransportMessageSender;
            pRequestMessage->resetValidBytes();
            pConnection->fpRequestMessage = pRequestMessage;
            pConnection->fSourceId        = fConfiguration.DiagAddress;
            pConnection->fTargetId        = targetId;
            pConnection->open();
            lock.lock();
            fReleasedOutgoingDiagConnections.push_back(*pConnection);
            lock.unlock();
            pOutgoingConnection = pConnection;
            return IOutgoingDiagConnectionProvider::CONNECTION_OK;
        }
        else if (result == ITransportMessageProvidingListener::ErrorCode::TPMSG_NO_MSG_AVAILABLE)
        {
            Logger::warn(
                UDS, "No request buffer available for outgoing connection to 0x%x", targetId);
            lock.lock();
            fOutgoingDiagConnections.push_back(*pConnection);
            lock.unlock();
            return IOutgoingDiagConnectionProvider::NO_CONNECTION_AVAILABLE;
        }
        else
        {
            Logger::warn(
                UDS,
                "Error 0x%x getting request buffer available for outgoing "
                "connection to 0x%x",
                result,
                targetId);
            lock.lock();
            fOutgoingDiagConnections.push_back(*pConnection);
            lock.unlock();
            return IOutgoingDiagConnectionProvider::GENERAL_ERROR;
        }
    }
    else
    {
        lock.unlock();
        Logger::warn(UDS, "No outgoing connection available for target 0x%x", targetId);
        return IOutgoingDiagConnectionProvider::GENERAL_ERROR;
    }
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

ManagedOutgoingDiagConnection*
DiagConnectionManager::getExpectingConnection(TransportMessage const& transportMessage)
{
    ManagedOutgoingDiagConnection* pExpectingConnection = nullptr;
    uint16_t maxMatchLevel                              = 0U;
    ::async::ModifiableLockType const lock;
    for (ManagedOutgoingDiagConnectionList::iterator itr = fReleasedOutgoingDiagConnections.begin();
         itr != fReleasedOutgoingDiagConnections.end();
         ++itr)
    {
        uint16_t const matchLevel    = itr->isExpectedResponse(transportMessage);
        uint8_t const* const payload = itr->fpRequestMessage->getPayload();
        Logger::debug(
            UDS,
            "(0x%x --> 0x%x): 0x%x 0x%x 0x%x has matchlevel %d",
            itr->fpRequestMessage->getSourceId(),
            itr->fpRequestMessage->getTargetId(),
            payload[0],
            payload[1],
            payload[2],
            matchLevel);
        if (matchLevel > maxMatchLevel)
        {
            maxMatchLevel        = matchLevel;
            pExpectingConnection = itr.operator->();
        }
    }
    return pExpectingConnection;
}

bool DiagConnectionManager::hasConflictingConnection(OutgoingDiagConnection const& connection)
{
    ::async::LockType const lock;
    for (ManagedOutgoingDiagConnectionList::iterator itr = fReleasedOutgoingDiagConnections.begin();
         itr != fReleasedOutgoingDiagConnections.end();
         ++itr)
    {
        if ((itr.operator->() != &connection) && (itr->isConflicting(connection)))
        {
            return true;
        }
    }
    return false;
}

void DiagConnectionManager::processPendingResponses()
{
    ::async::LockType const lock;
    for (ManagedOutgoingDiagConnectionList::iterator itr = fReleasedOutgoingDiagConnections.begin();
         itr != fReleasedOutgoingDiagConnections.end();
         ++itr)
    {
        itr->processResponseQueue();
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

        if ((!incomingDiagConnections.full()) || (!fReleasedOutgoingDiagConnections.empty()))
        {
            Logger::error(
                UDS,
                "DiagConnectionManager::problem at shutdown(in: %d/%d, out: %d/%d)",
                incomingDiagConnections.size(),
                incomingDiagConnections.max_size(),
                fOutgoingDiagConnections.size(),
                fOutgoingDiagConnections.size() + fReleasedOutgoingDiagConnections.size());
            fConfiguration.clearIncomingDiagConnections();
            fReleasedOutgoingDiagConnections.clear();
        }
        Logger::debug(UDS, "DiagConnectionManager shutdown complete");
        fShutdownDelegate();
    }
}

} // namespace uds
