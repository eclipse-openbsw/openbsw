/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpTransportMessageSendJob.h"

#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpSendJobHelper.h"

#include <transport/ITransportMessageProcessedListener.h>
#include <transport/TransportMessage.h>

#include <etl/unaligned_type.h>

namespace doip
{
using ::transport::ITransportMessageProcessedListener;
using ::transport::TransportMessage;

DoIpTransportMessageSendJob::DoIpTransportMessageSendJob(
    DoIpConstants::ProtocolVersion const protocolVersion,
    TransportMessage& message,
    ITransportMessageProcessedListener* const notificationListener,
    CallbackType& callback)
: _callback(callback)
, _message(message)
, _notificationListener(notificationListener)
, _sourceAddress(message.sourceAddress())
, _targetAddress(message.targetAddress())
, _protocolVersion(protocolVersion)
{}

DoIpTransportMessageSendJob::DoIpTransportMessageSendJob(
    DoIpConstants::ProtocolVersion const protocolVersion,
    ::transport::TransportMessage& message,
    ::transport::ITransportMessageProcessedListener* const notificationListener,
    uint16_t const sourceAddress,
    uint16_t const targetAddress,
    CallbackType& callback)
: _callback(callback)
, _message(message)
, _notificationListener(notificationListener)
, _sourceAddress(sourceAddress)
, _targetAddress(targetAddress)
, _protocolVersion(protocolVersion)
{}

void DoIpTransportMessageSendJob::sendTransportMessageProcessed(bool const success)
{
    if (_notificationListener != nullptr)
    {
        _notificationListener->transportMessageProcessed(
            _message,
            success ? ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR
                    : ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR);
    }
}

uint8_t DoIpTransportMessageSendJob::getSendBufferCount() const
{
    return static_cast<uint8_t>(BufferIndex::COUNT);
}

uint16_t DoIpTransportMessageSendJob::getTotalLength() const
{
    return _message.getPayloadLength() + static_cast<uint16_t>(DoIpConstants::DOIP_HEADER_LENGTH)
           + 4U;
}

::ip::IPEndpoint const* DoIpTransportMessageSendJob::getDestinationEndpoint() const
{
    return nullptr;
}

void DoIpTransportMessageSendJob::release(bool const success)
{
    _callback.releaseSendJob(*this, success);
}

::etl::span<uint8_t const> DoIpTransportMessageSendJob::getSendBuffer(
    ::etl::span<uint8_t> const staticBuffer, uint8_t const bufferIndex)
{
    switch (static_cast<BufferIndex>(bufferIndex))
    {
        case BufferIndex::HEADER:
        {
            return DoIpSendJobHelper::prepareHeaderBuffer(
                staticBuffer,
                static_cast<uint8_t>(_protocolVersion),
                DoIpConstants::PayloadTypes::DIAGNOSTIC_MESSAGE,
                static_cast<uint32_t>(_message.getPayloadLength()) + 4U);
        }
        case BufferIndex::STATIC_PAYLOAD:
        {
            if (staticBuffer.size() < sizeof(uint16_t) * 2)
            {
                return {};
            }
            auto const s = staticBuffer.reinterpret_as<::etl::be_uint16_t>();
            s[0]         = _sourceAddress;
            s[1]         = _targetAddress;
            return staticBuffer.subspan(0U, sizeof(uint16_t) * 2);
        }
        case BufferIndex::DYNAMIC_PAYLOAD:
        {
            uint8_t const* const payload = _message.getPayload();
            return ::etl::span<uint8_t const>(
                payload, static_cast<size_t>(_message.getPayloadLength()));
        }
        default:
        {
            return {};
        }
    }
}

} // namespace doip
