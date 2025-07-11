// Copyright 2024 Accenture.

#include "uds/connection/IncomingDiagConnection.h"

#include "platform/config.h"
#include "transport/AbstractTransportLayer.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "uds/UdsLogger.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/DiagConnectionManager.h"
#include "uds/connection/NestedDiagRequest.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/session/IDiagSessionManager.h"
#include "util/estd/assert.h"

#include <async/Async.h>
#include <etl/span.h>

using ::transport::AbstractTransportLayer;
using ::transport::TransportConfiguration;
using ::transport::TransportMessage;
using ::util::logger::Logger;
using ::util::logger::UDS;

namespace uds
{

void IncomingDiagConnection::addIdentifier()
{
    estd_assert(!fIsResponseActive);
    if (fNestedRequest != nullptr)
    {
        fNestedRequest->addIdentifier();
    }
    else if ((nullptr != fpRequestMessage) && (fIdentifiers.size() < fIdentifiers.max_size()))
    {
        fIdentifiers.push_back(fpRequestMessage->getPayload()[fIdentifiers.size()]);
    }
    else
    {
        // invalid index
    }
}

uint16_t IncomingDiagConnection::getNumIdentifiers() const { return fIdentifiers.size(); }

uint8_t IncomingDiagConnection::getIdentifier(uint16_t const idx) const
{
    if (fNestedRequest != nullptr)
    {
        return fNestedRequest->getIdentifier(idx);
    }
    if (idx >= fIdentifiers.size())
    {
        Logger::error(UDS, "IncomingDiagConnection::addIdentifier(): invalid index %d!", idx);
        return 0U;
    }

    return fIdentifiers[idx];
}

::uds::ErrorCode IncomingDiagConnection::sendPositiveResponse(AbstractDiagJob& sender)
{
    if (!fIsResponseActive)
    {
        releaseRequestGetResponse();
    }
    return sendPositiveResponseInternal(fPositiveResponse.getLength(), sender);
}

::uds::ErrorCode
IncomingDiagConnection::sendPositiveResponseInternal(uint16_t const length, AbstractDiagJob& sender)
{
    if (!fOpen)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }
    if (nullptr == fpMessageSender)
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
    if (nullptr != fpSender)
    {
        Logger::error(UDS, "IncomingDiagConnection::sendPositiveResponse(): BUSY!");
        return ::uds::ErrorCode::CONNECTION_BUSY;
    }
    if (nullptr != fpResponseMessage)
    {
        ++fNumPendingMessageProcessedCallbacks;

        fSendPositiveResponseClosure = SendPositiveResponseClosure::CallType(
            SendPositiveResponseClosure::CallType::delegate_type::
                create<IncomingDiagConnection, &IncomingDiagConnection::asyncSendPositiveResponse>(
                    *this),
            length,
            &sender);
        ::async::execute(fContext, fSendPositiveResponseClosure);
        return ::uds::ErrorCode::OK;
    }
    else
    {
        return ::uds::ErrorCode::NO_TP_MESSAGE;
    }
}

