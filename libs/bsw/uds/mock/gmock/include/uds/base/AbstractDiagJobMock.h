// Copyright 2024 Accenture.

#pragma once

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

    MOCK_METHOD(
        DiagReturnCode::Type,
        verify,
        (uint8_t const request[], uint16_t requestLength),
        (override));

    MOCK_METHOD(
        DiagReturnCode::Type,
        process,
        (IncomingDiagConnection & connection, uint8_t const request[], uint16_t requestLength),
        (override));
};

} // namespace uds
