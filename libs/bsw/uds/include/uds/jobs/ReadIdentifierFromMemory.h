// Copyright 2024 Accenture.

#pragma once

#include "estd/slice.h"
#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/jobs/DataIdentifierJob.h"

namespace uds
{
/**
 * Generic implementation of a ReadDataByIdentifier which can respond with a fixed
 * value given by a pointer to a memory location.
 */
class ReadIdentifierFromMemory : public DataIdentifierJob
{
    UNCOPYABLE(ReadIdentifierFromMemory);

public:
    ReadIdentifierFromMemory(
        uint16_t const identifier,
        uint8_t const* const responseData,
        uint16_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    ReadIdentifierFromMemory(
        uint16_t const identifier,
        ::estd::slice<uint8_t const> const& responseData,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    uint8_t _implementedRequest[3];
    ::estd::slice<uint8_t const> _responseSlice;
};

} // namespace uds

