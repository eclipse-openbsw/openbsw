/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/util.h"

#include <can/canframes/CANFrame.h>

#include <blob/Blob.h>
#include <etl/algorithm.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>

namespace routing
{
::etl::span<uint8_t> canFrameToPdu(::can::CANFrame const& frame, ::etl::span<uint8_t> buffer)
{
    uint32_t const payloadLength = frame.getPayloadLength();
    if ((payloadLength > ::can::CANFrame::MAX_FRAME_LENGTH)
        || (buffer.size() < (ROUTING_PDU_HEADER_SIZE + payloadLength)))
    {
        return {};
    }

    ::etl::be_uint32_t const messageId{frame.getId()};
    ::etl::be_uint32_t const parsedPayloadLength{payloadLength};
    etl::copy_n(messageId.data(), ROUTING_PDU_MESSAGE_ID_SIZE, buffer.begin());
    etl::copy_n(
        parsedPayloadLength.data(),
        ROUTING_PDU_PAYLOAD_LENGTH_SIZE,
        buffer.subspan(ROUTING_PDU_MESSAGE_ID_SIZE).begin());
    etl::copy_n(frame.getPayload(), payloadLength, buffer.subspan(ROUTING_PDU_HEADER_SIZE).begin());

    return buffer.first(ROUTING_PDU_HEADER_SIZE + payloadLength);
}

bool pduToCanFrame(::etl::span<uint8_t const> buffer, ::can::CANFrame& frame)
{
    if (buffer.size() < ROUTING_PDU_HEADER_SIZE)
    {
        return false;
    }

    uint32_t const payloadLength = ::etl::be_uint32_t(
        buffer.subspan(ROUTING_PDU_MESSAGE_ID_SIZE, ROUTING_PDU_PAYLOAD_LENGTH_SIZE).data());
    if ((payloadLength > ::can::CANFrame::MAX_FRAME_LENGTH)
        || (buffer.size() < (ROUTING_PDU_HEADER_SIZE + payloadLength)))
    {
        return false;
    }

    uint32_t const messageId
        = ::etl::be_uint32_t(buffer.subspan(0U, ROUTING_PDU_MESSAGE_ID_SIZE).data());
    frame = ::can::CANFrame{
        messageId,
        buffer.subspan(ROUTING_PDU_HEADER_SIZE).data(),
        static_cast<uint8_t>(payloadLength)};
    return true;
}

::blob::Config config(
    ::etl::span<uint8_t const> const blob, ::blob::Config::Type const type, uint8_t const channelId)
{
    for (auto const config : ::blob::Blob(blob))
    {
        if (config.type != type)
        {
            continue;
        }

        auto data = config.data;
        ::routing::Header header;
        if (!::routing::load(data, header))
        {
            continue;
        }

        if (header.channelId != channelId)
        {
            continue;
        }

        return config;
    }

    return {};
}

} // namespace routing
