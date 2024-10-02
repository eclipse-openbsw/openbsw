// Copyright 2024 Accenture.

#include "uds/jobs/WriteIdentifierToMemory.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <estd/memory.h>

namespace uds
{
WriteIdentifierToMemory::WriteIdentifierToMemory(
    uint16_t const identifier,
    ::estd::slice<uint8_t> const& memory,
    DiagSessionMask const sessionMask)
: DataIdentifierJob(_implementedRequest, sessionMask), _memory(memory)
{
    _implementedRequest[0] = 0x2EU;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8U) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

DiagReturnCode::Type
WriteIdentifierToMemory::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (!compare(request, getImplementedRequest() + 1U, 2U))
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }

    if (requestLength != (_memory.size() + 2))
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    return DiagReturnCode::OK;
}

DiagReturnCode::Type WriteIdentifierToMemory::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    (void)connection.releaseRequestGetResponse();
    (void)::estd::memory::copy(
        _memory, ::estd::slice<uint8_t const>::from_pointer(request, requestLength));

    (void)connection.sendPositiveResponse(*this);

    return DiagReturnCode::OK;
}

} // namespace uds
