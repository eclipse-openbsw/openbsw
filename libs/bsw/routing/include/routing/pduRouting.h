/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "routing/PduRoutingTable.h"

#include <etl/span.h>
#include <etl/unaligned_type.h>
#include <io/IReader.h>
#include <io/IWriter.h>

#include <cstdint>

namespace can
{
class CANFrame;
}

namespace routing
{

static constexpr size_t ROUTING_PDU_MESSAGE_ID_SIZE     = sizeof(uint32_t);
static constexpr size_t ROUTING_PDU_PAYLOAD_LENGTH_SIZE = sizeof(uint32_t);
static constexpr size_t ROUTING_PDU_HEADER_SIZE
    = ROUTING_PDU_MESSAGE_ID_SIZE + ROUTING_PDU_PAYLOAD_LENGTH_SIZE;

/**
 * Serializes a CAN frame into the routing PDU format.
 *
 * The output contains the frame ID, payload length, and payload in that order,
 * using big-endian 32-bit header fields. An empty span is returned when the
 * supplied buffer cannot hold the complete PDU.
 */
::etl::span<uint8_t> canFrameToPdu(::can::CANFrame const& frame, ::etl::span<uint8_t> buffer);

/**
 * Deserializes a routing PDU into a CAN frame.
 *
 * Returns false when the header is incomplete, the declared payload exceeds
 * the CAN frame capacity, or the buffer does not contain the declared payload.
 */
bool pduToCanFrame(::etl::span<uint8_t const> buffer, ::can::CANFrame& frame);

bool route(
    uint8_t srcChannelId,
    PduRoutingTable const& table,
    ::io::IReader& reader,
    ::etl::span<::io::IWriter*> writers);

void outputPdu(
    ::etl::be_uint32_t outputMessageId,
    uint8_t dstChannelId,
    ::etl::span<uint8_t const> const payload,
    ::io::IWriter& writer);

} // namespace routing
