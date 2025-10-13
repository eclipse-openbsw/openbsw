// Copyright 2024 Accenture.

#include "uds/connection/IncomingDiagConnection.h"

#include "platform/config.h"
#include "transport/AbstractTransportLayer.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "uds/DiagDispatcher.h"
#include "uds/UdsLogger.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/NestedDiagRequest.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/session/IDiagSessionManager.h"

#include <async/Async.h>

#include <etl/error_handler.h>
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
    ETL_ASSERT(!_isResponseActive, ETL_ERROR_GENERIC("response must not be active"));
    if (_nestedRequest != nullptr)
    {
        _nestedRequest->addIdentifier();
    }
    else if ((nullptr != requestMessage) && (_identifiers.size() < _identifiers.max_size()))
    {
        _identifiers.push_back(requestMessage->getPayload()[_identifiers.size()]);
    }
    else
    {
        // invalid index
    }
}

uint16_t IncomingDiagConnection::getNumIdentifiers() const { return _identifiers.size(); }

uint8_t IncomingDiagConnection::getIdentifier(uint16_t const idx) const
{
    if (_nestedRequest != nullptr)
    {
        return _nestedRequest->getIdentifier(idx);
    }
    if (idx >= _identifiers.size())
    {
        Logger::error(UDS, "IncomingDiagConnection::addIdentifier(): invalid index %d!", idx);
        return 0U;
    }

    return _identifiers[idx];
}

::uds::ErrorCode IncomingDiagConnection::sendPositiveResponse(AbstractDiagJob& sender)
{
    if (!_isResponseActive)
    {
        releaseRequestGetResponse();
    }
    return sendPositiveResponseInternal(_positiveResponse.getLength(), sender);
}

::uds::ErrorCode
IncomingDiagConnection::sendPositiveResponseInternal(uint16_t const length, AbstractDiagJob& sender)
{
    if (!isOpen)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }
    if (nullptr == messageSender)
    {
        return ::uds::ErrorCode::SEND_FAILED;
    }
    if (nullptr != _sender)
    {
        Logger::error(UDS, "IncomingDiagConnection::sendPositiveResponse(): BUSY!");
        return ::uds::ErrorCode::CONNECTION_BUSY;
    }
    if (nullptr == responseMessage)
    {
        return ::uds::ErrorCode::NO_TP_MESSAGE;
    }
    ++_numPendingMessageProcessedCallbacks;

    _sendPositiveResponseClosure = SendPositiveResponseClosure::CallType(
        SendPositiveResponseClosure::CallType::delegate_type::
            create<IncomingDiagConnection, &IncomingDiagConnection::asyncSendPositiveResponse>(
                *this),
        length,
        &sender);
    ::async::execute(_context, _sendPositiveResponseClosure);
    return ::uds::ErrorCode::OK;
}

