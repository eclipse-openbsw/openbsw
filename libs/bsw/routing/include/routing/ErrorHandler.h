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

#include "routing/Header.h"

#include <etl/delegate.h>
#include <ip/IPAddress.h>
#include <platform/estdint.h>

namespace routing
{
struct ErrorHandler
{
    enum class StatusCode : uint8_t
    {
        OK,
        // There is not enough data to parse the header.
        INVALID_MESSAGE_HEADER_SIZE,
        // The message ID isn't found in the RX/TX adapter table.
        UNKNOWN_MESSAGE_ID,
        // The parsed length of the message is larger than the length of data available.
        INVALID_PARSED_LENGTH,
        // The parsed length of the incoming message doesn't match the configured one (for messages
        // which have a configured messaged length).
        INVALID_MESSAGE_LENGTH,
        // The size of every PDU expected for the parsed ID is larger than the length of data
        // available. Only if at least a PDU is expected for this ID.
        INVALID_PAYLOAD_LENGTH,
        // There is not enough free space in the queue to write the outgoing PDU.
        MEM_ALLOCATION_FAILURE,
        // A PDU transport channel received data from an invalid source.
        INVALID_REMOTE_IP_ADDRESS,
    };

    using Function = ::etl::delegate<void(StatusCode, uint8_t, uint32_t)>;

    ErrorHandler() = default;

    ErrorHandler(Function const f, uint8_t const channelId) : _f(f), _channelId(channelId) {}

    void operator()(StatusCode const statusCode) const
    {
        uint32_t const messageId = 0U;
        _f(statusCode, _channelId, messageId);
    }

    void operator()(StatusCode const statusCode, ::etl::span<uint8_t const> message) const
    {
        uint32_t const messageId
            = (statusCode != StatusCode::INVALID_MESSAGE_HEADER_SIZE)
                      && (message.size() >= sizeof(::etl::be_uint32_t))
                  ? static_cast<uint32_t>(message.take<::etl::be_uint32_t const>())
                  : 0U;
        _f(statusCode, _channelId, messageId);
    }

    void operator()(StatusCode const statusCode, ::ip::IPAddress const& sourceIpAddress) const
    {
        _f(statusCode, _channelId, ::ip::ip4_to_u32(sourceIpAddress));
    }

private:
    Function _f;
    uint8_t const _channelId = ::routing::INVALID_CHANNEL_ID;
};

} // namespace routing
