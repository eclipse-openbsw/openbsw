/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpReceiver.h"

#include "someip/ITpTransceiver.h"
#include "someip/NetworkChannel.h"
#include "someip/logger.h"

#include "someip/endian_helpers.h"

#include <etl/algorithm.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

bool TpReceiver::isActive() const { return (_pChannel != nullptr); }

bool TpReceiver::isExpired(uint32_t const time) const
{
    return ((_timestamp > 0U) && (time >= (_timestamp + ITpTransceiver::TP_RECEIVE_TIMEOUT)));
}

bool TpReceiver::isMatching(NetworkChannel const& channel, SomeIpMessage const& message) const
{
    if (!isActive())
    {
        return false;
    }

    if (_pChannel->getRemoteEndpoint() != channel.getRemoteEndpoint())
    {
        return false;
    }
    if (_pChannel->getLocalPort() != channel.getLocalPort())
    {
        return false;
    }

    SomeIpMessage const _message(_buffer);

    if (_message.getMessageId() != message.getMessageId())
    {
        return false;
    }
    if (_message.getClientId() != message.getClientId())
    {
        return false;
    }
    if (_message.getRawMessageType() != message.getRawMessageType())
    {
        return false;
    }
    if (_message.getInterfaceVersion() != message.getInterfaceVersion())
    {
        return false;
    }

    return true;
}

void TpReceiver::start(NetworkChannel& channel, SomeIpMessage const& message, ITpListener& listener)
{
    _pChannel  = &channel;
    _pListener = &listener;

    // copy msg-header
    auto header = message.getBufferHeader();
    etl::copy(header.begin(), header.end(), _buffer.begin());
}

void TpReceiver::stop()
{
    _pChannel  = nullptr;
    _pListener = nullptr;
    _timestamp = 0U;

    etl::fill(_buffer.begin(), _buffer.end(), 0U);

    _totalPayloadLength    = 0U;
    _receivedPayloadLength = 0U;
}

TpReceiver::TpResult TpReceiver::receive(
    NetworkChannel const& /*channel*/, SomeIpMessage const& message, uint32_t const timestamp)
{
    INFO_LOG(
        SOMEIP,
        "TpReceiver[%p]::receive(): id: 0x%X, type: 0x%X => %d bytes",
        this,
        message.getMessageId(),
        message.getMessageType(),
        message.getTotalLength());

    auto const payload       = message.getBufferPayload();
    size_t const chunkLength = payload.size() - ITpTransceiver::TP_HEADER_LENGTH;
    auto const chunk         = payload.subspan(ITpTransceiver::TP_HEADER_LENGTH, chunkLength);

    // read tp-header
    if (payload.size() < ITpTransceiver::TP_HEADER_LENGTH)
    {
        ERROR_LOG(
            SOMEIP,
            "TpReceiver[%p]::receive(): id: 0x%X, type: 0x%X => no tp-header",
            this,
            message.getMessageId(),
            message.getMessageType());

        return TpReceiver::TpResult::TP_ERROR;
    }
    ITpTransceiver::TpHeader tpHeader{};
    ITpTransceiver::parseTpHeader(::someip::endian::read_be<uint32_t>(&payload[0U]), tpHeader);

    if (!tpHeader.hasMoreSegments)
    {
        _totalPayloadLength = (tpHeader.payloadOffset + chunkLength);
    }

    // copy payload
    size_t const targetLength = static_cast<size_t>(
        chunkLength + static_cast<size_t>(SomeIpMessage::OFFSET_PAYLOAD) + tpHeader.payloadOffset);
    if (_buffer.size() < targetLength)
    {
        ERROR_LOG(
            SOMEIP,
            "TpReceiver[%p]::receive(): id: 0x%X, type: 0x%X => buffer exceeded at %d bytes",
            this,
            message.getMessageId(),
            message.getMessageType(),
            targetLength);

        return TpReceiver::TpResult::TP_ERROR;
    }
    auto destination = _buffer.subspan(
        static_cast<size_t>(SomeIpMessage::OFFSET_PAYLOAD) + tpHeader.payloadOffset);
    etl::copy(chunk.begin(), chunk.end(), destination.begin());
    _receivedPayloadLength += chunkLength;

    bool const lastSegmentReceived = ((_totalPayloadLength != 0U) || (!tpHeader.hasMoreSegments));

    if (lastSegmentReceived && (_totalPayloadLength <= _receivedPayloadLength))
    {
        // adjust msg-header
        SomeIpMessage _message(_buffer);
        _message.setRawMessageType(
            _message.getRawMessageType()
            & static_cast<uint8_t>(~ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK));
        _message.setPayloadLength(static_cast<uint32_t>(_totalPayloadLength));
        if (_pListener != nullptr)
        {
            _pListener->receivedTpMessage(*_pChannel, _message);
        }

        return TpReceiver::TpResult::TP_OK;
    }

    _timestamp = timestamp;

    return TpReceiver::TpResult::TP_PENDING;
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
