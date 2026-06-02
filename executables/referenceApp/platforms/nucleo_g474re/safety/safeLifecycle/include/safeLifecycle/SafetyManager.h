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
     * Checks whether the previous reset was caused by the IWDG watchdog and
     * clears the reset flag. It is called from the safety task.
     */
    void init();
    /**
     * Enables the IWDG watchdog (250 ms timeout). Called once the cyclic
     * scheduler is running, so the timeout cannot fire before the first kick.
     */
    void run();
    void shutdown();
    /**
     * Reports the enter/leave sequence checkpoints to the SafeSupervisor and
     * services the watchdog every 8th cycle (80 ms at the 10 ms safety cycle).
     */
    void cyclic();
    // [PUBLIC_API_END]

private:
    uint16_t _counter;
    static constexpr uint8_t WATCHDOG_CYCLIC_COUNTER = 8U; ///< Kick every 8 cycles (80ms @ 10ms)
};

} // namespace safety
