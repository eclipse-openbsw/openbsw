// Copyright 2025 Accenture.

#include "doip/server/DoIpServerTransportMessageHandler.h"

#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpLock.h"
#include "doip/common/IDoIpTransportMessageProvidingListener.h"
#include "doip/server/DoIpServerLogger.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"
#include "doip/server/IDoIpServerConnection.h"

#include <transport/ITransportMessageProcessedListener.h>
#include <transport/TransportMessage.h>

#include <estd/big_endian.h>
#include <estd/memory.h>

#include <algorithm>

namespace doip
{
using ::transport::ITransportMessageListener;
using ::transport::ITransportMessageProcessedListener;
using ::transport::ITransportMessageProvider;
using ::transport::ITransportMessageProvidingListener;
using ::transport::TransportMessage;
using ::util::logger::DOIP;
using ::util::logger::Logger;

DoIpServerTransportMessageHandler::DoIpServerTransportMessageHandler(
    DoIpConstants::ProtocolVersion const protocolVersion,
    ::util::estd::block_pool& diagnosticSendJobBlockPool,
    ::util::estd::block_pool& protocolSendJobBlockPool,
    DoIpServerTransportConnectionConfig const& config)
: IDoIpServerMessageHandler()
, _payloadPeekContext()
, _connection(nullptr)
, _diagnosticSendJobPool(diagnosticSendJobBlockPool)
, _protocolSendJobPool(protocolSendJobBlockPool)
, _transportMessage(nullptr)
, _config(config)
, _receiveMessagePayloadLength(0U)
, _protocolVersion(protocolVersion)
, _isRoutingActive(false)
, _readBuffer()
{}

bool DoIpServerTransportMessageHandler::send(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (_connection != nullptr)
    {
        DoIpTransportMessageSendJob* const job
            = allocateJob(transportMessage, pNotificationListener);
        if (job != nullptr)
        {
            if (_connection->sendMessage(*job))
            {
                return true;
            }
            releaseSendJob(_diagnosticSendJobPool, *job);
        }
    }
    return false;
}

void DoIpServerTransportMessageHandler::connectionOpened(IDoIpServerConnection& connection)
{
    _connection = &connection;
}

void DoIpServerTransportMessageHandler::routingActive() { _isRoutingActive = true; }

void DoIpServerTransportMessageHandler::connectionClosed()
{
    Logger::debug(
        DOIP,
        "DoIpServerTransportMessageHandler(%s, 0x%04x)::connectionClosed()",
        ::common::busid::BusIdTraits::getName(_config.getBusId()),
        _connection->getSourceAddress());
    releaseTransportMessage();
}

bool DoIpServerTransportMessageHandler::headerReceived(DoIpHeader const& header)
{
    if (_connection == nullptr)
    {
        return false;
    }

    if (header.payloadType == DoIpConstants::PayloadTypes::DIAGNOSTIC_MESSAGE)
    {
        uint32_t const payloadLength = header.payloadLength;
        if (payloadLength < 4U)
        {
            _connection->sendNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH, true);
        }
        else if (payloadLength > _config.getParameters().getMaxPayloadLength())
        {
            _connection->sendNack(DoIpConstants::NackCodes::NACK_MESSAGE_TOO_LARGE, false);
        }
        else
        {
            _receiveMessagePayloadLength = static_cast<uint16_t>(payloadLength - 4U);
            return _connection->receivePayload(
                _readBuffer,
                IDoIpConnection::PayloadReceivedCallbackType ::create<
                    DoIpServerTransportMessageHandler,
                    &DoIpServerTransportMessageHandler::
                        diagnosticMessageLogicalAddressInfoReceived>(*this));
        }
        _connection->endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
        return true;
    }
    return false;
}

DoIpTransportMessageSendJob* DoIpServerTransportMessageHandler::allocateJob(
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const pNotificationListener)
{
    // RAII lock not unused
    DoIpLock const lock;

    auto const sourceAddress = transportMessage.sourceAddress();
    auto const targetAddress = _connection->getSourceAddress();
    if (_diagnosticSendJobPool.empty())
    {
        Logger::warn(
            DOIP,
            "DoIpServerTransportMessageHandler::send(0x%04x -> 0x%04x): diagnostic send "
            "job pool depleted",
            sourceAddress,
            targetAddress);
        return nullptr;
    }
    return &_diagnosticSendJobPool.allocate<DoIpTransportMessageSendJob>().construct(
        _protocolVersion,
        ::estd::by_ref(transportMessage),
        pNotificationListener,
        sourceAddress,
        targetAddress,
        ::estd::by_ref(static_cast<IDoIpSendJobCallback<DoIpTransportMessageSendJob>&>(*this)));
}

