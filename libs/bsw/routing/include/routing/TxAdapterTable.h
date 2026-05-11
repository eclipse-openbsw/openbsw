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
// [TX_ADAPTER_TABLE_BEGIN]
// The sizes below are expresed as a function of:
//  - N: the number of known PDUs
struct TxAdapterTable
{
    // The message ids which can be sent by this adapter
    // size: N
    ::etl::span<::etl::be_uint32_t const> messageIds;
    // The length of each message to be sent, achieved using padding with 0xFF.
    // size: N
    ::etl::span<::etl::be_uint32_t const> messageLengths;
    // The offset of the PDU to be sent in the message.
    // size: N
    ::etl::span<::etl::be_uint32_t const> pduOffsets;
};

// [TX_ADAPTER_TABLE_END]

/**
 * Fills table with values loaded from mem, returning true on success and false on error.
 */
bool load(::etl::span<uint8_t const> mem, TxAdapterTable& table);

} // namespace routing
