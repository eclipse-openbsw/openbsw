// Copyright 2026 BMW AG
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include <etl/span.h>
#include <uds/jobs/DataIdentifierJob.h>

namespace uds
{

/**
 * WriteDID handler for VIN (F190) that accepts variable-length payloads.
 *
 * Unlike WriteIdentifierToMemory (which enforces exact length), this handler
 * accepts any payload from 1 to memory.size() bytes — matching the Rust
 * firmware behavior tested by test_uds_services_full.py.
 */
class Stm32WriteVin : public DataIdentifierJob
{
public:
    Stm32WriteVin(uint16_t identifier, ::etl::span<uint8_t> const& memory);

    DiagReturnCode::Type
    verify(uint8_t const* request, uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;

private:
    uint8_t _implementedRequest[3];
    ::etl::span<uint8_t> _memory;
};

} // namespace uds
