// Copyright 2024 Accenture.

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
     * Initializes the safety relevant components. Enables the watchdog, initializes Memory
     * protection unit and Rom check. It is called from the safety task.
     */
    void init();
    void run();
    void shutdown();
    /**
     * Enables and disables the write access to the protected RAM region using the scoped unlock
     * because only safety relevant software can write to the safety RAM region. Services the
     * watchdog every 80ms by calling cyclic method of safeWatchdog. Calls the cyclic function of
     * SafeMemory.
     */
    void cyclic();
    // [PUBLIC_API_END]

private:
    uint16_t _counter;
    static constexpr uint8_t WATCHDOG_CYCLIC_COUNTER = 8;
};

} // namespace safety
