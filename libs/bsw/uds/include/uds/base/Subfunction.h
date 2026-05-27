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
 * Base class for UDS subfunctions
 *
 */
class Subfunction : public AbstractDiagJob
{
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
