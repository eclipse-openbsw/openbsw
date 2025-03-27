// Copyright 2024 Accenture.

#pragma once

#include "estd/array.h"
#include "estd/slice.h"
#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/jobs/DataIdentifierJob.h"

namespace uds
{
class ReadIdentifierPot : public DataIdentifierJob
{
    UNCOPYABLE(ReadIdentifierPot);

public:
    ReadIdentifierPot(DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    uint8_t _implementedRequest[3];
};

} // namespace uds