void DoIpServerTransportMessageHandler::releaseSendJob(
    DoIpTransportMessageSendJob& sendJob, bool const success)
{
    auto const* const busIdName = ::common::busid::BusIdTraits::getName(_config.getBusId());
    auto const sourceAddress    = sendJob.getSourceAddress();
    if (success)
    {
        Logger::debug(
            DOIP,
            "DoIpServerTransportMessageHandler(%s, 0x%04x)::sendTransportMessageProcessed(): Sent "
            "successfully.",
            busIdName,
            sourceAddress);
    }
    else
    {
        Logger::warn(
            DOIP,
            "DoIpServerTransportMessageHandler(%s, 0x%04x)::sendTransportMessageProcessed(): "
            "Failed to send.",
            busIdName,
            sourceAddress);
    }
    sendJob.sendTransportMessageProcessed(success);
    releaseSendJob(_diagnosticSendJobPool, sendJob);
}

void DoIpServerTransportMessageHandler::diagnosticMessageLogicalAddressInfoReceived(
    ::estd::slice<uint8_t const> const logicalAddressInfo)
{
    _payloadPeekContext.sourceAddress
        = logicalAddressInfo.reinterpret_as<::estd::be_uint16_t const>()[0];
    _payloadPeekContext.targetAddress
        = logicalAddressInfo.reinterpret_as<::estd::be_uint16_t const>()[1];
    auto const payloadPrefixSize = std::min(
        static_cast<size_t>(_receiveMessagePayloadLength), size_t(PAYLOAD_PREFIX_BUFFER_SIZE));
    (void)_connection->receivePayload(
        ::estd::make_slice(_payloadPeekContext.payloadPrefixBuffer).subslice(payloadPrefixSize),
        IDoIpConnection::PayloadReceivedCallbackType::create<
            DoIpServerTransportMessageHandler,
            &DoIpServerTransportMessageHandler::diagnosticMessagePayloadPrefixDataReceived>(*this));
}

IDoIpTransportMessageProvidingListener::GetResult
DoIpServerTransportMessageHandler::getTpMessageAndReceiveDiagnosticUserData(
    uint16_t const /* sourceAddress */,
    uint16_t const /* targetAddress */,
    ::estd::slice<uint8_t const> const payloadPrefix)
{
    auto const peekSlice
        = payloadPrefix.subslice(std::min(payloadPrefix.size(), size_t(PEEK_MAX_SIZE)));
    auto const getResult = _config.getMessageProvidingListener().getTransportMessage(
        _config.getBusId(),
        _connection->getInternalSourceAddress(),
        _payloadPeekContext.targetAddress,
        _receiveMessagePayloadLength,
        peekSlice,
        _transportMessage);
    auto const tpErrorCode = getResult.getResult();
    Logger::debug(
        DOIP,
        "DoIpServerTransportMessageHandler::getTransportMessage(%s)(0x%04x -> 0x%04x), "
        "returned %x)",
        ::common::busid::BusIdTraits::getName(_config.getBusId()),
        _payloadPeekContext.sourceAddress,
        _payloadPeekContext.targetAddress,
        tpErrorCode);
    if (tpErrorCode == ITransportMessageProvider::ErrorCode::TPMSG_OK)
    {
        _transportMessage->resetValidBytes();
        _transportMessage->setSourceAddress(_connection->getInternalSourceAddress());
        _transportMessage->setTargetAddress(_payloadPeekContext.targetAddress);
        _transportMessage->setPayloadLength(_receiveMessagePayloadLength);
        (void)::estd::memory::copy(
            ::estd::slice<uint8_t>::from_pointer(
                _transportMessage->getBuffer(), _transportMessage->getBufferLength()),
            payloadPrefix);
        (void)_transportMessage->increaseValidBytes(payloadPrefix.size());
        if (payloadPrefix.size() < _receiveMessagePayloadLength)
        {
            auto const remainingPayload = ::estd::slice<uint8_t>::from_pointer(
                                              _transportMessage->getBuffer(),
                                              static_cast<size_t>(_receiveMessagePayloadLength))
                                              .offset(payloadPrefix.size());
            (void)_connection->receivePayload(
                remainingPayload,
                IDoIpConnection::PayloadReceivedCallbackType ::create<
                    DoIpServerTransportMessageHandler,
                    &DoIpServerTransportMessageHandler::diagnosticMessageUserDataReceived>(*this));
        }
        else
        {
            // all payload data has been received as part of the prefix already
            diagnosticMessageUserDataReceived({});
        }
    }

    return getResult;
}

