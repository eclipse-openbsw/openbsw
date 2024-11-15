// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"
#include "uds/jobs/DataIdentifierJob.h"

#include <etl/span.h>

namespace uds
{
/**
 * Generic implementation of a ReadDataByIdentifier which can respond with a
 * value stored in a slice. In this class only the reference to the slice is stored
 * so the slice content could be changed during runtime
 */
class ReadIdentifierFromSliceRef : public DataIdentifierJob
{
public:
    ReadIdentifierFromSliceRef(
        uint16_t const identifier,
        ::etl::span<uint8_t const> const& responseData,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    uint8_t _implementedRequest[3];
    ::etl::span<uint8_t const> const& _responseSlice;
};

} // namespace uds
