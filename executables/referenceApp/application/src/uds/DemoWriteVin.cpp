/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/DemoWriteVin.h"

#include "uds/UdsConstants.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <etl/algorithm.h>
#include <etl/memory.h>

namespace uds
{

DemoWriteVin::DemoWriteVin(uint16_t const identifier, ::etl::span<uint8_t> const& memory)
: DataIdentifierJob(_implementedRequest), _memory(memory)
{
    _implementedRequest[0] = ServiceId::WRITE_DATA_BY_IDENTIFIER;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8U) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

DiagReturnCode::Type
DemoWriteVin::verify(uint8_t const* const request, uint16_t const requestLength)
{
    // request[0..1] = DID high/low (SID already stripped by parent)
    if (!compare(request, getImplementedRequest() + 1U, 2U))
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }

    // Must have at least 2 DID bytes + 1 data byte
    if (requestLength < 3U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    // Data portion must fit in the memory buffer
    uint16_t const dataLen = requestLength - 2U;
    if (dataLen > _memory.size())
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    return DiagReturnCode::OK;
}

DiagReturnCode::Type DemoWriteVin::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    // Framework already stripped DID bytes; request is data-only.
    (void)::etl::copy(
        ::etl::span<uint8_t const>(request, static_cast<size_t>(requestLength)), _memory);
    // Zero-fill the tail so a short write leaves no stale bytes behind.
    ::etl::fill(_memory.begin() + requestLength, _memory.end(), static_cast<uint8_t>(0U));

    (void)connection.releaseRequestGetResponse();
    (void)connection.sendPositiveResponse(*this);
    return DiagReturnCode::OK;
}

} // namespace uds
