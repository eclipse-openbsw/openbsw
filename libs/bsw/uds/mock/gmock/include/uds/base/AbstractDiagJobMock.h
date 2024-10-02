// Copyright 2024 Accenture.

#ifndef GUARD_9D13D63A_CA83_4EB5_85E4_8C365BAF90F8
#define GUARD_9D13D63A_CA83_4EB5_85E4_8C365BAF90F8

#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <gmock/gmock.h>

namespace uds
{
class AbstractDiagJobMock : public AbstractDiagJob
{
public:
    AbstractDiagJobMock(
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask)
    : AbstractDiagJob(implementedRequest, requestLength, prefixLength, sessionMask)
    {}

    MOCK_METHOD2(verify, DiagReturnCode::Type(uint8_t const request[], uint16_t requestLength));

    MOCK_METHOD3(
        process,
        DiagReturnCode::Type(
            IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength));
};

} // namespace uds

#endif // GUARD_9D13D63A_CA83_4EB5_85E4_8C365BAF90F8