void DoIpServerTransportMessageHandler::diagnosticMessagePayloadPrefixDataReceived(
    ::estd::slice<uint8_t const> const payloadPrefix)
{
    if ((!_isRoutingActive)
        || (_connection->getSourceAddress() != _payloadPeekContext.sourceAddress))
    {
        diagnosticMessageUserDataPrefixReceivedNack(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS,
            true,
            payloadPrefix);
        return;
    }

    auto const getResult = getTpMessageAndReceiveDiagnosticUserData(
        _payloadPeekContext.sourceAddress, _payloadPeekContext.targetAddress, payloadPrefix);
    if (getResult.getResult() != ITransportMessageProvider::ErrorCode::TPMSG_OK)
    {
        uint8_t const nackCode = getResult.getNackCode();
        bool const closeAfterSend
            = (getResult.getResult()
               == ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS);
        diagnosticMessageUserDataPrefixReceivedNack(nackCode, closeAfterSend, payloadPrefix);
    }
}

void DoIpServerTransportMessageHandler::diagnosticMessageUserDataPrefixReceivedNack(
    uint8_t const nackCode, bool const closeAfterSend, ::estd::slice<uint8_t const> const payload)
{
    _connection->endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    auto* const job = queueDiagnosticAck(
        DoIpConstants::PayloadTypes::DIAGNOSTIC_MESSAGE_NEGATIVE_ACK,
        _payloadPeekContext.sourceAddress,
        _payloadPeekContext.targetAddress,
        nackCode,
        closeAfterSend,
        payload);
    if (job == nullptr)
    {
        Logger::warn(
            DOIP,
            "DoIpServerTransportMessageHandler(%s)(0x%04x -> 0x%04x): Failed to allocate and queue "
            "diagnostic NACK.",
            ::common::busid::BusIdTraits::getName(_config.getBusId()),
            _payloadPeekContext.sourceAddress,
            _payloadPeekContext.targetAddress);
    }
}

