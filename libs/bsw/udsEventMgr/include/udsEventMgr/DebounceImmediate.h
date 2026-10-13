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
 * Immediate detection strategy — no debounce, direct qualification.
 * Any PREFAILED report immediately yields FAILED; any PREPASSED yields PASSED.
 * Algorithm 0
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
