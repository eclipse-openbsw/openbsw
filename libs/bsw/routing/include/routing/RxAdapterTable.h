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

#include <etl/span.h>
#include <etl/unaligned_type.h>

#include <platform/estdint.h>

namespace routing
{
// [RX_ADAPTER_TABLE_BEGIN]
// The sizes below are expressed as a function of:
//  - N: the number of known PDUs (>= P)
//  - P: the number of known messages
struct RxAdapterTable
{
    // the first internal PDU ID of this channel. All other PDU IDs are derived from it.
    ::etl::be_uint32_t firstId = {};
    // The message IDs that can be received.
    // The index is called messageIndex. It is shared with messageLengths and pduLengthsOffsets.
    // size: P
    ::etl::span<::etl::be_uint32_t const> messageIds;
    // The lengths of the messages that can be received. If the length of a message
    // does not match the expected one, no PDU is extracted. If empty, it is ignored.
    // size: P
    ::etl::span<::etl::be_uint32_t const> messageLengths;
    // A pointer into the PDULengths and PDUOffsets tables.
    // size: P + 1
    ::etl::span<::etl::be_uint32_t const> pduLengthsOffsets;
    // The length of each PDU in each frame.
    // The index is called channelSpecificPduId. It is shared with pduOffsets and
    // used to derive the internal PDU ID by adding it to firstId.
    // size: N
    ::etl::span<::etl::be_uint32_t const> pduLengths;
    // The offset in each frame of each PDU.
    // size: N
    ::etl::span<::etl::be_uint32_t const> pduOffsets;
};

// [RX_ADAPTER_TABLE_END]

/**
 * Fills table with values loaded from mem, returning true on success and false on error.
 */
bool load(::etl::span<uint8_t const> mem, RxAdapterTable& table);

} // namespace routing