void IncomingDiagConnection::asyncSendPositiveResponse(
    uint16_t const length, AbstractDiagJob* const pSender)
{
    if ((pSender != nullptr) && (fNestedRequest != nullptr))
    {
        --fNumPendingMessageProcessedCallbacks;
        fNestedRequest->setNestedResponseLength(length);
        if (fPositiveResponse.isOverflow())
        {
            fNestedRequest->handleResponseOverflow();
        }
        pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        return;
    }
    // invert source and target because we are a incoming connection
    uint16_t const tmpSourceId = fSourceId;
    if (nullptr != fpResponseMessage)
    {
        setSourceId(*fpResponseMessage);
        fpResponseMessage->setTargetId(tmpSourceId);
        if (DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER == fpResponseMessage->getServiceId())
        { // we did send a negative response before --> restore payload
            for (uint8_t i = 0U; i < DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH; ++i)
            {
                fpResponseMessage->getPayload()[i] = fNegativeResponseTempBuffer[i];
            }
        }
        fpResponseMessage->resetValidBytes();
        for (uint8_t i = 0U; i < fIdentifiers.size(); ++i)
        {
            (void)fpResponseMessage->append(fIdentifiers[i]);
        }
        fpResponseMessage->setServiceId(fServiceId + DiagReturnCode::POSITIVE_RESPONSE_OFFSET);
        (void)fpResponseMessage->increaseValidBytes(length);
        fpResponseMessage->setPayloadLength(fIdentifiers.size() + length);
        fpSender = pSender;
        fpDiagSessionManager->responseSent(
            *this, DiagReturnCode::OK, &((*fpResponseMessage)[fIdentifiers.size()]), length);
    }
    if ((!fSuppressPositiveResponse) || fResponsePendingSent)
    { // this is not a positive response to a suppressed request --> send
        if (fResponsePendingIsBeingSent)
        {
            // we will send that later after the pending has been sent
        }
        else
        {
            (void)sendResponse();
        }
    }
    else
    { // ignore response as it is suppressed
        --fNumPendingMessageProcessedCallbacks;
        if (fpSender != nullptr)
        {
            AbstractDiagJob* const tmpSender = fpSender;
            fpSender                         = nullptr;
            tmpSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
    }
}

void IncomingDiagConnection::disableResponseTimeout()
{
    fResponsePendingTimeout._asyncTimeout.cancel();
}

void IncomingDiagConnection::disableGlobalTimeout()
{
    fGlobalPendingTimeout._asyncTimeout.cancel();
}

DiagReturnCode::Type IncomingDiagConnection::startNestedRequest(
    AbstractDiagJob& sender,
    NestedDiagRequest& nestedRequest,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    estd_assert(fNestedRequest == nullptr);
    (void)releaseRequestGetResponse();
    estd_assert(requestLength < fpResponseMessage->getBufferLength());
    fIdentifiers.resize(nestedRequest.getPrefixLength());
    nestedRequest.init(
        sender,
        ::etl::span<uint8_t>(
            fpResponseMessage->getBuffer() + fIdentifiers.size(),
            fpResponseMessage->getBufferLength() - fIdentifiers.size()),
        ::etl::span<uint8_t const>(request, static_cast<size_t>(requestLength)));
    fNestedRequest = &nestedRequest;
    triggerNextNestedRequest();
    return DiagReturnCode::OK;
}

::uds::ErrorCode IncomingDiagConnection::sendResponse()
{
    AbstractTransportLayer::ErrorCode const sendResult
        = fpMessageSender->send(*fpResponseMessage, this);
    if (sendResult == AbstractTransportLayer::ErrorCode::TP_OK)
    {
        return ::uds::ErrorCode::OK;
    }
    else
    {
        --fNumPendingMessageProcessedCallbacks;
        if (fpSender != nullptr)
        {
            AbstractDiagJob* const pSender = fpSender;
            fpSender                       = nullptr;
            pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SEND_FAILED);
        }
        if (fConnectionTerminationIsPending && (0U == fNumPendingMessageProcessedCallbacks))
        {
            terminate();
        }
        return ::uds::ErrorCode::SEND_FAILED;
    }
}

bool IncomingDiagConnection::terminateNestedRequest()
{
    if (fNestedRequest != nullptr)
    {
        if (fNestedRequest->getResponseCode() == DiagReturnCode::OK)
        {
            ::async::execute(fContext, fTriggerNextNestedRequestDelegate);
        }
        else
        {
            endNestedRequest();
        }
        return false;
    }
    else
    {
        return true;
    }
}

::uds::ErrorCode
IncomingDiagConnection::sendNegativeResponse(uint8_t const responseCode, AbstractDiagJob& sender)
{
    if (!fOpen)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }

    if (nullptr == fpMessageSender)
    {
        Logger::error(
            UDS,
            "IncomingDiagConnection::sendNegativeResponse(): "
            "fpMessageSender is NULL!");
        return ::uds::ErrorCode::SEND_FAILED;
    }
    if (!fIsResponseActive)
    {
        (void)releaseRequestGetResponse();
    }
    if (fpResponseMessage != nullptr)
    {
        ++fNumPendingMessageProcessedCallbacks;

        fSendNegativeResponseClosure = SendNegativeResponseClosure::CallType(
            SendNegativeResponseClosure::CallType::delegate_type::
                create<IncomingDiagConnection, &IncomingDiagConnection::asyncSendNegativeResponse>(
                    *this),
            responseCode,
            &sender);
        ::async::execute(fContext, fSendNegativeResponseClosure);
        return ::uds::ErrorCode::OK;
    }
    else
    {
        return ::uds::ErrorCode::NO_TP_MESSAGE;
    }
}

