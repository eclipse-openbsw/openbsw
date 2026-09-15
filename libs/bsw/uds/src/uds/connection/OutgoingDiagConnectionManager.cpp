/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/connection/OutgoingDiagConnectionManager.h"

#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProvider.h>
#include <transport/ITransportMessageProvidingListener.h>
#include <transport/TransportConfiguration.h>
#include <transport/TransportMessage.h>

#include "uds/DiagDispatcher.h"
#include "uds/DiagnosisConfiguration.h"
#include "uds/UdsLogger.h"
#include "uds/connection/ErrorCode.h"
#include "uds/connection/OutgoingDiagConnection.h"

using ::transport::AbstractTransportLayer;
using ::transport::ITransportMessageProvider;
using ::transport::ITransportMessageProvidingListener;
using ::transport::TransportConfiguration;
using ::transport::TransportMessage;
using ::util::logger::Logger;
using ::util::logger::UDS;

namespace uds
{
OutgoingDiagConnectionManager::OutgoingDiagConnectionManager(
    DiagnosisConfiguration& configuration,
    transport::AbstractTransportLayer& outgoingSender,
    transport::ITransportMessageProvider& outgoingProvider,
    DiagDispatcher& diagDispatcher)
: _configuration(configuration)
, _outgoingTransportMessageSender(outgoingSender)
, _outgoingTransportMessageProvider(outgoingProvider)
, _diagDispatcher(diagDispatcher)
, _outgoingDiagConnections()
, _releasedOutgoingDiagConnections()
, _shutdownRequested(false)
{}

void OutgoingDiagConnectionManager::diagConnectionTerminated(
    ManagedOutgoingDiagConnection& diagConnection)
{
    TransportMessage* const pRequestMessage = diagConnection._requestMessage;
    if (pRequestMessage != nullptr)
    {
        _outgoingTransportMessageProvider.releaseTransportMessage(*pRequestMessage);
    }

    {
        ::async::LockType const lock;
        _releasedOutgoingDiagConnections.erase(diagConnection);
        _outgoingDiagConnections.push_back(diagConnection);
    }
    checkShutdownProgress();
}

IOutgoingDiagConnectionProvider::ErrorCode OutgoingDiagConnectionManager::requestOutgoingConnection(
    uint16_t const targetAddress,
    OutgoingDiagConnection*& pOutgoingConnection,
    TransportMessage* pRequestMessage)
{
    ::async::ModifiableLockType lock;
    pOutgoingConnection = nullptr;
    if ((!_outgoingDiagConnections.empty()) && (!_shutdownRequested))
    {
        ManagedOutgoingDiagConnection* const pConnection = &_outgoingDiagConnections.front();
        _outgoingDiagConnections.pop_front();
        lock.unlock();
        uint16_t payloadSize;
        if (TransportConfiguration::isFunctionalAddress(targetAddress))
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
            result = _outgoingTransportMessageProvider.getTransportMessage(
                _configuration.DiagBusId,
                _configuration.DiagAddress,
                targetAddress,
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
            pConnection->_messageSender         = &_outgoingTransportMessageSender;
            pConnection->_diagConnectionManager = this;
            pConnection->_context               = _configuration.Context;
            pRequestMessage->resetValidBytes();
            pConnection->_requestMessage = pRequestMessage;
            pConnection->_sourceAddress  = _configuration.DiagAddress;
            pConnection->_targetAddress  = targetAddress;
            pConnection->open();
            lock.lock();
            _releasedOutgoingDiagConnections.push_back(*pConnection);
            lock.unlock();
            pOutgoingConnection = pConnection;
            return IOutgoingDiagConnectionProvider::CONNECTION_OK;
        }
        else if (result == ITransportMessageProvidingListener::ErrorCode::TPMSG_NO_MSG_AVAILABLE)
        {
            Logger::warn(
                UDS, "No request buffer available for outgoing connection to 0x%x", targetAddress);
            lock.lock();
            _outgoingDiagConnections.push_back(*pConnection);
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
                targetAddress);
            lock.lock();
            _outgoingDiagConnections.push_back(*pConnection);
            lock.unlock();
            return IOutgoingDiagConnectionProvider::GENERAL_ERROR;
        }
    }
    else
    {
        lock.unlock();
        Logger::warn(UDS, "No outgoing connection available for target 0x%x", targetAddress);
        return IOutgoingDiagConnectionProvider::GENERAL_ERROR;
    }
}

ManagedOutgoingDiagConnection*
OutgoingDiagConnectionManager::getExpectingConnection(TransportMessage const& transportMessage)
{
    ManagedOutgoingDiagConnection* pExpectingConnection = nullptr;
    uint16_t maxMatchLevel                              = 0U;
    ::async::ModifiableLockType const lock;
    for (ManagedOutgoingDiagConnectionList::iterator itr = _releasedOutgoingDiagConnections.begin();
         itr != _releasedOutgoingDiagConnections.end();
         ++itr)
    {
        uint16_t const matchLevel    = itr->isExpectedResponse(transportMessage);
        uint8_t const* const payload = itr->_requestMessage->getPayload();
        uint16_t const payloadLength = itr->_requestMessage->getPayloadLength();
        Logger::debug(
            UDS,
            "(0x%x --> 0x%x): 0x%x 0x%x 0x%x (len=%u) has matchlevel %d",
            itr->_requestMessage->getSourceId(),
            itr->_requestMessage->getTargetId(),
            (payloadLength > 0U) ? payload[0] : 0U,
            (payloadLength > 1U) ? payload[1] : 0U,
            (payloadLength > 2U) ? payload[2] : 0U,
            payloadLength,
            matchLevel);
        if (matchLevel > maxMatchLevel)
        {
            maxMatchLevel        = matchLevel;
            pExpectingConnection = itr.operator->();
        }
    }
    return pExpectingConnection;
}

bool OutgoingDiagConnectionManager::hasConflictingConnection(
    OutgoingDiagConnection const& connection)
{
    ::async::LockType const lock;
    for (ManagedOutgoingDiagConnectionList::iterator itr = _releasedOutgoingDiagConnections.begin();
         itr != _releasedOutgoingDiagConnections.end();
         ++itr)
    {
        if ((itr.operator->() != &connection) && (itr->isConflicting(connection)))
        {
            return true;
        }
    }
    return false;
}

void OutgoingDiagConnectionManager::processPendingResponses()
{
    ::async::LockType const lock;
    for (ManagedOutgoingDiagConnectionList::iterator itr = _releasedOutgoingDiagConnections.begin();
         itr != _releasedOutgoingDiagConnections.end();
         ++itr)
    {
        itr->processResponseQueue();
    }
}

void OutgoingDiagConnectionManager::triggerResponseProcessing() { _diagDispatcher.trigger(); }

void OutgoingDiagConnectionManager::shutdown()
{
    _shutdownRequested = true;
    checkShutdownProgress();
}

void OutgoingDiagConnectionManager::checkShutdownProgress()
{
    if (_shutdownRequested)
    {
        if (!_releasedOutgoingDiagConnections.empty())
        {
            Logger::error(
                UDS,
                "OutgoingDiagConnectionManager::problem at shutdown(out: %d/%d)",
                _outgoingDiagConnections.size(),
                _outgoingDiagConnections.size() + _releasedOutgoingDiagConnections.size());
            while (!_releasedOutgoingDiagConnections.empty())
            {
                _releasedOutgoingDiagConnections.front().terminate();
            }
        }
        Logger::debug(UDS, "OutgoingDiagConnectionManager shutdown complete");
    }
}

} // namespace uds
