/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/connection/OutgoingDiagConnection.h"

#include <etl/error_handler.h>
#include <etl/platform.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/TransportConfiguration.h>
#include <transport/TransportMessage.h>

#include "uds/DiagCodes.h"
#include "uds/DiagReturnCode.h"
#include "uds/UdsLogger.h"
#include "uds/connection/OutgoingDiagConnectionManager.h"

namespace uds
{
using ::transport::AbstractTransportLayer;
using ::transport::TransportConfiguration;

using ::util::logger::Logger;
using ::util::logger::UDS;

::uds::ErrorCode OutgoingDiagConnection::sendDiagRequest(
    uint16_t const length,
    AbstractDiagApplication& sender,
    uint32_t const timeout,
    uint16_t const matchingResponseBytes,
    bool const acceptNegativeResponse,
    bool const suppressSend,
    bool const suppressIncomingPendings)
{
    if (!_open)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }
    if (_sendInProgress)
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
    if (_messageSender == nullptr)
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
    _timeout = timeout;
    if (timeout == INFINITE_RESPONSE_TIMEOUT)
    {
        _infiniteTimeout = true;
    }
    else
    {
        _infiniteTimeout              = false;
        ErrorCode const timeoutStatus = setTimeout(_timeout);
        if (timeoutStatus != ::uds::ErrorCode::OK)
        {
            return ::uds::ErrorCode::SEND_FAILED;
        }
    }
    _matchingResponseBytes    = matchingResponseBytes;
    _acceptNegativeResponse   = acceptNegativeResponse;
    _suppressIncomingPendings = suppressIncomingPendings;
    _sender                   = &sender;
    _requestMessage->resetValidBytes();
    (void)_requestMessage->increaseValidBytes(length);
    _requestMessage->setPayloadLength(length);
    _serviceId = _requestMessage->getServiceId();
    if ((_diagConnectionManager != nullptr)
        && (_diagConnectionManager->hasConflictingConnection(*this)))
    {
        return ::uds::ErrorCode::CONFLICTING_REQUEST;
    }
    _sendInProgress = true;
    AbstractTransportLayer::ErrorCode sendResult;
    if (suppressSend)
    {
        _sendInProgress = false;
        sendResult      = AbstractTransportLayer::ErrorCode::TP_OK;
    }
    else
    {
        sendResult = _messageSender->send(*_requestMessage, this);
    }
    if (sendResult == AbstractTransportLayer::ErrorCode::TP_OK)
    {
        return ::uds::ErrorCode::OK;
    }
    else
    {
        _sendInProgress = false;
        return ::uds::ErrorCode::SEND_FAILED;
    }
}

::uds::ErrorCode OutgoingDiagConnection::resend(uint32_t const timeout)
{
    if (_sender != nullptr)
    {
        return sendDiagRequest(_requestMessage->getPayloadLength(), *_sender, timeout);
    }
    else
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
}

bool OutgoingDiagConnection::isConflicting(OutgoingDiagConnection const& connection)
{
    if (_sender == nullptr)
    { // request has not been sent --> no conflicts
        return false;
    }
    if (_sourceAddress != connection._sourceAddress)
    { // no conflict if sourceIds differ
        return false;
    }
    if ((!TransportConfiguration::isFunctionalAddress(_targetAddress))
        && (!TransportConfiguration::isFunctionalAddress(connection._targetAddress))
        && (_targetAddress != connection._targetAddress))
    { // there are no broadcasts and targetIds differ
        return false;
    }
    if (_serviceId != connection._serviceId)
    { // no conflict if serviceIds differ
        return false;
    }
    if ((_acceptNegativeResponse != connection._acceptNegativeResponse)
        && ((_matchingResponseBytes > 0U) || (connection._matchingResponseBytes > 0U)))
    {
        return false;
    }

    uint16_t const bytesToCheck = (_matchingResponseBytes < connection._matchingResponseBytes)
                                      ? _matchingResponseBytes
                                      : connection._matchingResponseBytes;
    for (uint16_t i = 1U; i <= bytesToCheck; ++i)
    {
        if ((*_requestMessage)[i] != (*connection._requestMessage)[i])
        {
            return false;
        }
    }
    return true;
}

