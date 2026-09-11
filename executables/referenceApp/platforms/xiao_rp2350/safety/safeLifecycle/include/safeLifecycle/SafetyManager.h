/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <platform/estdint.h>

namespace safety
{

class SafetyManager
{
public:
    // [PUBLIC_API_START]
    /**
     * Initializes the counter used for cyclic checks with 0.
     */
    SafetyManager();
    /**
     * Initializes the safety relevant components. Enables the watchdog. It is called from the
     * safety task.
     */
    void init();
    void run();
    void shutdown();
    /**
     * Services the watchdog every 80ms by calling the cyclic method of safeWatchdog.
     */
    void cyclic();
    // [PUBLIC_API_END]

private:
    uint32_t _counter;
    static constexpr uint32_t WATCHDOG_CYCLIC_COUNTER = 8U;
};

} // namespace safety
