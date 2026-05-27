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
 * Helper base class for UDS services.
 *
 *
 * \see AbstractDiagJob
 */
class Service : public AbstractDiagJob
{
public:
    Service(uint8_t const service, DiagSession::DiagSessionMask sessionMask);

    Service(
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::enableSuppressPositiveResponse()
     */
    using AbstractDiagJob::enableSuppressPositiveResponse;

    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

private:
    void init(uint8_t service);

    uint8_t fService[1];
};

} // namespace uds
