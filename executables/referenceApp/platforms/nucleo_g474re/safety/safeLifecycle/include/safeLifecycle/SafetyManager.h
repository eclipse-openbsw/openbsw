// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

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
