// Copyright 2024 Accenture.

#include "uds/connection/OutgoingDiagConnection.h"

#include "platform/config.h"
#include "transport/AbstractTransportLayer.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "uds/DiagCodes.h"
#include "uds/DiagReturnCode.h"
#include "uds/UdsLogger.h"
#include "uds/connection/DiagConnectionManager.h"
#include "util/estd/assert.h"

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
    if (!fOpen)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }
    if (fSendInProgress)
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
    if (nullptr == fpMessageSender)
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
    fTimeout = timeout;
    if (timeout == INFINITE_RESPONSE_TIMEOUT)
    {
        fInfiniteTimeout = true;
    }
    else
    {
        fInfiniteTimeout              = false;
        ErrorCode const timeoutStatus = setTimeout(fTimeout);
        if (timeoutStatus != ::uds::ErrorCode::OK)
        {
            return ::uds::ErrorCode::SEND_FAILED;
        }
    }
    fMatchingResponseBytes    = matchingResponseBytes;
    fAcceptNegativeResponse   = acceptNegativeResponse;
    fSuppressIncomingPendings = suppressIncomingPendings;
    fpSender                  = &sender;
    fpRequestMessage->resetValidBytes();
    (void)fpRequestMessage->increaseValidBytes(length);
    fpRequestMessage->setPayloadLength(length);
    fServiceId = fpRequestMessage->getServiceId();
    if ((nullptr != fpDiagConnectionManager)
        && (fpDiagConnectionManager->hasConflictingConnection(*this)))
    {
        return ::uds::ErrorCode::CONFLICTING_REQUEST;
    }
    fSendInProgress = true;
    AbstractTransportLayer::ErrorCode sendResult;
    if (suppressSend)
    {
        fSendInProgress = false;
        sendResult      = AbstractTransportLayer::ErrorCode::TP_OK;
    }
    else
    {
        sendResult = fpMessageSender->send(*fpRequestMessage, this);
    }
    if (sendResult == AbstractTransportLayer::ErrorCode::TP_OK)
    {
        return ::uds::ErrorCode::OK;
    }
    else
    {
        fSendInProgress = false;
        return ::uds::ErrorCode::SEND_FAILED;
    }
}

::uds::ErrorCode OutgoingDiagConnection::resend(uint32_t const timeout)
{
    if (fpSender != nullptr)
    {
        return sendDiagRequest(fpRequestMessage->getPayloadLength(), *fpSender, timeout);
    }
    else
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
}

bool OutgoingDiagConnection::isConflicting(OutgoingDiagConnection const& connection)
{
    if (nullptr == fpSender)
    { // request has not been sent --> no conflicts
        return false;
    }
    if (fSourceId != connection.fSourceId)
    { // no conflict if sourceIds differ
        return false;
    }
    if ((!TransportConfiguration::isFunctionalAddress(fTargetId))
        && (!TransportConfiguration::isFunctionalAddress(connection.fTargetId))
        && (fTargetId != connection.fTargetId))
    { // there are no broadcasts and targetIds differ
        return false;
    }
    if (fServiceId != connection.fServiceId)
    { // no conflict if serviceIds differ
        return false;
    }
    if (fServiceId == connection.fServiceId)
    {
        if ((fAcceptNegativeResponse != connection.fAcceptNegativeResponse)
            && ((fMatchingResponseBytes > 0U) || (connection.fMatchingResponseBytes > 0U)))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    uint16_t const bytesToCheck = (fMatchingResponseBytes < connection.fMatchingResponseBytes)
                                      ? fMatchingResponseBytes
                                      : connection.fMatchingResponseBytes;
    for (uint16_t i = 1U; i <= bytesToCheck; ++i)
    {
        if ((*fpRequestMessage)[i] != (*connection.fpRequestMessage)[i])
        {
            return false;
        }
    }
    return true;
}

// METRIC STCYC 11 // The function is in use as is
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
            && (fAcceptNegativeResponse))
        {
            if (fServiceId == response[1])
            {
                return 1U;
            }
        }
        return 0U;
    }
    // now check positive response
    if ((fServiceId + DiagReturnCode::POSITIVE_RESPONSE_OFFSET) == response[0])
    {
        ++matchingBytes;
    }
    else
    { // serviceId does not match
        return 0U;
    }
    if (fMatchingResponseBytes > (response.getPayloadLength() - 1U)) //-1 because of serviceId
    {                                                                // we cannot check enough bytes
        return 0U;
    }
    if (fMatchingResponseBytes > 0U)
    {
        // loop starts with 1 because serviceId has special treatment above
        for (uint16_t i = 1U; i <= fMatchingResponseBytes; ++i)
        {
            if (fpRequestMessage->getPayload()[i] == response[i])
            {
                ++matchingBytes;
            }
        }
        if (matchingBytes <= fMatchingResponseBytes)
        { // not all bytes matched --> not for us!
            return 0U;
        }
    }
    return matchingBytes;
}

