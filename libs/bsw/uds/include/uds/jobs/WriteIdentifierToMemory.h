// Copyright 2024 Accenture.

#ifndef GUARD_33F9C8F7_79C8_4F96_9C2A_F9F0E90A0693
#define GUARD_33F9C8F7_79C8_4F96_9C2A_F9F0E90A0693

#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/jobs/DataIdentifierJob.h"

#include <estd/slice.h>

#include <cstdint>

namespace uds
{
/**
 * Generic implementation of a WriteDataByIdentifier which can copy the received data
 * to a memory location given by pointer.
 */
class WriteIdentifierToMemory : public DataIdentifierJob
{
    UNCOPYABLE(WriteIdentifierToMemory);

public:
    WriteIdentifierToMemory(
        uint16_t const identifier,
        ::estd::slice<uint8_t> const& memory,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    uint8_t _implementedRequest[3];
    ::estd::slice<uint8_t> const _memory;
};

} // namespace uds

#endif // GUARD_33F9C8F7_79C8_4F96_9C2A_F9F0E90A0693
