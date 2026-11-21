/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once
#include "udsEventMgr/IDebounceAlgorithm.h"

namespace uds
{

/**
 * Counter-based debounce (Algorithm 1).
 * Increments on failure reports, decrements on pass reports.
 * Qualifies FAILED when counter >= failThreshold.
 * Qualifies PASSED when counter drops to 0.
 */
class DebounceCounter : public IDebounceAlgorithm
{
public:
    DebounceCounter();

    DebounceCounter(uint16_t failThreshold, uint16_t passThreshold);

    EventStatus update(bool isFailed, uint16_t elapsedMs) override;
    void reset() override;

private:
    uint16_t _failThreshold;
    uint16_t _passThreshold;
    uint16_t _counter;
    EventStatus _currentStatus;
};

} // namespace uds
