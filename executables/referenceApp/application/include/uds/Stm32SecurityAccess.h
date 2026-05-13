// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include "uds/base/Service.h"

namespace uds
{
/**
 * SecurityAccess (SID 0x27) implementation matching the Rust openbsw-rust firmware.
 *
 * Algorithm:
 *   Seed: LCG (a=1664525, c=1013904223) | 1, returned as 2 bytes (upper 16 bits)
 *   Key:  (seed ^ 0xDEADBEEF) truncated to uint16_t
 *
 * Subfunctions:
 *   0x01 = requestSeed  → returns 2-byte seed (or 0x0000 if already unlocked)
 *   0x02 = sendKey      → validates 2-byte key
 */
class Stm32SecurityAccess : public Service
{
public:
    Stm32SecurityAccess();

    bool isUnlocked() const { return _unlocked; }

    void reset();

private:
    static uint8_t const MAX_FAILED_ATTEMPTS = 3U;
    static uint32_t const XOR_SECRET         = 0xDEADBEEFU;
    static uint32_t const LCG_A              = 1664525U;
    static uint32_t const LCG_C              = 1013904223U;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    DiagReturnCode::Type handleRequestSeed(IncomingDiagConnection& connection);
    DiagReturnCode::Type handleSendKey(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength);

    uint32_t nextSeed();

    uint32_t _seed;
    bool _seedIssued;
    bool _unlocked;
    uint8_t _failedAttempts;
    bool _lockedUntilReset;
};

} // namespace uds
