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

#include "platform/estdint.h"
#include "uds/jobs/DataIdentifierJob.h"

#include <etl/span.h>

#include <cstdint>

namespace uds
{
/**
 * Generic implementation of a WriteDataByIdentifier which can copy the received data
 * to a memory location given by pointer.
 */
class WriteIdentifierToMemory : public DataIdentifierJob
{
public:
    WriteIdentifierToMemory(
        uint16_t const identifier,
        ::etl::span<uint8_t> const& memory,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    uint8_t _implementedRequest[3];
    ::etl::span<uint8_t> const _memory;
};

} // namespace uds
