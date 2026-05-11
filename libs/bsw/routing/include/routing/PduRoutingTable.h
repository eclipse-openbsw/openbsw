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
// [PDU_ROUTING_TABLE_BEGIN]
/**
 * The input of the routing is the output of the adapters, i.e. Internal PDU ids.
 *
 * The routing table consists of two parts:
 *
 * 1) Routing to destinations.
 *    The internal PDU id is used as index into destinationOffsets. The range
 *    destinations[destinationOffsets[pduId], destinationOffsets[pduId + 1][ are the
 *    destination channel indices to which the PDU is routed.
 *
 * 2) Translation from internal PDU id to destination channel specific addressing.
 *    The index of a routing in the destinations table is also used in the outputMessageIds table.
 *    This contains the message ID to output on the channel for this message.
 *
 * The sizes of the spans are expressed according to:
 *  - N: the number of known PDUs
 *  - M: the number of channels
 */
struct PduRoutingTable
{
    // size: [N,(M*N)]
    ::etl::span<uint8_t const> destinations;

    // size: N+1
    ::etl::span<::etl::be_uint32_t const> destinationOffsets;

    // size: [N, (M*N)]
    ::etl::span<::etl::be_uint32_t const> outputMessageIds;
};

// [PDU_ROUTING_TABLE_END]

/**
 * Fills table with values loaded from mem, returning true on success and false on error.
 */
bool load(::etl::span<uint8_t const> mem, ::routing::PduRoutingTable& table);

} // namespace routing
