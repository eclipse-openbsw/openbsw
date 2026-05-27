/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerConnectionHandler.h"

#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpLock.h"
#include "doip/common/IDoIpSendJob.h"
#include "doip/server/DoIpServerLogger.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"
#include "doip/server/IDoIpServerConnectionHandlerCallback.h"

#include <async/Async.h>
#include <etl/memory.h>
#include <etl/optional.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>
#include <transport/TransportMessage.h>

namespace doip
{
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.
using ::util::logger::DOIP;
using ::util::logger::Logger;

DoIpServerConnectionHandler::DoIpServerConnectionHandler(
    DoIpConstants::ProtocolVersion const protocolVersion,
    uint8_t const socketGroupId,
    IDoIpTcpConnection& connection,
    ::async::ContextType const context,
    uint16_t const logicalEntityAddress,
    DoIpServerTransportLayerParameters const& parameters)
: IDoIpConnectionHandler()
, _callback(nullptr)
, _connection(connection)
, _parameters(parameters)
, _timerTimeout()
, _logicalEntityAddress(logicalEntityAddress)
, _sourceAddress(::transport::TransportMessage::INVALID_ADDRESS)
, _internalSourceAddress(::transport::TransportMessage::INVALID_ADDRESS)
, _protocolVersion(protocolVersion)
, _context(context)
, _state(State::INACTIVE)
, _socketGroupId(socketGroupId)
, _readBuffer()
, _aliveCheckPending(false)
, _isOrWasRouting(false)
{}

void DoIpServerConnectionHandler::addMessageHandler(IDoIpServerMessageHandler& messageHandler)
{
    _messageHandlers.push_back(messageHandler);
}

bool DoIpServerConnectionHandler::resume(uint16_t const sourceAddress)
{
    if (_callback == nullptr)
    {
        Logger::info(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, 0x%x, %d)::resume(source: 0x%04x)",
            _sourceAddress,
            _state,
            _aliveCheckPending,
            sourceAddress);
        _sourceAddress  = sourceAddress;
        _state          = State::ACTIVE;
        _isOrWasRouting = true;
        return true;
    }
    return false;
}

void DoIpServerConnectionHandler::start(IDoIpServerConnectionHandlerCallback& callback)
{
    if (_callback == nullptr)
    {
        Logger::info(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, 0x%x, %d)::close()",
            _sourceAddress,
            _state,
            _aliveCheckPending);
        _callback = &callback;
        _connection.init(*this);
        sendConnectionOpened();
        if (_state == State::INACTIVE)
        {
            setTimer(static_cast<uint32_t>(_parameters.getInitialInactivityTimeout()));
        }
        else
        {
            auto const localEndpoint  = getLocalEndpoint();
            auto const remoteEndpoint = getRemoteEndpoint();
            IDoIpServerConnectionFilter::RoutingActivationCheckResult const result
                = _callback->checkRoutingActivation(
                    _sourceAddress, 0U, _socketGroupId, localEndpoint, remoteEndpoint, 0U, true);
            _internalSourceAddress = result.resolveInternalSourceAddress(_sourceAddress);
            setTimer(_parameters.getGeneralInactivityTimeout());
            sendRoutingActive();
        }
    }
}

void DoIpServerConnectionHandler::close()
{
    if (_state != State::SHUTDOWN)
    {
        Logger::info(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, 0x%x, %d)::close()",
            _sourceAddress,
            _state,
            _aliveCheckPending);
        _state = State::SHUTDOWN;
        _connection.close();
        for (auto& messageHandler : _messageHandlers)
        {
            messageHandler.connectionClosed();
        }
        (void)_timerTimeout.cancel();
        _callback->connectionClosed(*this);

        // The aliveCheckResponseReceived() callback must be called after the connectionClosed()
        // callback to ensure correct handling of a new connection that uses the same source address
        // as an already established connection (that is being close()d here).
        if (_aliveCheckPending)
        {
            _callback->aliveCheckResponseReceived(*this, false);
        }
    }
}

