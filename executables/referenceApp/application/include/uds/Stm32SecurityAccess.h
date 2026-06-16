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

#include "uds/base/Service.h"

namespace uds
{
/**
 * SecurityAccess (SID 0x27) implementation for the STM32 diagnostic demo.
 *
 * Algorithm:
 *   Seed: LCG (a=1664525, c=1013904223) | 1, returned as four big-endian bytes
 *   Key:  (seed ^ 0xDEADBEEF) truncated to uint16_t
 *
 * Subfunctions:
 *   0x01 = requestSeed  -> returns 4-byte seed (or zero seed if already unlocked)
 *   0x02 = sendKey      -> validates 2-byte key
 */
class Stm32SecurityAccess : public Service
{
public:
    Stm32SecurityAccess();

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
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    uint32_t nextSeed();

    uint32_t _seed;
    bool _seedIssued;
    bool _unlocked;
    uint8_t _failedAttempts;
    bool _lockedUntilReset;
};

} // namespace uds
