/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