void IncomingDiagConnection::asyncSendPositiveResponse(
    uint16_t const length, AbstractDiagJob* const pSender)
{
    if ((pSender != nullptr) && (_nestedRequest != nullptr))
    {
        --_numPendingMessageProcessedCallbacks;
        _nestedRequest->setNestedResponseLength(length);
        if (_positiveResponse.isOverflow())
        {
            _nestedRequest->handleResponseOverflow();
        }
        pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        return;
    }
    // invert source and target because we are a incoming connection
    uint16_t const tmpSourceId = sourceAddress;
    if (nullptr != responseMessage)
    {
        responseMessage->setSourceAddress(sourceAddressForResponse());
        responseMessage->setTargetAddress(tmpSourceId);
        if (DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER == responseMessage->getServiceId())
        { // we did send a negative response before --> restore payload
            for (uint8_t i = 0U; i < DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH; ++i)
            {
                responseMessage->getPayload()[i] = _negativeResponseTempBuffer[i];
            }
        }
        responseMessage->resetValidBytes();
        for (uint8_t i = 0U; i < _identifiers.size(); ++i)
        {
            (void)responseMessage->append(_identifiers[i]);
        }
        responseMessage->setServiceId(serviceId + DiagReturnCode::POSITIVE_RESPONSE_OFFSET);
        (void)responseMessage->increaseValidBytes(length);
        responseMessage->setPayloadLength(_identifiers.size() + length);
        _sender = pSender;
        diagSessionManager->responseSent(
            *this, DiagReturnCode::OK, &((*responseMessage)[_identifiers.size()]), length);
    }
    if ((!_suppressPositiveResponse) || _responsePendingSent)
    { // this is not a positive response to a suppressed request --> send
        if (!_responsePendingIsBeingSent)
        {
            (void)sendResponse();
        }
    }
    else
    { // ignore response as it is suppressed
        --_numPendingMessageProcessedCallbacks;
        if (_sender != nullptr)
        {
            AbstractDiagJob* const tmpSender = _sender;
            _sender                          = nullptr;
            tmpSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
    }
}

void IncomingDiagConnection::disableResponseTimeout()
{
    _responsePendingTimeout._asyncTimeout.cancel();
}

void IncomingDiagConnection::disableGlobalTimeout()
{
    _globalPendingTimeout._asyncTimeout.cancel();
}

DiagReturnCode::Type IncomingDiagConnection::startNestedRequest(
    AbstractDiagJob& sender,
    NestedDiagRequest& nestedRequest,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    ETL_ASSERT(
        _nestedRequest == nullptr, ETL_ERROR_GENERIC("a nested request must not run already"));
    (void)releaseRequestGetResponse();
    ETL_ASSERT(
        requestLength < responseMessage->getBufferLength(),
        ETL_ERROR_GENERIC("request length must be smaller than supported maximum"));
    _identifiers.resize(nestedRequest.prefixLength);
    nestedRequest.init(
        sender,
        ::etl::span<uint8_t>(
            responseMessage->getBuffer() + _identifiers.size(),
            responseMessage->getBufferLength() - _identifiers.size()),
        ::etl::span<uint8_t const>(request, static_cast<size_t>(requestLength)));
    _nestedRequest = &nestedRequest;
    triggerNextNestedRequest();
    return DiagReturnCode::OK;
}

::uds::ErrorCode IncomingDiagConnection::sendResponse()
{
    AbstractTransportLayer::ErrorCode const sendResult
        = messageSender->send(*responseMessage, this);
    if (sendResult == AbstractTransportLayer::ErrorCode::TP_OK)
    {
        return ::uds::ErrorCode::OK;
    }
    --_numPendingMessageProcessedCallbacks;
    if (_sender != nullptr)
    {
        AbstractDiagJob* const pSender = _sender;
        _sender                        = nullptr;
        pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SEND_FAILED);
    }
    if (_connectionTerminationIsPending && (0U == _numPendingMessageProcessedCallbacks))
    {
        terminate();
    }
    return ::uds::ErrorCode::SEND_FAILED;
}

bool IncomingDiagConnection::terminateNestedRequest()
{
    if (_nestedRequest == nullptr)
    {
        return true;
    }
    if (_nestedRequest->responseCode == DiagReturnCode::OK)
    {
        ::async::execute(_context, _triggerNextNestedRequestDelegate);
    }
    else
    {
        endNestedRequest();
    }
    return false;
}

::uds::ErrorCode
IncomingDiagConnection::sendNegativeResponse(uint8_t const responseCode, AbstractDiagJob& sender)
{
    if (!isOpen)
    {
        return ::uds::ErrorCode::CONNECTION_NOT_OPEN;
    }

    if (nullptr == messageSender)
    {
        Logger::error(
            UDS,
            "IncomingDiagConnection::sendNegativeResponse(): "
            "messageSender is NULL!");
        return ::uds::ErrorCode::SEND_FAILED;
    }
    if (!_isResponseActive)
    {
        (void)releaseRequestGetResponse();
    }
    if (responseMessage == nullptr)
    {
        return ::uds::ErrorCode::NO_TP_MESSAGE;
    }
    ++_numPendingMessageProcessedCallbacks;

    _sendNegativeResponseClosure = SendNegativeResponseClosure::CallType(
        SendNegativeResponseClosure::CallType::delegate_type::
            create<IncomingDiagConnection, &IncomingDiagConnection::asyncSendNegativeResponse>(
                *this),
        responseCode,
        &sender);
    ::async::execute(_context, _sendNegativeResponseClosure);
    return ::uds::ErrorCode::OK;
}

