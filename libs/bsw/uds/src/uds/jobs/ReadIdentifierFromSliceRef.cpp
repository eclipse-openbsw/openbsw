/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/jobs/ReadIdentifierFromSliceRef.h"

#include "uds/connection/IncomingDiagConnection.h"

namespace uds
{
ReadIdentifierFromSliceRef::ReadIdentifierFromSliceRef(
    uint16_t const identifier,
    ::etl::span<uint8_t const> const& responseData,
    DiagSessionMask const sessionMask)
: DataIdentifierJob(_implementedRequest, sessionMask), _responseSlice(responseData)
{
    _implementedRequest[0] = 0x22U;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

DiagReturnCode::Type ReadIdentifierFromSliceRef::process(
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendData(_responseSlice.data(), static_cast<uint16_t>(_responseSlice.size()));
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);

    return DiagReturnCode::OK;
}

} // namespace uds