void DoIpServerTransportMessageHandler::diagnosticMessageUserDataReceived(
    ::estd::slice<uint8_t const> const remainingPayload)
{
    (void)_transportMessage->increaseValidBytes(remainingPayload.size());
    uint16_t const sourceAddress = _connection->getSourceAddress();
    uint16_t const targetAddress = _transportMessage->targetAddress();
    Logger::debug(
        DOIP,
        "DoIpServerTransportMessageHandler(%s)(0x%04x -> 0x%04x): Message received.",
        ::common::busid::BusIdTraits::getName(_config.getBusId()),
        sourceAddress,
        targetAddress);
    // suspend sending until after the result from the messageProvidingListener determines whether
    // an ACK or NACK shall be sent
    _connection->suspendSending();
    // Ensure that the diagnostic ack is sent before any other messages sent by `messageReceived`.
    StaticPayloadSendJobType* const job = queueDiagnosticAck(
        DoIpConstants::PayloadTypes::DIAGNOSTIC_MESSAGE_POSITIVE_ACK,
        sourceAddress,
        targetAddress,
        0x00U,
        false,
        ::estd::slice<uint8_t const>::from_pointer(
            _transportMessage->getBuffer(), _transportMessage->validBytes()));
    if (job == nullptr)
    {
        Logger::warn(
            DOIP,
            "DoIpServerTransportMessageHandler(%s)(0x%04x -> 0x%04x): Failed to allocate and queue "
            "diagnostic ACK.",
            ::common::busid::BusIdTraits::getName(_config.getBusId()),
            sourceAddress,
            targetAddress);
        // if the diag ack job could not be queued, we will just discard the diagnostic message
        // instead of attempting to process it
        releaseTransportMessage();
    }
    else
    {
        IDoIpTransportMessageProvidingListener::ReceiveResult const result
            = _config.getMessageProvidingListener().messageReceived(
                _config.getBusId(), *_transportMessage, &_config.getMessageProcessedListener());
        if (result.getResult() == ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)
        {
            // passed to the messageProvidingListener, so we drop our reference to it
            _transportMessage = nullptr;
        }
        else
        {
            job->setPayloadType(DoIpConstants::PayloadTypes::DIAGNOSTIC_MESSAGE_NEGATIVE_ACK);
            job->accessPayloadBuffer()[4U] = result.getNackCode();
            releaseTransportMessage();
        }
    }
    _connection->resumeSending();
    _connection->endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

DoIpServerTransportMessageHandler::StaticPayloadSendJobType*
DoIpServerTransportMessageHandler::queueDiagnosticAck(
    uint16_t const payloadType,
    uint16_t const sourceAddress,
    uint16_t const targetAddress,
    uint8_t const responseCode,
    bool const closeAfterSend,
    estd::slice<uint8_t const> const receivedMessageData)
{
    StaticPayloadSendJobType* job = nullptr;
    estd::slice<uint8_t const> receivedMessageDataPrefix;
    {
        // RAII mutex
        DoIpLock const lock;
        if (!_protocolSendJobPool.empty())
        {
            receivedMessageDataPrefix = receivedMessageData.subslice(
                std::min(receivedMessageData.size(), static_cast<size_t>(ACK_PAYLOAD_SIZE)));
            job = &_protocolSendJobPool.allocate<StaticPayloadSendJobType>().construct(
                static_cast<uint8_t>(_protocolVersion),
                payloadType,
                5U + receivedMessageDataPrefix.size(),
                closeAfterSend ? StaticPayloadSendJobType::ReleaseCallbackType::create<
                    DoIpServerTransportMessageHandler,
                    &DoIpServerTransportMessageHandler::releaseSendJobAndClose>(*this)
                               : StaticPayloadSendJobType::ReleaseCallbackType::create<
                                   DoIpServerTransportMessageHandler,
                                   &DoIpServerTransportMessageHandler::releaseSendJob>(*this));
        }
    }
    if (job != nullptr)
    {
        ::estd::slice<uint8_t> payloadBuffer                     = job->accessPayloadBuffer();
        ::estd::memory::take<::estd::be_uint16_t>(payloadBuffer) = targetAddress;
        ::estd::memory::take<::estd::be_uint16_t>(payloadBuffer) = sourceAddress;
        ::estd::memory::take<uint8_t>(payloadBuffer)             = responseCode;
        if (receivedMessageDataPrefix.size() > 0)
        {
            (void)::estd::memory::copy(payloadBuffer, receivedMessageDataPrefix);
        }
        if (!_connection->sendMessage(*job))
        {
            releaseSendJob(_protocolSendJobPool, *job);
            job = nullptr;
        }
    }
    return job;
}

void DoIpServerTransportMessageHandler::releaseSendJobAndClose(
    IDoIpSendJob& sendJob, bool const /*success*/)
{
    Logger::debug(
        DOIP,
        "DoIpServerTransportMessageHandler(%s, 0x%04x)::releaseSendJobAndClose()",
        ::common::busid::BusIdTraits::getName(_config.getBusId()),
        _connection->getSourceAddress());
    releaseSendJob(_protocolSendJobPool, sendJob);
    _connection->close();
}

void DoIpServerTransportMessageHandler::releaseSendJob(
    IDoIpSendJob& sendJob, bool const /*success*/)
{
    releaseSendJob(_protocolSendJobPool, sendJob);
}

void DoIpServerTransportMessageHandler::releaseTransportMessage()
{
    if (_transportMessage != nullptr)
    {
        _config.getMessageProvidingListener().releaseTransportMessage(*_transportMessage);
        _transportMessage = nullptr;
    }
}

void DoIpServerTransportMessageHandler::releaseSendJob(
    ::util::estd::derived_object_pool<IDoIpSendJob>& pool, IDoIpSendJob& sendJob)
{
    // RAII lock not unused
    DoIpLock const lock;
    pool.release(sendJob);
}

} // namespace doip
