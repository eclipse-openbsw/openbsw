/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ITpTransceiver.h"

#include "someip/SomeIpConstants.h"

namespace someip
{
// static
bool ITpTransceiver::isOutgoingTpMessage(proto::type const proto, uint32_t const length)
{
    if (proto::SD_L4_PROTO_UDP != proto)
    {
        return false;
    }

    return (length > UDP_PACKET_MAX_SIZE);
}

// static
bool ITpTransceiver::isIncomingTpMessage(
    proto::type const proto, SomeIpMessage::MessageType const type)
{
    if (proto::SD_L4_PROTO_UDP != proto)
    {
        return false;
    }

    return (
        (static_cast<uint8_t>(type) & static_cast<uint8_t>(TP_MESSAGE_TYPE_BIT_MASK))
        == static_cast<uint8_t>(TP_MESSAGE_TYPE_BIT_MASK));
}

// static
uint32_t ITpTransceiver::serializeTpHeader(TpHeader const& input)
{
    uint32_t const offset = static_cast<uint32_t>(input.payloadOffset & 0xFFFFFFF0U);
    uint32_t const flags
        = input.hasMoreSegments ? static_cast<uint32_t>(1U) : static_cast<uint32_t>(0U);

    return (offset | flags);
}

// static
void ITpTransceiver::parseTpHeader(uint32_t const input, TpHeader& output)
{
    output.payloadOffset   = static_cast<size_t>(input & 0xFFFFFFF0U);
    output.hasMoreSegments = static_cast<bool>(input & 0x00000001U);
}

} // namespace someip
