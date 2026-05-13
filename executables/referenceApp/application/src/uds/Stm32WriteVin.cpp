// Copyright 2026 BMW AG
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32WriteVin.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <algorithm>

namespace uds
{

Stm32WriteVin::Stm32WriteVin(
    uint16_t const identifier, ::etl::span<uint8_t> const& memory)
: DataIdentifierJob(_implementedRequest), _memory(memory)
{
    _implementedRequest[0] = 0x2EU;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8U) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

DiagReturnCode::Type
Stm32WriteVin::verify(uint8_t const* const request, uint16_t const requestLength)
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

DiagReturnCode::Type Stm32WriteVin::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    // Framework already stripped DID bytes; request is data-only, requestLength is data length
    for (uint16_t i = 0U; i < requestLength; ++i)
    {
        _memory[i] = request[i];
    }

    (void)connection.releaseRequestGetResponse();
    (void)connection.sendPositiveResponse(*this);
    return DiagReturnCode::OK;
}

} // namespace uds
