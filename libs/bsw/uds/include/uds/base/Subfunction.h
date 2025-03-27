// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "uds/base/AbstractDiagJob.h"

namespace uds
{
/**
 * Base class for UDS subfunctions
 *
 */
class Subfunction : public AbstractDiagJob
{
    UNCOPYABLE(Subfunction);

public:
    Subfunction(uint8_t const implementedRequest[], DiagSession::DiagSessionMask sessionMask);

    Subfunction(
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type
    verify(uint8_t const* const request, uint16_t const requestLength) override;

private:
    static uint8_t const MINIMUM_REQUEST_LENGTH = 1U;
};

} // namespace uds
