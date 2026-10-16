// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "uds/eventmgr/IDebounceAlgorithm.h"

namespace uds
{

/**
 * Time-based debounce (Algorithm 2, DGN-FEL002/008).
 * Accumulates elapsed milliseconds of continuous failure/pass reporting.
 * Qualifies FAILED when accumulator >= failThresholdMs.
 * Qualifies PASSED when accumulator >= passThresholdMs.
 * Accumulator resets on direction change.
 */
class DebounceTimeBased : public IDebounceAlgorithm
{
public:
    DebounceTimeBased();

    DebounceTimeBased(uint16_t failThresholdMs, uint16_t passThresholdMs);

    EventStatus update(bool isFailed, uint16_t elapsedMs) override;
    void reset() override;

private:
    uint16_t _failThresholdMs;
    uint16_t _passThresholdMs;
    uint16_t _failAccumulatorMs;
    uint16_t _passAccumulatorMs;
    EventStatus _currentStatus;
    bool _lastSignalWasFail;
};

} // namespace uds
