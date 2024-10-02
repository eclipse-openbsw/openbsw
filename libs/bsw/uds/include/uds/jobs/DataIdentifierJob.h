// Copyright 2024 Accenture.

#ifndef GUARD_85AC65F2_2879_44D0_95FB_D57230E57612
#define GUARD_85AC65F2_2879_44D0_95FB_D57230E57612

#include "estd/uncopyable.h"
#include "uds/base/AbstractDiagJob.h"

namespace uds
{
/**
 * Helper class for all ReadDataByIdentifier and WriteDataByIdentifier jobs.
 *
 *
 * This class implements the verify() function in a suitable way for
 * ReadDataByIdentifier and WriteDataByIdentifier jobs. The implementedRequest's
 * length is expected to be 3.
 *
 * \note    Multiple identifiers are not supported!
 */
class DataIdentifierJob : public AbstractDiagJob
{
    UNCOPYABLE(DataIdentifierJob);

public:
    DataIdentifierJob(
        uint8_t const* const implementedRequest,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(implementedRequest, 3U, 1U, sessionMask)
    {}

    DataIdentifierJob(
        uint8_t const* const implementedRequest,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(implementedRequest, 3U, 1U, requestPayloadLength, responseLength, sessionMask)
    {}

    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};

} // namespace uds

#endif // GUARD_85AC65F2_2879_44D0_95FB_D57230E57612