void DoIpServerConnectionHandler::startAliveCheck()
{
    if (_state == State::ACTIVE)
    {
        Logger::info(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, 0x%x, %d)::startAliveCheck()",
            _sourceAddress,
            _state,
            _aliveCheckPending);
        _aliveCheckPending = true;
        StaticPayloadSendJobType* const job
            = allocateSendJob(DoIpConstants::PayloadTypes::ALIVE_CHECK_REQUEST, 0U, false);
        if (job != nullptr)
        {
            setTimer(static_cast<uint32_t>(_parameters.getAliveCheckTimeout()));
            (void)sendOrReleaseMessage(*job);
        }
        else
        {
            close();
        }
    }
    else if (_callback != nullptr)
    {
        _callback->aliveCheckResponseReceived(*this, false);
    }
    else
    {
        // the connection cannot send a response without being initialized
    }
}

void DoIpServerConnectionHandler::routingActivationCompleted(
    bool const success, uint8_t const responseCode)
{
    if (_state == State::ACTIVATING)
    {
        Logger::info(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, %d, %d)::routingActivationCompleted(%d, 0x%02x)",
            _sourceAddress,
            _state,
            _aliveCheckPending,
            success,
            responseCode);
        sendRoutingActivationResponse(_sourceAddress, responseCode, !success);
        if (success)
        {
            _state          = State::ACTIVE;
            _isOrWasRouting = true;
            setTimer(_parameters.getGeneralInactivityTimeout());
            sendRoutingActive();
        }
    }
}

void DoIpServerConnectionHandler::connectionClosed(bool const closedByRemotePeer)
{
    Logger::info(
        DOIP,
        "DoIpServerConnectionHandler(0x%04x, %d, %d)::connectionClosed(%d)",
        _sourceAddress,
        _state,
        _aliveCheckPending,
        closedByRemotePeer);
    close();
}

IDoIpConnectionHandler::HeaderReceivedContinuation
DoIpServerConnectionHandler::headerReceivedRoutingActivationRequest(DoIpHeader const& header)
{
    if ((header.payloadLength != 7U) && (header.payloadLength != 11U))
    {
        sendNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH, true);
        return HeaderReceivedContinuation{IDoIpConnection::PayloadDiscardedCallbackType()};
    }
    return _connection.receivePayload(
               ::etl::span<uint8_t>(_readBuffer).subspan(0U, header.payloadLength),
               IDoIpConnection::PayloadReceivedCallbackType::create<
                   DoIpServerConnectionHandler,
                   &DoIpServerConnectionHandler::routingActivationRequestReceived>(*this))
               ? HeaderReceivedContinuation{IDoIpConnectionHandler::HandledByThisHandler{}}
               : HeaderReceivedContinuation{IDoIpConnection::PayloadDiscardedCallbackType()};
}

IDoIpConnectionHandler::HeaderReceivedContinuation
DoIpServerConnectionHandler::headerReceivedAliveCheckResponse(DoIpHeader const& header)
{
    if (header.payloadLength != 2U)
    {
        sendNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH, true);
        return HeaderReceivedContinuation{IDoIpConnection::PayloadDiscardedCallbackType()};
    }
    return _connection.receivePayload(
               ::etl::span<uint8_t>(_readBuffer).subspan(0U, 2U),
               IDoIpConnection::PayloadReceivedCallbackType::create<
                   DoIpServerConnectionHandler,
                   &DoIpServerConnectionHandler::aliveCheckResponseReceived>(*this))
               ? HeaderReceivedContinuation{IDoIpConnectionHandler::HandledByThisHandler{}}
               : HeaderReceivedContinuation{IDoIpConnection::PayloadDiscardedCallbackType()};
}

IDoIpConnectionHandler::HeaderReceivedContinuation
DoIpServerConnectionHandler::headerReceivedNegativeAck(DoIpHeader const& /* header */)
{
    // we consume generic header negative ACKs without further processing
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType());
    return HeaderReceivedContinuation{IDoIpConnectionHandler::HandledByThisHandler{}};
}