// METRIC STCYC 12 // The function is already in use as is
void IncomingDiagConnection::asyncSendNegativeResponse(
    uint8_t const responseCode, AbstractDiagJob* const pSender)
{
    // end nested request
    if (fNestedRequest != nullptr)
    {
        --fNumPendingMessageProcessedCallbacks;
        if (responseCode != static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING))
        {
            fNestedRequest->handleNegativeResponseCode(
                static_cast<DiagReturnCode::Type>(responseCode));
            pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
        else if (!fNestedRequest->isPendingSent())
        {
            fNestedRequest->setPendingResponseSender(pSender);
            if (fNumPendingMessageProcessedCallbacks != 0U)
            { // don't send ResponsePending while response is being sent
                fResponsePendingIsPending = true;
            }
            else
            {
                fResponsePendingIsPending = false;
                sendResponsePending();
            }
            restartPendingTimeout();
        }
        else
        {
            pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
        return;
    }
    fpResponseMessage->setPayloadLength(DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH);
    // invert source and target because we are a incoming connection
    setSourceId(*fpResponseMessage);
    fpResponseMessage->setTargetId(fSourceId);
    if (DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER != fpResponseMessage->getServiceId())
    { // make a backup only the first time a negative response is sent!
        for (uint8_t i = 0U; i < DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH; ++i)
        {
            fNegativeResponseTempBuffer[i] = fpResponseMessage->getPayload()[i];
        }
    }
    fpResponseMessage->resetValidBytes();
    (void)fpResponseMessage->append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)fpResponseMessage->append(fServiceId);
    (void)fpResponseMessage->append(responseCode);
    fpSender = pSender;
    if (responseCode != static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING))
    {
        fpDiagSessionManager->responseSent(
            *this,
            static_cast<DiagReturnCode::Type>(responseCode),
            fpResponseMessage->getPayload() + DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH,
            0U);
    }
    if (!((TransportConfiguration::isFunctionalAddress(fTargetId))
          && ((static_cast<uint8_t>(DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED) == responseCode)
              || (static_cast<uint8_t>(DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED)
                  == responseCode)
              || (static_cast<uint8_t>(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE) == responseCode)
              || (static_cast<uint8_t>(
                      DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION)
                  == responseCode)
              || (static_cast<uint8_t>(DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION)
                  == responseCode))))
    { // this is no SNS or SFNS to a functional request --> send response
        if (fResponsePendingIsBeingSent)
        {
            // we will send that later after the pending has been sent
        }
        else
        {
            if (static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING) == responseCode)
            {
                restartPendingTimeout();
            }
            (void)sendResponse();
        }
    }
    else
    { // ignore response as it is suppressed in this case
        --fNumPendingMessageProcessedCallbacks;
        if (fpSender != nullptr)
        {
            AbstractDiagJob* const tmpSender = fpSender;
            fpSender                         = nullptr;
            tmpSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
    }
}

void IncomingDiagConnection::triggerNextNestedRequest()
{
    while ((fNestedRequest->getResponseCode() == DiagReturnCode::OK)
           && (fNestedRequest->prepareNextRequest()))
    {
        fIsResponseActive                       = false;
        DiagReturnCode::Type const responseCode = fNestedRequest->processNextRequest(*this);
        if (responseCode == DiagReturnCode::OK)
        {
            return;
        }
        else
        {
            fNestedRequest->handleNegativeResponseCode(responseCode);
        }
    }
    endNestedRequest();
}

void IncomingDiagConnection::endNestedRequest()
{
    AbstractDiagJob* const sender           = fNestedRequest->getSender();
    uint16_t const length                   = fNestedRequest->getResponseLength();
    DiagReturnCode::Type const responseCode = fNestedRequest->getResponseCode();
    fNestedRequest                          = nullptr;
    if (responseCode == DiagReturnCode::OK)
    {
        (void)sendPositiveResponseInternal(length, *sender);
    }
    else
    {
        (void)sendNegativeResponse(static_cast<uint8_t>(responseCode), *sender);
    }
}

void IncomingDiagConnection::transportMessageProcessed(
    transport::TransportMessage& transportMessage, ProcessingResult const result)
{
    fTransportMessageProcessedClosure = TransportMessageClosure::CallType(
        TransportMessageClosure::CallType::delegate_type::
            create<IncomingDiagConnection, &IncomingDiagConnection::asyncTransportMessageProcessed>(
                *this),
        &transportMessage,
        result);
    ::async::execute(fContext, fTransportMessageProcessedClosure);
}

