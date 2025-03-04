// Copyright 2024 Accenture.

#ifndef GUARD_B5DC1958_B003_4B28_88CC_EA8728B2E820
#define GUARD_B5DC1958_B003_4B28_88CC_EA8728B2E820

#include <platform/estdint.h>

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
    uint8_t const WATCHDOG_CYCLIC_COUNTER = 8;
};

} // namespace safety

#endif /* GUARD_B5DC1958_B003_4B28_88CC_EA8728B2E820 */
