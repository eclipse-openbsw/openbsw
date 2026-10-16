// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "uds/eventmgr/IDebounceAlgorithm.h"

namespace uds
{

/**
 * Immediate detection strategy — no debounce, direct qualification.
 * Any PREFAILED report immediately yields FAILED; any PREPASSED yields PASSED.
 * Algorithm 0 (DGN-FEL008).
 */
class DebounceImmediate : public IDebounceAlgorithm
{
public:
    DebounceImmediate();

    EventStatus update(bool isFailed, uint16_t elapsedMs) override;
    void reset() override;

private:
    EventStatus _currentStatus;
};

} // namespace uds
