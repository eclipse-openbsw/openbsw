/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpSender.h"

#include "someip/ITpTransceiver.h"
#include "someip/NetworkChannel.h"
#include "someip/SomeIpConstants.h"
#include "someip/endian_helpers.h"
#include "someip/logger.h"

#include <etl/algorithm.h>

namespace someip
{
using ::util::logger::SOMEIP;

TpSender::TpResult TpSender::send(NetworkChannel& channel, SomeIpMessage const& message)
{
    if (_buffer.size() < UDP_PACKET_MAX_SIZE)
    {
        ERROR_LOG(
            SOMEIP, "TpSender[%p]::send(): buffer of %d bytes too small!", this, _buffer.size());

        return TpSender::TpResult::TP_ERROR;
    }

    auto const header  = message.getBufferHeader();
    auto const payload = message.getBufferPayload();

    size_t const payloadLength = message.getPayloadLength();
    size_t payloadOffset       = 0U;

    do
    {
        size_t const payloadLeft = payloadLength - payloadOffset;
        size_t const chunkLength
            = (payloadLeft > TP_PAYLOAD_MAX_SIZE) ? TP_PAYLOAD_MAX_SIZE : payloadLeft;
        ::etl::span<uint8_t const> const chunk = payload.subspan(payloadOffset, chunkLength);

        SomeIpMessage tpMessage(_buffer);
        size_t packetOffset = 0U;

        // copy msg-header
        auto dest = _buffer.subspan(packetOffset);
        etl::copy(header.begin(), header.end(), dest.begin());
        packetOffset += header.size();

        // adjust msg-header
        tpMessage.setRawMessageType(
            message.getRawMessageType()
            | static_cast<uint8_t>(ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK));
        tpMessage.setPayloadLength(
            static_cast<uint32_t>(ITpTransceiver::TP_HEADER_LENGTH + chunkLength));

        // add tp-header
        ITpTransceiver::TpHeader tpHeader{};
        tpHeader.payloadOffset   = static_cast<uint32_t>(payloadOffset);
        tpHeader.hasMoreSegments = ((payloadLeft - chunkLength) > 0U);
        ::someip::endian::write_be<uint32_t>(
            &_buffer[packetOffset], ITpTransceiver::serializeTpHeader(tpHeader));
        packetOffset += ITpTransceiver::TP_HEADER_LENGTH;

        // copy payload
        dest = _buffer.subspan(packetOffset);
        etl::copy(chunk.begin(), chunk.end(), dest.begin());
        packetOffset += chunk.size();

        INFO_LOG(
            SOMEIP,
            "TpSender[%p]::send(): id: 0x%X, type: 0x%X => %d bytes",
            this,
            tpMessage.getMessageId(),
            tpMessage.getMessageType(),
            packetOffset);

        if (!channel.send(static_cast<uint32_t>(packetOffset), _buffer))
        {
            ERROR_LOG(
                SOMEIP,
                "TpSender[%p]::send(): send failed at offset %d of %d bytes payload",
                this,
                payloadOffset,
                payloadLength);

            return TpSender::TpResult::TP_ERROR;
        }

        payloadOffset += chunkLength;

    } while ((payloadLength - payloadOffset) > 0);

    return TpSender::TpResult::TP_OK;
}

} // namespace someip