::uds::ErrorCode OutgoingDiagConnection::setTimeout(uint32_t const newTimeout)
{
    if (!fOpen)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }
    if (fInfiniteTimeout)
    {
        return ::uds::ErrorCode::OK;
    }
    if (newTimeout == RESPONSE_PENDING_TIMEOUT)
    {
        fResponsePendingTimeout._asyncTimeout.cancel();
        ::async::schedule(
            fContext,
            fResponsePendingTimeout,
            fResponsePendingTimeout._asyncTimeout,
            newTimeout,
            ::async::TimeUnit::MILLISECONDS);
        fResponsePendingTimeout._isActive = true;
    }
    else
    {
        fResponseTimeout._asyncTimeout.cancel();
        ::async::schedule(
            fContext,
            fResponseTimeout,
            fResponseTimeout._asyncTimeout,
            newTimeout,
            ::async::TimeUnit::MILLISECONDS);
        fResponseTimeout._isActive = true;
    }
    return ::uds::ErrorCode::OK;
}

void OutgoingDiagConnection::getRequestBuffer(uint8_t*& pBuffer, uint16_t& length)
{
    if (nullptr == fpRequestMessage)
    {
        Logger::critical(
            UDS, "OutgoingDiagConnection::getRequesteBuffer(): fpResponseMessage is NULL!");
        estd_assert(fpRequestMessage);
    }
    pBuffer = fpRequestMessage->getPayload();
    length  = fpRequestMessage->getMaxPayloadLength();
}

void OutgoingDiagConnection::transportMessageProcessed(
    transport::TransportMessage& transportMessage, ProcessingResult const result)
{
    if (fpSender != nullptr)
    {
        fSendInProgress = false;
        if (ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR == result)
        {
            fpSender->requestSent(*this, AbstractDiagApplication::REQUEST_SENT);
        }
        else
        {
            Logger::warn(
                UDS,
                "OutgoingDiagConnection: sending request 0x%x --> 0x%x failed",
                transportMessage.getSourceId(),
                transportMessage.getTargetId());
            fpSender->requestSent(*this, AbstractDiagApplication::REQUEST_SEND_FAILED);
        }
    }
    else
    {
        Logger::warn(
            UDS,
            "OutgoingDiagConnection::transportMessageProcessed() called with fpSender == NULL");
    }
}

void OutgoingDiagConnection::execute(::async::RunnableType const& /* timeout */)
{
    if (fpSender != nullptr)
    {
        if ((!fResponseTimeout._isActive) && (!fResponsePendingTimeout._isActive))
        {
            fKeepAlive = false;
            fpSender->responseTimeout(*this);
            if (!fKeepAlive)
            {
                timeoutOccurred();
            }
            else
            {
                fKeepAlive = false;
            }
        }
    }
    else
    {
        Logger::warn(UDS, "OutgoingDiagConnection::timeout() called with fpSender == NULL");
    }
}

} // namespace uds
