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

#include <cstdint>

namespace safety
{

class SafetyManager
{
public:
    SafetyManager();
    void init();
    void run();
    void shutdown();
    void cyclic();

private:
    uint16_t _counter;
    static constexpr uint8_t WATCHDOG_CYCLIC_COUNTER = 8U; ///< Kick every 8 cycles (80ms @ 10ms)
};

} // namespace safety
