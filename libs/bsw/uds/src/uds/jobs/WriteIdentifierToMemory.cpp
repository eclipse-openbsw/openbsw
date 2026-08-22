/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/jobs/WriteIdentifierToMemory.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <etl/memory.h>

namespace uds
{
namespace
{
::etl::array<uint8_t, 3U> makeImplementedRequest(uint16_t const identifier)
{
    return {
        {0x2EU,
         static_cast<uint8_t>((identifier >> 8U) & 0xFFU),
         static_cast<uint8_t>(identifier & 0xFFU)}};
}
} // namespace

WriteIdentifierToMemory::WriteIdentifierToMemory(
    uint16_t const identifier,
    ::etl::span<uint8_t> const& memory,
    DiagSessionMask const sessionMask)
: DataIdentifierJob(makeImplementedRequest(identifier), sessionMask), _memory(memory)
{}

DiagReturnCode::Type
WriteIdentifierToMemory::verify(uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type const result = DataIdentifierJob::verify(request, requestLength);
    if (result != DiagReturnCode::OK)
    {
        return result;
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
    (void)::etl::copy(
        etl::span<uint8_t const>(request, static_cast<size_t>(requestLength)), _memory);

    (void)connection.sendPositiveResponse(*this);

    return DiagReturnCode::OK;
}

} // namespace uds