IDoIpConnectionHandler::HeaderReceivedContinuation
DoIpServerConnectionHandler::headerReceivedDefault(DoIpHeader const& header)
{
    for (auto& messageHandler : _messageHandlers)
    {
        if (messageHandler.headerReceived(header))
        {
            return IDoIpConnectionHandler::HandledByThisHandler{};
        }
    }
    sendNack(DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE, false);
    return HeaderReceivedContinuation{IDoIpConnection::PayloadDiscardedCallbackType()};
}

IDoIpConnectionHandler::HeaderReceivedContinuation
DoIpServerConnectionHandler::headerReceived(DoIpHeader const& header)
{
    if (!checkProtocolVersion(header, static_cast<uint8_t>(_protocolVersion)))
    {
        sendNack(DoIpConstants::NackCodes::NACK_INCORRECT_PATTERN, true);
        return IDoIpConnection::PayloadDiscardedCallbackType();
    }
    if (isRouting())
    {
        setTimer(_parameters.getGeneralInactivityTimeout());
    }
    switch (header.payloadType)
    {
        case DoIpConstants::PayloadTypes::ROUTING_ACTIVATION_REQUEST:
        {
            return headerReceivedRoutingActivationRequest(header);
        }
        case DoIpConstants::PayloadTypes::ALIVE_CHECK_RESPONSE:
        {
            return headerReceivedAliveCheckResponse(header);
        }
        case DoIpConstants::PayloadTypes::NEGATIVE_ACK:
        {
            return headerReceivedNegativeAck(header);
        }
        default:
        {
            return headerReceivedDefault(header);
        }
    }
}

void DoIpServerConnectionHandler::execute() { timerExpired(); }

void DoIpServerConnectionHandler::routingActivationRequestReceived(
    ::etl::span<uint8_t const> payload)
{
    uint16_t const sourceAddress = payload.take<::etl::be_uint16_t const>();
    uint8_t const activationType = payload.take<uint8_t const>();
    payload.advance(4U);
    ::etl::optional<uint32_t> oemField;
    if (payload.size() == 4) // if message contains VM-specific data
    {
        oemField = payload.take<::etl::be_uint32_t const>();
    }
    auto const localEndpoint  = getLocalEndpoint();
    auto const remoteEndpoint = getRemoteEndpoint();
    IDoIpServerConnectionFilter::RoutingActivationCheckResult const result
        = _callback->checkRoutingActivation(
            sourceAddress,
            activationType,
            _socketGroupId,
            localEndpoint,
            remoteEndpoint,
            oemField,
            false);
    if (result.getAction() != IDoIpServerConnectionFilter::Action::CONTINUE)
    {
        sendRoutingActivationResponse(
            sourceAddress,
            result.getResponseCode(),
            result.getAction() == IDoIpServerConnectionFilter::Action::REJECT);
    }
    else if (_state == State::INACTIVE)
    {
        _sourceAddress         = sourceAddress;
        _internalSourceAddress = result.resolveInternalSourceAddress(sourceAddress);
        _state                 = State::ACTIVATING;
        _callback->handleRoutingActivationRequest(*this);
    }
    else if (_sourceAddress == sourceAddress)
    {
        sendRoutingActivationResponse(
            sourceAddress, DoIpConstants::RoutingResponseCodes::ROUTING_SUCCESS, false);
    }
    else
    {
        sendRoutingActivationResponse(
            sourceAddress, DoIpConstants::RoutingResponseCodes::ROUTING_WRONG_SOURCE_ADDRESS, true);
    }
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType());
}