// METRIC STCYC 11 // The function is already in use as is
void IncomingDiagConnection::asyncTransportMessageProcessed(
    transport::TransportMessage* pTransportMessage, ProcessingResult const status)
{
    --fNumPendingMessageProcessedCallbacks;
    if (pTransportMessage == &fPendingMessage)
    {
        fResponsePendingIsBeingSent = false;
        if (fNestedRequest != nullptr)
        {
            fNestedRequest->setIsPendingSent();
            AbstractDiagJob* const pendingResponseSender
                = fNestedRequest->getPendingResponseSender();
            if (pendingResponseSender != nullptr)
            {
                fNestedRequest->setPendingResponseSender(nullptr);
                pendingResponseSender->responseSent(
                    *this,
                    (status == ProcessingResult::PROCESSED_NO_ERROR)
                        ? AbstractDiagJob::RESPONSE_SENT
                        : AbstractDiagJob::RESPONSE_SEND_FAILED);
            }
            return;
        }
    }
    if (fpSender != nullptr)
    {
        if (pTransportMessage != &fPendingMessage)
        {
            AbstractDiagJob* const pSender = fpSender;
            fpSender                       = nullptr;
            if (status == ProcessingResult::PROCESSED_NO_ERROR)
            {
                pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
            }
            else
            {
                pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SEND_FAILED);
            }
        }
        else
        { // a response is pending and responsePending has been sent
            (void)sendResponse();
        }
    }
    if (fNumPendingMessageProcessedCallbacks == 0U)
    { // all responses have been sent
        if (fConnectionTerminationIsPending)
        {
            terminate();
        }
        else if (fResponsePendingIsPending)
        {
            fResponsePendingIsPending = false;
            sendResponsePending();
        }
        else
        {
            // nothing to do
        }
    }
    if (status != ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)
    {
        Logger::error(
            UDS,
            "IncomingDiagConnection::transportMessageSent(): failed to send "
            "message from 0x%x to 0x%x",
            pTransportMessage->getSourceId(),
            pTransportMessage->getTargetId());
    }
}

uint16_t IncomingDiagConnection::getMaximumResponseLength() const
{
    if (fNestedRequest != nullptr)
    {
        return fNestedRequest->getMaxNestedResponseLength();
    }
    else
    {
        return fpRequestMessage->getMaxPayloadLength() - fIdentifiers.size();
    }
}

PositiveResponse& IncomingDiagConnection::releaseRequestGetResponse()
{
    if (fNestedRequest != nullptr)
    {
        fIsResponseActive                         = true;
        ::etl::span<uint8_t> const responseBuffer = fNestedRequest->getResponseBuffer();
        uint8_t* const data                       = responseBuffer.data();
        fPositiveResponse.init(data, static_cast<uint16_t>(responseBuffer.size()));
    }
    else
    {
        if (nullptr == fpResponseMessage)
        {
            fResponseMessage.init(
                fpRequestMessage->getBuffer(), fpRequestMessage->getBufferLength());
            fpResponseMessage = &fResponseMessage;
        }
        fIsResponseActive = true;
        fPositiveResponse.init(
            &((*fpResponseMessage)[fIdentifiers.size()]),
            fpResponseMessage->getMaxPayloadLength() - fIdentifiers.size());
    }
    return fPositiveResponse;
}

void IncomingDiagConnection::expired(::async::RunnableType const& timeout)
{
    if (&timeout == &fResponsePendingTimeout)
    {
        if (fNumPendingMessageProcessedCallbacks != 0U)
        { // don't send ResponsePending while response is being sent
            fResponsePendingIsPending = true;
        }
        else
        {
            fResponsePendingIsPending = false;
            sendResponsePending();
        }
        restartPendingTimeout();
    }
    else if (&timeout == &fGlobalPendingTimeout)
    {
        fResponsePendingTimeout._asyncTimeout.cancel();
        terminate();
    }
    else
    {
        // nothing to do
    }
}

void IncomingDiagConnection::restartPendingTimeout()
{
    fResponsePendingTimeout._asyncTimeout.cancel();
    if (fPendingActivated)
    {
        ::async::schedule(
            fContext,
            fResponsePendingTimeout,
            fResponsePendingTimeout._asyncTimeout,
            fPendingTimeOut,
            ::async::TimeUnit::MILLISECONDS);
    }
}