// METRIC STCYC 12 // The function is already in use as is
void IncomingDiagConnection::asyncSendNegativeResponse(
    uint8_t const responseCode, AbstractDiagJob* const pSender)
{
    // end nested request
    if (_nestedRequest != nullptr)
    {
        --_numPendingMessageProcessedCallbacks;
        if (responseCode != static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING))
        {
            _nestedRequest->handleNegativeResponseCode(
                static_cast<DiagReturnCode::Type>(responseCode));
            pSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
        else if (!_nestedRequest->isPendingSent)
        {
            _nestedRequest->pendingResponseSender = pSender;
            _responsePendingIsPending             = (_numPendingMessageProcessedCallbacks != 0U);

            if (!_responsePendingIsPending)
            { // Only send ResponsePending while response is not being sent
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
    responseMessage->setPayloadLength(DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH);
    // invert source and target because we are a incoming connection
    responseMessage->setSourceAddress(sourceAddressForResponse());
    responseMessage->setTargetAddress(sourceAddress);
    if (DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER != responseMessage->getServiceId())
    { // make a backup only the first time a negative response is sent!
        for (uint8_t i = 0U; i < DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH; ++i)
        {
            _negativeResponseTempBuffer[i] = responseMessage->getPayload()[i];
        }
    }
    responseMessage->resetValidBytes();
    (void)responseMessage->append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)responseMessage->append(serviceId);
    (void)responseMessage->append(responseCode);
    _sender = pSender;
    if (responseCode != static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING))
    {
        diagSessionManager->responseSent(
            *this,
            static_cast<DiagReturnCode::Type>(responseCode),
            responseMessage->getPayload() + DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH,
            0U);
    }
    if (!((TransportConfiguration::isFunctionalAddress(targetAddress))
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
        if (_responsePendingIsBeingSent)
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
        --_numPendingMessageProcessedCallbacks;
        if (_sender != nullptr)
        {
            AbstractDiagJob* const tmpSender = _sender;
            _sender                          = nullptr;
            tmpSender->responseSent(*this, AbstractDiagJob::RESPONSE_SENT);
        }
    }
}

void IncomingDiagConnection::triggerNextNestedRequest()
{
    while ((_nestedRequest->responseCode == DiagReturnCode::OK)
           && (_nestedRequest->prepareNextRequest()))
    {
        _isResponseActive       = false;
        auto const responseCode = _nestedRequest->processNextRequest(*this);
        if (responseCode == DiagReturnCode::OK)
        {
            return;
        }
        _nestedRequest->handleNegativeResponseCode(responseCode);
    }
    endNestedRequest();
}

void IncomingDiagConnection::endNestedRequest()
{
    auto const sender       = _nestedRequest->senderJob;
    auto const length       = _nestedRequest->responseLength();
    auto const responseCode = _nestedRequest->responseCode;
    _nestedRequest          = nullptr;
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
    _transportMessageProcessedClosure = TransportMessageClosure::CallType(
        TransportMessageClosure::CallType::delegate_type::
            create<IncomingDiagConnection, &IncomingDiagConnection::asyncTransportMessageProcessed>(
                *this),
        &transportMessage,
        result);
    ::async::execute(_context, _transportMessageProcessedClosure);
}

// METRIC STCYC 11 // The function is already in use as is
void IncomingDiagConnection::asyncTransportMessageProcessed(
    transport::TransportMessage* pTransportMessage, ProcessingResult const status)
{
    --_numPendingMessageProcessedCallbacks;
    if (pTransportMessage == &_pendingMessage)
    {
        _responsePendingIsBeingSent = false;
        if (_nestedRequest != nullptr)
        {
            _nestedRequest->isPendingSent                = true;
            AbstractDiagJob* const pendingResponseSender = _nestedRequest->pendingResponseSender;
            if (pendingResponseSender != nullptr)
            {
                _nestedRequest->pendingResponseSender = nullptr;
                pendingResponseSender->responseSent(
                    *this,
                    (status == ProcessingResult::PROCESSED_NO_ERROR)
                        ? AbstractDiagJob::RESPONSE_SENT
                        : AbstractDiagJob::RESPONSE_SEND_FAILED);
            }
            return;
        }
    }
    if (_sender != nullptr)
    {
        if (pTransportMessage != &_pendingMessage)
        {
            AbstractDiagJob* const pSender = _sender;
            _sender                        = nullptr;
            pSender->responseSent(
                *this,
                (status == ProcessingResult::PROCESSED_NO_ERROR)
                    ? AbstractDiagJob::RESPONSE_SENT
                    : AbstractDiagJob::RESPONSE_SEND_FAILED);
        }
        else
        { // a response is pending and responsePending has been sent
            (void)sendResponse();
        }
    }
    if (_numPendingMessageProcessedCallbacks == 0U)
    { // all responses have been sent
        if (_connectionTerminationIsPending)
        {
            terminate();
        }
        else if (_responsePendingIsPending)
        {
            _responsePendingIsPending = false;
            sendResponsePending();
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
    if (_nestedRequest != nullptr)
    {
        return _nestedRequest->getMaxNestedResponseLength();
    }
    else
    {
        return requestMessage->getMaxPayloadLength() - _identifiers.size();
    }
}

PositiveResponse& IncomingDiagConnection::releaseRequestGetResponse()
{
    if (_nestedRequest != nullptr)
    {
        _isResponseActive                         = true;
        ::etl::span<uint8_t> const responseBuffer = _nestedRequest->getResponseBuffer();
        uint8_t* const data                       = responseBuffer.data();
        _positiveResponse.init(data, static_cast<uint16_t>(responseBuffer.size()));
    }
    else
    {
        if (nullptr == responseMessage)
        {
            _responseMessage.init(requestMessage->getBuffer(), requestMessage->getBufferLength());
            responseMessage = &_responseMessage;
        }
        _isResponseActive = true;
        _positiveResponse.init(
            &((*responseMessage)[_identifiers.size()]),
            responseMessage->getMaxPayloadLength() - _identifiers.size());
    }
    return _positiveResponse;
}

void IncomingDiagConnection::expired(::async::RunnableType const& timeout)
{
    if (&timeout == &_responsePendingTimeout)
    {
        _responsePendingIsPending = (_numPendingMessageProcessedCallbacks != 0U);
        if (!_responsePendingIsPending)
        {
            // Only send ResponsePending while response is not being sent
            sendResponsePending();
        }
        restartPendingTimeout();
    }
    else if (&timeout == &_globalPendingTimeout)
    {
        _responsePendingTimeout._asyncTimeout.cancel();
        terminate();
    }
}

void IncomingDiagConnection::restartPendingTimeout()
{
    _responsePendingTimeout._asyncTimeout.cancel();
    if (_pendingActivated)
    {
        ::async::schedule(
            _context,
            _responsePendingTimeout,
            _responsePendingTimeout._asyncTimeout,
            _pendingTimeOut,
            ::async::TimeUnit::MILLISECONDS);
    }
}

void IncomingDiagConnection::sendResponsePending()
{
    if ((!isOpen) || (nullptr == messageSender))
    {
        return;
    }
    _pendingMessage.setTargetAddress(sourceAddress);
    _pendingMessage.setSourceAddress(sourceAddressForResponse());
    _pendingMessage.resetValidBytes();
    (void)_pendingMessage.append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
    (void)_pendingMessage.append(serviceId);
    (void)_pendingMessage.append(static_cast<uint8_t>(DiagReturnCode::ISO_RESPONSE_PENDING));
    _pendingMessage.setPayloadLength(PENDING_MESSAGE_PAYLOAD_LENGTH);
    ++_numPendingMessageProcessedCallbacks;
    // this flag indicates that pending has been sent --> a positive response must follow!
    bool const responsePendingSent                     = _responsePendingSent;
    bool const responsePendingIsBeingSent              = _responsePendingIsBeingSent;
    _responsePendingSent                               = true;
    _responsePendingIsBeingSent                        = true;
    AbstractTransportLayer::ErrorCode const sendResult = messageSender->send(_pendingMessage, this);
    if (sendResult != AbstractTransportLayer::ErrorCode::TP_OK)
    {
        --_numPendingMessageProcessedCallbacks;
        // this flag indicates that pending has been sent --> a positive response must follow!
        _responsePendingSent        = responsePendingSent;
        _responsePendingIsBeingSent = responsePendingIsBeingSent;
        Logger::error(
            UDS,
            "IncomingDiagConnection: Unable to send ResponsePending: "
            "sendResult = %d!",
            sendResult);
    }
}

uint16_t IncomingDiagConnection::sourceAddressForResponse() const
{
    if (TransportConfiguration::isFunctionalAddress(targetAddress))
    {
        // FIXME: Above branch should not be necessary: diagDispatcher should also
        // be set for functional requests.
        if (diagDispatcher == nullptr)
        {
            ETL_ASSERT_FAIL(ETL_ERROR_GENERIC("diagDispatcher must not be null"));
        }
    }
    return (TransportConfiguration::isFunctionalAddress(targetAddress))
               ? diagDispatcher->getDispatcherSourceId()
               : targetAddress;
}

void IncomingDiagConnection::open(bool const activatePending)
{
    if (isOpen)
    {
        Logger::error(UDS, "IncomingDiagConnection::open(): opening already open connection!");
        return;
    }
    isOpen                      = true;
    _pendingActivated           = activatePending;
    _suppressPositiveResponse   = false;
    _responsePendingSent        = false;
    _responsePendingIsBeingSent = false;
    _isResponseActive           = false;
    _identifiers.clear();

    _responsePendingTimeout._asyncTimeout.cancel();
    changeRespPendingTimer(0);
    if (_pendingActivated)
    {
        ::async::schedule(
            _context,
            _responsePendingTimeout,
            _responsePendingTimeout._asyncTimeout,
            INITIAL_PENDING_TIMEOUT_MS,
            ::async::TimeUnit::MILLISECONDS);
        ::async::schedule(
            _context,
            _globalPendingTimeout,
            _globalPendingTimeout._asyncTimeout,
            GLOBAL_PENDING_TIMEOUT_MS,
            ::async::TimeUnit::MILLISECONDS);
    }
}

void IncomingDiagConnection::terminate()
{
    if (!terminateNestedRequest())
    {
        return;
    }
    Logger::debug(
        UDS,
        "IncomingDiagConnection::terminate(): 0x%x --> 0x%x, service 0x%x",
        sourceAddress,
        targetAddress,
        serviceId);
    {
        ::async::ModifiableLockType lock;
        if (!isOpen)
        {
            return;
        }
        if (_numPendingMessageProcessedCallbacks != 0U)
        {
            _connectionTerminationIsPending = true;
            return;
        }
        isOpen = false;
    }
    _responsePendingTimeout._asyncTimeout.cancel();
    _globalPendingTimeout._asyncTimeout.cancel();
    if (nullptr == diagDispatcher)
    {
        Logger::critical(
            UDS, "IncomingDiagConnection::terminate(): fpDiagConnectionManager == nullptr!");
        ETL_ASSERT_FAIL(ETL_ERROR_GENERIC("diagnostic connection manager must not be null"));
    }
    _connectionTerminationIsPending = false;
    _sender                         = nullptr;
    diagDispatcher->diagConnectionTerminated(*this);
}

} // namespace uds