uint16_t OutgoingDiagConnection::verifyResponse(transport::TransportMessage const& response)
{
    uint16_t matchingBytes = 0U;
    if (response.getPayloadLength() < 1U)
    {
        return 0U;
    }
    if (DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER == response[0])
    { // check negative response
        if ((response.getPayloadLength() == DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH)
            && (_acceptNegativeResponse))
        {
            if (_serviceId == response[1])
            {
                return 1U;
            }
        }
        return 0U;
    }
    // now check positive response
    if ((_serviceId + DiagReturnCode::POSITIVE_RESPONSE_OFFSET) == response[0])
    {
        ++matchingBytes;
    }
    else
    { // serviceId does not match
        return 0U;
    }
    if (_matchingResponseBytes > (response.getPayloadLength() - 1U)) //-1 because of serviceId
    {                                                                // we cannot check enough bytes
        return 0U;
    }
    if (_matchingResponseBytes > 0U)
    {
        // loop starts with 1 because serviceId has special treatment above
        for (uint16_t i = 1U; i <= _matchingResponseBytes; ++i)
        {
            if (_requestMessage->getPayload()[i] == response[i])
            {
                ++matchingBytes;
            }
        }
        if (matchingBytes <= _matchingResponseBytes)
        { // not all bytes matched --> not for us!
            return 0U;
        }
    }
    return matchingBytes;
}

::uds::ErrorCode OutgoingDiagConnection::setTimeout(uint32_t const newTimeout)
{
    if (!_open)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }
    if (_infiniteTimeout)
    {
        return ::uds::ErrorCode::OK;
    }
    _responseTimeout._asyncTimeout.cancel();
    _responsePendingTimeout._asyncTimeout.cancel();
    _responseTimeout._isActive        = false;
    _responsePendingTimeout._isActive = false;
    if (newTimeout == RESPONSE_PENDING_TIMEOUT)
    {
        ::async::schedule(
            _context,
            _responsePendingTimeout,
            _responsePendingTimeout._asyncTimeout,
            newTimeout,
            ::async::TimeUnit::MILLISECONDS);
        _responsePendingTimeout._isActive = true;
    }
    else
    {
        ::async::schedule(
            _context,
            _responseTimeout,
            _responseTimeout._asyncTimeout,
            newTimeout,
            ::async::TimeUnit::MILLISECONDS);
        _responseTimeout._isActive = true;
    }
    return ::uds::ErrorCode::OK;
}

::etl::span<uint8_t> OutgoingDiagConnection::getRequestBuffer()
{
    ETL_ASSERT(_requestMessage != nullptr, ETL_ERROR_GENERIC("_requestMessage must not be null"));
    return ::etl::span<uint8_t>(
        _requestMessage->getPayload(), _requestMessage->getMaxPayloadLength());
}

void OutgoingDiagConnection::transportMessageProcessed(
    transport::TransportMessage& transportMessage, ProcessingResult const result)
{
    if (_sender != nullptr)
    {
        _sendInProgress = false;
        if (ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR == result)
        {
            _sender->requestSent(*this, AbstractDiagApplication::REQUEST_SENT);
        }
        else
        {
            Logger::warn(
                UDS,
                "OutgoingDiagConnection: sending request 0x%x --> 0x%x failed",
                transportMessage.sourceAddress(),
                transportMessage.targetAddress());
            _sender->requestSent(*this, AbstractDiagApplication::REQUEST_SEND_FAILED);
        }
    }
    else
    {
        Logger::warn(
            UDS,
            "OutgoingDiagConnection::transportMessageProcessed() called with _sender == nullptr");
    }
}

void OutgoingDiagConnection::execute(::async::RunnableType const& timeout)
{
    if (&timeout == static_cast<::async::RunnableType const*>(&_responseTimeout))
    {
        _responseTimeout._isActive = false;
    }
    else if (&timeout == static_cast<::async::RunnableType const*>(&_responsePendingTimeout))
    {
        _responsePendingTimeout._isActive = false;
    }

    if (_sender != nullptr)
    {
        if ((!_responseTimeout._isActive) && (!_responsePendingTimeout._isActive))
        {
            _keepAlive = false;
            _sender->responseTimeout(*this);
            if (!_keepAlive)
            {
                timeoutOccurred();
            }
            else
            {
                _keepAlive = false;
            }
        }
    }
    else
    {
        Logger::warn(UDS, "OutgoingDiagConnection::timeout() called with _sender == nullptr");
    }
}

} // namespace uds