void DoIpServerConnectionHandler::aliveCheckResponseReceived(
    ::etl::span<uint8_t const> const payload)
{
    uint16_t const sourceAddress = ::etl::be_uint16_t(payload.data());

    bool const isExpectedAddress = (sourceAddress == _sourceAddress);
    if (_aliveCheckPending)
    {
        _aliveCheckPending = false;
        _callback->aliveCheckResponseReceived(*this, isExpectedAddress);
    }
    if (isExpectedAddress)
    {
        _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType());
    }
    else
    {
        Logger::info(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, %d, %d)::aliveCheckResponseReceived(): closing",
            _sourceAddress,
            _state,
            _aliveCheckPending);
        close();
    }
}

void DoIpServerConnectionHandler::sendConnectionOpened()
{
    for (auto& messageHandler : _messageHandlers)
    {
        messageHandler.connectionOpened(*this);
    }
}

void DoIpServerConnectionHandler::sendRoutingActive()
{
    Logger::debug(
        DOIP,
        "DoIpServerConnectionHandler(0x%04x, %d, %d)::sendRoutingActive()",
        _sourceAddress,
        _state,
        _aliveCheckPending);
    for (auto& messageHandler : _messageHandlers)
    {
        messageHandler.routingActive();
    }
    _callback->routingActive(*this);
}

void DoIpServerConnectionHandler::setTimer(uint32_t const timeoutInMs)
{
    _timerTimeout.cancel();
    ::async::schedule(_context, *this, _timerTimeout, timeoutInMs, ::async::TimeUnit::MILLISECONDS);
}

void DoIpServerConnectionHandler::timerExpired()
{
    switch (_state)
    {
        case State::INACTIVE:
        {
            Logger::warn(
                DOIP,
                "DoIpServerConnectionHandler(0x%04x, %d, %d)::timerExpired(): No routing request "
                "received within %d ms.",
                _sourceAddress,
                _state,
                _aliveCheckPending,
                _parameters.getInitialInactivityTimeout());
            close();
            break;
        }
        case State::ACTIVE:
        {
            if (_aliveCheckPending)
            {
                Logger::info(
                    DOIP,
                    "DoIpServerConnectionHandler(0x%04x, %d, %d)::timerExpired(): No alive check "
                    "response received within %d ms.",
                    _sourceAddress,
                    _state,
                    _aliveCheckPending,
                    _parameters.getAliveCheckTimeout());
            }
            else
            {
                Logger::warn(
                    DOIP,
                    "DoIpServerConnectionHandler(0x%04x, %d, %d)::timerExpired(): No data received "
                    "for %d seconds.",
                    _sourceAddress,
                    _state,
                    _aliveCheckPending,
                    _parameters.getGeneralInactivityTimeout() / 1000U);
                _connection.setCloseMode(IDoIpTcpConnection::CloseMode::ABORT);
            }
            close();
            break;
        }
        default:
        {
            break;
        }
    }
}

void DoIpServerConnectionHandler::sendRoutingActivationResponse(
    uint16_t const sourceAddress, uint8_t const responseCode, bool const closeAfterSend)
{
    Logger::info(
        DOIP,
        "DoIpServerConnectionHandler(0x%04x, %d, %d)::sendRoutingActivationResponse(0x%04x, "
        "0x%02x, %d",
        _sourceAddress,
        _state,
        _aliveCheckPending,
        sourceAddress,
        responseCode,
        closeAfterSend);
    constexpr size_t ROUTING_ACTIVATION_RESPONSE_PAYLOAD_LENGTH = 9U;
    StaticPayloadSendJobType* const job                         = allocateSendJob(
        DoIpConstants::PayloadTypes::ROUTING_ACTIVATION_RESPONSE,
        ROUTING_ACTIVATION_RESPONSE_PAYLOAD_LENGTH,
        closeAfterSend);
    if (job != nullptr)
    {
        ::etl::span<uint8_t> buffer       = job->accessPayloadBuffer();
        buffer.take<::etl::be_uint16_t>() = sourceAddress;
        buffer.take<::etl::be_uint16_t>() = _logicalEntityAddress;
        buffer.take<uint8_t>()            = responseCode;
        // SAFETY: allocateSendJob returned a ROUTING_ACTIVATION_RESPONSE_PAYLOAD_LENGTH = 9 B
        // buffer. After taking 2 B twice and 1 B once, 4 B remain.
        ::etl::mem_set(buffer.begin(), 4U, static_cast<uint8_t>(0U));
        (void)sendOrReleaseMessage(*job);
    }
    else
    {
        Logger::warn(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, %d, %d)::sendRoutingActivationResponse(): no send "
            "job, closing",
            _sourceAddress,
            _state,
            _aliveCheckPending);
        close();
    }
}

