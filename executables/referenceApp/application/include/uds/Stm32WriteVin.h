/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/span.h>
#include <uds/jobs/DataIdentifierJob.h>

namespace uds
{

/**
 * WriteDID handler for VIN (F190) that accepts variable-length payloads.
 *
 * Unlike WriteIdentifierToMemory (which enforces exact length), this handler
 * accepts any payload from 1 to memory.size() bytes.
 */
class Stm32WriteVin : public DataIdentifierJob
{
public:
    Stm32WriteVin(uint16_t identifier, ::etl::span<uint8_t> const& memory);

    DiagReturnCode::Type verify(uint8_t const* request, uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;

private:
    uint8_t _implementedRequest[3];
    ::etl::span<uint8_t> _memory;
};

} // namespace uds
