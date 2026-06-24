/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/definition.h"

#include "routing/PduRoutingTable.h"
#include "routing/pduRouting.h"

#include <etl/algorithm.h>
#include <etl/iterator.h>

namespace routing
{
void sort(::etl::span<DefinitionRef> const definitions)
{
    ::etl::sort(
        definitions.begin(),
        definitions.end(),
        [](DefinitionRef const& a, DefinitionRef const& b)
        {
            if (a.d->input.channelId != b.d->input.channelId)
            {
                return (a.d->input.channelId < b.d->input.channelId);
            }
            return (a.d->input.messageId < b.d->input.messageId);
        });
}

void load(
    RxAdapterTable& table,
    ::etl::span<Definition const> input,
    uint8_t const channelId,
    RxAdapterTableMem& mem)
{
    ::etl::vector<DefinitionRef, 1024> definition;
    for (auto const& i : input)
    {
        definition.push_back({&i});
    }
    sort(definition);

    auto const channelBegin = ::etl::find_if(
        definition.begin(),
        definition.end(),
        [channelId](DefinitionRef const e) { return e.d->input.channelId == channelId; });
    auto const channelEnd = ::etl::find_if(
        channelBegin,
        definition.end(),
        [channelId](DefinitionRef const e) { return e.d->input.channelId != channelId; });

    uint32_t currentPduOffset = 0;
    uint32_t currentMsgId     = 0;
    for (auto e = channelBegin; e < channelEnd; e++)
    {
        if (currentMsgId != e->d->input.messageId)
        {
            currentMsgId = e->d->input.messageId;
            mem.messageIds.emplace_back(::etl::be_uint32_t(currentMsgId));
            auto const currentMsgLength = e->d->input.messageLength;
            if (currentMsgLength > 0)
            {
                mem.messageLengths.emplace_back(::etl::be_uint32_t(currentMsgLength));
            }
            mem.pduLengthsOffsets.push_back(::etl::be_uint32_t(currentPduOffset));
        }
        mem.pduLengths.push_back(::etl::be_uint32_t(e->d->input.length));
        mem.pduOffsets.push_back(::etl::be_uint32_t(e->d->input.offset));
        ++currentPduOffset;
    }
    mem.pduLengthsOffsets.push_back(::etl::be_uint32_t(currentPduOffset));

    table.firstId        = static_cast<uint32_t>(::etl::distance(definition.begin(), channelBegin));
    table.messageIds     = mem.messageIds;
    table.messageLengths = {};
    if (mem.messageIds.size() == mem.messageLengths.size())
    {
        table.messageLengths = mem.messageLengths;
    }
    table.pduLengthsOffsets = mem.pduLengthsOffsets;
    table.pduLengths        = mem.pduLengths;
    table.pduOffsets        = mem.pduOffsets;
}

void load(
    TxAdapterTable& table,
    ::etl::span<Definition const> input,
    uint8_t const channelId,
    TxAdapterTableMem& mem)
{
    ::etl::vector<DefinitionRef, 1024> definition;
    for (auto const& i : input)
    {
        definition.push_back({&i});
    }
    sort(definition);

    for (auto const& e : definition)
    {
        for (auto const& o : e.d->outputs)
        {
            if (o.channelId == channelId)
            {
                mem.messageIds.push_back(::etl::be_uint32_t(o.messageId));
                mem.messageLengths.push_back(::etl::be_uint32_t(o.messageLength));
                mem.offsets.push_back(::etl::be_uint32_t(o.offset));
            }
        }
    }

    table.messageIds     = mem.messageIds;
    table.messageLengths = mem.messageLengths;
    table.pduOffsets     = mem.offsets;
}

void load(PduRoutingTable& table, ::etl::span<Definition const> input, RoutingTableMem& mem)
{
    ::etl::vector<DefinitionRef, 1024> definition;
    for (auto const& i : input)
    {
        definition.push_back({&i});
    }
    sort(definition);

    uint32_t currentRoutingDestinationIndex = 0;
    mem.destinationOffsets.emplace_back(::etl::be_uint32_t(currentRoutingDestinationIndex));
    for (auto const e : definition)
    {
        for (auto const& d : e.d->outputs)
        {
            mem.destinations.emplace_back(d.channelId);
            mem.outputMessageIds.emplace_back(d.messageId);
            ++currentRoutingDestinationIndex;
        }
        mem.destinationOffsets.emplace_back(::etl::be_uint32_t(currentRoutingDestinationIndex));
    }

    table.destinationOffsets = mem.destinationOffsets;
    table.destinations       = mem.destinations;
    table.outputMessageIds   = mem.outputMessageIds;
}

} // namespace routing