void DoIpServerConnectionHandler::sendNack(uint8_t const nackCode, bool const closeAfterSend)
{
    Logger::warn(
        DOIP,
        "DoIpServerConnectionHandler(0x%04x, %d, %d)::sendNack(nackCode: 0x%02x, closeAfterSend: "
        "%d)",
        _sourceAddress,
        _state,
        _aliveCheckPending,
        nackCode,
        closeAfterSend);
    StaticPayloadSendJobType* const job
        = allocateSendJob(DoIpConstants::PayloadTypes::NEGATIVE_ACK, 1U, closeAfterSend);
    if (job != nullptr)
    {
        job->accessPayloadBuffer()[0] = nackCode;
        (void)sendOrReleaseMessage(*job);
    }
    else
    {
        Logger::warn(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, %d, %d)::sendNack(): no send job, closing",
            _sourceAddress,
            _state,
            _aliveCheckPending);
        close();
    }
}

bool DoIpServerConnectionHandler::sendOrReleaseMessage(DoIpStaticPayloadSendJob& job)
{
    if (!_connection.sendMessage(job))
    {
        Logger::warn(
            DOIP,
            "DoIpServerConnectionHandler(0x%04x, %d, %d)::sendOrReleaseMessage(): send failed, "
            "release %p",
            _sourceAddress,
            _state,
            _aliveCheckPending,
            &job);
        releaseSendJob(job, false);
        return false;
    }
    return true;
}

DoIpServerConnectionHandler::StaticPayloadSendJobType* DoIpServerConnectionHandler::allocateSendJob(
    uint16_t const payloadType, uint16_t const payloadLength, bool const closeAfterSend)
{
    if (_state == State::SHUTDOWN)
    {
        return nullptr;
    }

    if (closeAfterSend)
    {
        _state = State::CLOSING;
    }

    // RAII mutex
    DoIpLock const lock;
    if (_sendJobPool.full())
    {
        return nullptr;
    }

    return _sendJobPool.create(
        static_cast<uint8_t>(_protocolVersion),
        payloadType,
        payloadLength,
        closeAfterSend
            ? DoIpStaticPayloadSendJob::ReleaseCallbackType::create<
                DoIpServerConnectionHandler,
                &DoIpServerConnectionHandler::releaseSendJobAndClose>(*this)
            : DoIpStaticPayloadSendJob::ReleaseCallbackType::
                create<DoIpServerConnectionHandler, &DoIpServerConnectionHandler::releaseSendJob>(
                    *this));
}

void DoIpServerConnectionHandler::releaseSendJobAndClose(
    DoIpStaticPayloadSendJob& job, bool const success)
{
    Logger::warn(
        DOIP,
        "DoIpServerConnectionHandler(0x%04x, %d, %d)::releaseSendJobAndClose(%p)",
        _sourceAddress,
        _state,
        _aliveCheckPending,
        &job);
    releaseSendJob(job, success);
    close();
}

void DoIpServerConnectionHandler::releaseSendJob(
    DoIpStaticPayloadSendJob& job, bool const /*success*/)
{
    // RAII mutex
    DoIpLock const lock;
    _sendJobPool.destroy(&job);
}

void DoIpServerConnectionHandler::suspendSending() { return _connection.suspendSending(); }

void DoIpServerConnectionHandler::resumeSending() { _connection.resumeSending(); }

void DoIpServerConnectionHandler::shutdown() { (void)_timerTimeout.cancel(); }

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace doip
