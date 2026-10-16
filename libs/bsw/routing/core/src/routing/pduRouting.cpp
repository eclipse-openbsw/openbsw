/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/pduRouting.h"

#include "routing/Logger.h"

#include <blob/Config.h>
#include <blob/util.h>
#include <etl/span.h>
#include <util/logger/Logger.h>

namespace routing
{
namespace logger = ::util::logger;

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.

bool load(::etl::span<uint8_t const> mem, ::routing::PduRoutingTable& table)
{
    if (mem.empty())
    {
        return false;
    }

    auto const header = ::blob::loadConfigHeader(mem);
    if (header.configType != ::blob::Config::Type::ROUTING)
    {
        return false;
    }

    mem = ::blob::loadColumn(table.destinationOffsets, mem);
    mem = ::blob::loadColumn(table.outputMessageIds, mem);
    mem = ::blob::loadColumn(table.destinations, mem);

    return true;
}

bool route(
    uint8_t const srcChannelId,
    PduRoutingTable const& table,
    ::io::IReader& reader,
    ::etl::span<::io::IWriter*> const writers)
{
    auto const pdu = reader.peek();

    if (pdu.empty())
    {
        return false;
    }

    // We need to be able to extract at least the internalPduId
    if (pdu.size() < sizeof(::etl::be_uint32_t))
    {
        reader.release();
        logger::Logger::error(
            logger::ROUTING, "Invalid PDU size [%u]", static_cast<uint32_t>(pdu.size()));
        return false;
    }

    logger::Logger::debug(
        logger::ROUTING,
        "Processing input PDU (src-channel-index: %u, pdu-size: %u)",
        srcChannelId,
        static_cast<uint32_t>(pdu.size()));

    uint32_t const internalPduId = ::etl::be_uint32_t(pdu.data());
    if (static_cast<size_t>(internalPduId) + 1U >= table.destinationOffsets.size())
    {
        reader.release();
        logger::Logger::error(
            logger::ROUTING, "Unknown PDU [%u]", static_cast<uint32_t>(internalPduId));
        return false;
    }

    logger::Logger::debug(
        logger::ROUTING,
        "Looking for a route (internal-pdu-id: %u)",
        static_cast<uint32_t>(internalPduId));

    uint32_t const start = table.destinationOffsets[internalPduId];
    uint32_t const end   = table.destinationOffsets[static_cast<size_t>(internalPduId) + 1U];

    auto const dsts             = table.destinations.subspan(start, end - start);
    auto const outputMessageIds = table.outputMessageIds.subspan(start, end - start);

    auto const payload = pdu.subspan(sizeof(::etl::be_uint32_t));
    for (size_t i = 0; i < dsts.size(); i++)
    {
        auto const dst = dsts[i];
        logger::Logger::debug(logger::ROUTING, "Routing PDU to destination [%u]", dst);
        auto* const writer = dst < writers.size() ? writers[dst] : nullptr;
        if (writer == nullptr)
        {
            continue;
        }
        ::routing::outputPdu(outputMessageIds[i], dst, payload, *writer);
    }

    reader.release();

    return true;
}

void outputPdu(
    ::etl::be_uint32_t const outputMessageId,
    uint8_t const dstChannelId,
    ::etl::span<uint8_t const> const payload,
    ::io::IWriter& writer)
{
    auto const pduSize = sizeof(::etl::be_uint32_t) + payload.size();
    auto dstPdu        = writer.allocate(pduSize);
    if (dstPdu.size() != pduSize)
    {
        return;
    }

    dstPdu.take<::etl::be_uint32_t>() = outputMessageId;
    (void)::etl::copy(payload, dstPdu);
    writer.commit();

    logger::Logger::debug(
        logger::ROUTING,
        "Writing PDU to output (msg-id: %u, dst-channel-index: %u, payload-size: %u)",
        static_cast<uint32_t>(outputMessageId),
        dstChannelId,
        static_cast<uint32_t>(payload.size()));
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace routing