void IncomingDiagConnection::sendResponsePending()
{
    if ((!fOpen) || (nullptr == fpMessageSender))
    {
        return;
    }
    fPendingMessage.setTargetId(fSourceId);
    setSourceId(fPendingMessage);
    fPendingMessage.resetValidBytes();
    (void)fPendingMessage.append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)fPendingMessage.append(fServiceId);
    (void)fPendingMessage.append(static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING));
    fPendingMessage.setPayloadLength(PENDING_MESSAGE_PAYLOAD_LENGTH);
    ++fNumPendingMessageProcessedCallbacks;
    // this flag indicates that pending has been sent --> a positive response must follow!
    bool const responsePendingSent        = fResponsePendingSent;
    bool const responsePendingIsBeingSent = fResponsePendingIsBeingSent;
    fResponsePendingSent                  = true;
    fResponsePendingIsBeingSent           = true;
    AbstractTransportLayer::ErrorCode const sendResult
        = fpMessageSender->send(fPendingMessage, this);
    if (sendResult != AbstractTransportLayer::ErrorCode::TP_OK)
    {
        --fNumPendingMessageProcessedCallbacks;
        // this flag indicates that pending has been sent --> a positive response must follow!
        fResponsePendingSent        = responsePendingSent;
        fResponsePendingIsBeingSent = responsePendingIsBeingSent;
        Logger::error(
            UDS,
            "IncomingDiagConnection: Unable to send ResponsePending: "
            "sendResult = %d!",
            sendResult);
    }
}

void IncomingDiagConnection::setSourceId(TransportMessage& transportMessage) const
{
    if (TransportConfiguration::isFunctionalAddress(fTargetId))
    {
        if (nullptr == fpDiagConnectionManager)
        {
            Logger::critical(
                UDS,
                "IncomingDiagConnection::setSourceId(): "
                "fpDiagConnectionManager == nullptr!");
            estd_assert(fpDiagConnectionManager != nullptr);
        }
        transportMessage.setSourceId(fpDiagConnectionManager->getSourceDiagId());
    }
    else
    {
        transportMessage.setSourceId(fTargetId);
    }
}

void IncomingDiagConnection::open(bool const activatePending)
{
    if (!fOpen)
    {
        fOpen                       = true;
        fPendingActivated           = activatePending;
        fSuppressPositiveResponse   = false;
        fResponsePendingSent        = false;
        fResponsePendingIsBeingSent = false;
        fIsResponseActive           = false;
        fIdentifiers.clear();

        fResponsePendingTimeout._asyncTimeout.cancel();
        changeRespPendingTimer(0);
        if (fPendingActivated)
        {
            ::async::schedule(
                fContext,
                fResponsePendingTimeout,
                fResponsePendingTimeout._asyncTimeout,
                INITIAL_PENDING_TIMEOUT_MS,
                ::async::TimeUnit::MILLISECONDS);
            ::async::schedule(
                fContext,
                fGlobalPendingTimeout,
                fGlobalPendingTimeout._asyncTimeout,
                GLOBAL_PENDING_TIMEOUT_MS,
                ::async::TimeUnit::MILLISECONDS);
        }
    }
    else
    {
        Logger::error(UDS, "IncomingDiagConnection::open(): opening already open connection!");
    }
}

void IncomingDiagConnection::terminate()
{
    if (terminateNestedRequest())
    {
        ::async::ModifiableLockType lock;
        if (fOpen)
        {
            Logger::debug(
                UDS,
                "IncomingDiagConnection::terminate(): 0x%x --> 0x%x, service 0x%x",
                fSourceId,
                fTargetId,
                fServiceId);
            if (fNumPendingMessageProcessedCallbacks != 0U)
            {
                fConnectionTerminationIsPending = true;
                return;
            }
            fOpen = false;
            lock.unlock();
            fResponsePendingTimeout._asyncTimeout.cancel();
            fGlobalPendingTimeout._asyncTimeout.cancel();
            if (nullptr == fpDiagConnectionManager)
            {
                Logger::critical(
                    UDS,
                    "IncomingDiagConnection::terminate(): fpDiagConnectionManager == "
                    "nullptr!");
                estd_assert(fpDiagConnectionManager != nullptr);
            }
            fConnectionTerminationIsPending = false;
            fpSender                        = nullptr;
            fpDiagConnectionManager->diagConnectionTerminated(*this);
        }
        else
        {
            // nothing to do
        }
    }
}

} // namespace uds
