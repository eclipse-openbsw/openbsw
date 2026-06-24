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

#include "routing/RxAdapterTable.h"
#include "routing/TxAdapterTable.h"

#include <etl/span.h>
#include <etl/unaligned_type.h>
#include <etl/vector.h>

#include <platform/estdint.h>

namespace routing
{
struct PduRoutingTable;

struct Definition
{
    static constexpr size_t MAX_OUTPUTS = 255;

    struct Pdu
    {
        Pdu(uint8_t const channelId,
            uint32_t const messageId,
            uint32_t const messageLength,
            uint32_t const offset,
            uint32_t const length)
        : channelId(channelId)
        , messageId(::etl::be_uint32_t(messageId))
        , messageLength(::etl::be_uint32_t(messageLength))
        , offset(offset)
        , length(length){};

        Pdu() = default;

        uint8_t channelId                = 0;
        ::etl::be_uint32_t messageId     = 0U;
        ::etl::be_uint32_t messageLength = 0U;
        uint32_t offset                  = 0;
        uint32_t length                  = 0;
    };

    Definition() = default;

    Definition&
    in(uint8_t const channelId,
       uint32_t const messageId,
       uint32_t const offset,
       uint32_t const length)
    {
        input = {channelId, messageId, 0, offset, length};
        return *this;
    }

    Definition&
    in(uint8_t const channelId,
       uint32_t const messageId,
       uint32_t const messageLength,
       uint32_t const offset,
       uint32_t const length)
    {
        input = {channelId, messageId, messageLength, offset, length};
        return *this;
    }

    Definition&
    out(uint8_t const channelId,
        uint32_t const messageId,
        uint32_t const messageLength,
        uint32_t const offset)
    {
        outputs.push_back({channelId, messageId, messageLength, offset, 0});
        return *this;
    }

    Pdu input;
    ::etl::vector<Pdu, MAX_OUTPUTS> outputs;
};

struct DefinitionRef
{
    Definition const* d;
};

struct RoutingTableMem
{
    static constexpr size_t MAX_IDS = 1024;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> destinationOffsets;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> outputMessageIds;
    ::etl::vector<uint8_t, MAX_IDS> destinations;
};

struct RxAdapterTableMem
{
    static constexpr size_t MAX_IDS = 1024;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> messageIds;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> messageLengths;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> pduLengthsOffsets;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> pduLengths;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> pduOffsets;
};

struct TxAdapterTableMem
{
    static constexpr size_t MAX_IDS = 1024;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> messageIds;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> messageLengths;
    ::etl::vector<::etl::be_uint32_t, MAX_IDS> offsets;
};

void sort(::etl::span<DefinitionRef> const definitions);

void load(
    TxAdapterTable& table,
    ::etl::span<Definition const> input,
    uint8_t const channelId,
    TxAdapterTableMem& mem);

void load(
    RxAdapterTable& table,
    ::etl::span<Definition const> input,
    uint8_t const channelId,
    RxAdapterTableMem& mem);

void load(PduRoutingTable& table, ::etl::span<Definition const> input, RoutingTableMem& mem);

} // namespace routing
