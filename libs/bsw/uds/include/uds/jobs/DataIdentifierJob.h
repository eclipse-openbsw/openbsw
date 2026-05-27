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
