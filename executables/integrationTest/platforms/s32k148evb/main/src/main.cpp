// Copyright 2025 Accenture.

#include "async/Config.h"
#include "cache/cache.h"
#include "clock/clockConfig.h"
#include "interrupt_manager.h"
#include "lifecycle/StaticBsp.h"
#include "systems/BspSystem.h"

#include <etl/alignment.h>
#include <lifecycle/LifecycleManager.h>
#include <watchdog/Watchdog.h>

#include <cinttypes>
#include <cstdio>

extern void app_main();

extern "C"
{
void ExceptionHandler()
{
    while (true)
    {
        printf("ExceptionHandler :(");
    }
}

void boardInit()
{
    ::safety::bsp::Watchdog::disableWatchdog();
    configurPll();
    cacheEnable();
}

void setupApplicationsIsr(void)
{
    // interrupts
    ENABLE_INTERRUPTS();
}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

::etl::typed_storage<::systems::BspSystem> bspSystem;

/**
 * Callout from main application to give platform the chance to add a
 * ::lifecycle::ILifecycleComponent to the \p lifecycleManager at a given \p level.
 */
void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    if (level == 1U)
    {
        lifecycleManager.addComponent("bsp", bspSystem.create(TASK_BSP, staticBsp), level);
    }
}
} // namespace platform

int main()
{
    ::platform::staticBsp.init();
    printf("main(RCM::SRS 0x%" PRIx32 ")\r\n", *reinterpret_cast<uint32_t volatile*>(0x4007F008));
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
