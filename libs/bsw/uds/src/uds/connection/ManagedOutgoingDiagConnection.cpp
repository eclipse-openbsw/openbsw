/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/connection/ManagedOutgoingDiagConnection.h"

#include <async/Async.h>
#include <etl/error_handler.h>
#include <transport/TransportConfiguration.h>
#include <transport/TransportMessage.h>

#include "uds/DiagCodes.h"
#include "uds/DiagDispatcher.h"
#include "uds/DiagReturnCode.h"
#include "uds/UdsLogger.h"
#include "uds/connection/OutgoingDiagConnectionManager.h"

namespace uds
{
using ::transport::TransportConfiguration;
using ::transport::TransportMessage;
using ::util::logger::Logger;
using ::util::logger::UDS;

ManagedOutgoingDiagConnection::ManagedOutgoingDiagConnection()
: OutgoingDiagConnection()
, _pendingResponses(nullptr)
, _processingResponse(false)
, _connectionTerminationIsPending(false)
{}

void ManagedOutgoingDiagConnection::setSourceAddress(uint16_t const sourceAddress)
{
    ETL_ASSERT(_requestMessage != nullptr, ETL_ERROR_GENERIC("_requestMessage must not be null"));
    _sourceAddress = sourceAddress;
    _requestMessage->setSourceAddress(sourceAddress);
}

void ManagedOutgoingDiagConnection::setTargetAddress(uint16_t const targetAddress)
{
    ETL_ASSERT(_requestMessage != nullptr, ETL_ERROR_GENERIC("_requestMessage must not be null"));
    _targetAddress = targetAddress;
    _requestMessage->setTargetAddress(targetAddress);
}

uint16_t ManagedOutgoingDiagConnection::isExpectedResponse(TransportMessage const& transportMessage)
{
    if (!_open)
    {
        return 0U;
    }

    // check Sender
    if ((transportMessage.sourceAddress() != _targetAddress)
        && (_targetAddress != DiagCodes::FUNCTIONAL_ID_ALL_ISO14229_ECUS)
        && (_targetAddress != DiagCodes::FUNCTIONAL_ID_ALL_KWP2000_ECUS))
    {
        Logger::debug(
            UDS,
            "Incorrect Sender 0x%x, Target was 0x%x",
            transportMessage.sourceAddress(),
            _targetAddress);
        return 0U;
    }

    return verifyResponse(transportMessage);
}

void ManagedOutgoingDiagConnection::processResponseQueue()
{
    if (!_processingResponse)
    {
        if ((_pendingResponses != nullptr) && (!_pendingResponses->empty()))
        {
            if (_sender != nullptr)
            {
                _responseMessage    = _pendingResponses->front().transportMessage;
                _processingResponse = true;
                _sender->responseReceived(
                    *this,
                    static_cast<uint8_t>(_responseMessage->sourceAddress()),
                    ::etl::span<uint8_t const>(
                        _responseMessage->getPayload(), _responseMessage->getPayloadLength()));
            }
            else
            {
                Logger::error(
                    UDS,
                    "ManagedOutgoingDiagConnection::processResponseQueue(), _sender == nullptr --> "
                    "discarding pending responses");
                while (!_pendingResponses->empty())
                {
                    _pendingResponses->pop();
                }
            }
        }
    }
}

void ManagedOutgoingDiagConnection::responseReceived(
    transport::TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (_sender != nullptr)
    { // this connection is still active
        bool isResponsePending  = false;
        ErrorCode timeoutStatus = ::uds::ErrorCode::OK;
        if ((transportMessage.getPayloadLength() >= DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH)
            && (transportMessage.getPayload()[0] == DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER)
            && (transportMessage.getPayload()[2]
                == static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING)))
        {
            timeoutStatus     = setTimeout(RESPONSE_PENDING_TIMEOUT);
            isResponsePending = true;
        }
        else if (TransportConfiguration::isFunctionalAddress(_targetAddress))
        {
            timeoutStatus = setTimeout(_timeout);
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
                _sourceAddress,
                _targetAddress);
        }
        if (isResponsePending && _suppressIncomingPendings)
        {
            if (pNotificationListener != nullptr)
            {
                pNotificationListener->transportMessageProcessed(
                    transportMessage,
                    ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
            }
        }
        else if ((_pendingResponses != nullptr) && (!_pendingResponses->full()))
        {
            TransportJob& response     = _pendingResponses->emplace();
            response.transportMessage  = &transportMessage;
            response.processedListener = pNotificationListener;
            processResponseQueue();
        }
        else
        {
            Logger::warn(
                UDS,
                "ManagedOutgoingDiagConnection::responseReceived(0x%x --> 0x%x) response queue "
                "full!!",
                _sourceAddress,
                _targetAddress);
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
            "ManagedOutgoingDiagConnection::responseReceived(0x%x --> 0x%x) with _sender == "
            "nullptr",
            _sourceAddress,
            _targetAddress);
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
    ::async::LockType const lock;
    if (_open || _connectionTerminationIsPending)
    {
        _open                           = false;
        _connectionTerminationIsPending = false;
        Logger::debug(
            UDS,
            "ManagedOutgoingDiagConnection::terminate(): 0x%x --> 0x%x, "
            "service 0x%x, %d pending responses",
            _sourceAddress,
            _targetAddress,
            _serviceId,
            (nullptr != _pendingResponses) ? _pendingResponses->size() : 0U);
        _responseTimeout._asyncTimeout.cancel();
        _responsePendingTimeout._asyncTimeout.cancel();
        _responseTimeout._isActive        = false;
        _responsePendingTimeout._isActive = false;

        _sendInProgress  = false;
        _infiniteTimeout = false;
        _sender          = nullptr;
        if (_diagConnectionManager != nullptr)
        {
            _diagConnectionManager->diagConnectionTerminated(*this);
        }
        if (_pendingResponses != nullptr)
        {
            // clean up pending responses
            while (!_pendingResponses->empty())
            {
                TransportJob& response = _pendingResponses->front();
                _pendingResponses->pop();
                if (response.processedListener != nullptr)
                {
                    if (_processingResponse)
                    {
                        // terminate is implicit responseProcessed --> no error
                        response.processedListener->transportMessageProcessed(
                            *response.transportMessage,
                            ITransportMessageProcessedListener::ProcessingResult::
                                PROCESSED_NO_ERROR);
                        _processingResponse = false;
                    }
                    else
                    {
                        response.processedListener->transportMessageProcessed(
                            *response.transportMessage,
                            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
                    }
                }
            }
        }
        // just in case _pendingResponses was empty
        _processingResponse = false;
    }
    else
    {
        // nothing to do
    }
}

void ManagedOutgoingDiagConnection::responseProcessed()
{
    _processingResponse = false;
    if ((_pendingResponses != nullptr) && (!_pendingResponses->empty()))
    {
        TransportJob& response = _pendingResponses->front();
        _pendingResponses->pop();
        if (response.processedListener != nullptr)
        {
            response.processedListener->transportMessageProcessed(
                *response.transportMessage,
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        }
        if (_diagConnectionManager != nullptr)
        {
            _diagConnectionManager->triggerResponseProcessing();
        }
    }
    else
    {
        Logger::error(
            UDS,
            "ManagedOutgoingDiagConnection::responseProcessed() called "
            "with no pending response!");
    }

    if (_connectionTerminationIsPending && (_pendingResponses != nullptr)
        && (_pendingResponses->empty()))
    {
        terminate();
    }
}

void ManagedOutgoingDiagConnection::timeoutOccurred()
{
    if ((_pendingResponses != nullptr) && (_pendingResponses->empty()))
    {
        terminate();
    }
    else
    {
        _open                           = false;
        _connectionTerminationIsPending = true;
    }
}

} // namespace uds
