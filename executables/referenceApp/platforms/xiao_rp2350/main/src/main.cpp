/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "pico.h"
#include "pico/stdlib.h"
#include "systems/BspSystem.h"

#include <etl/alignment.h>
#include <etl/print.h>
#include <printf/printf.h>

#include <cinttypes>
#include <cstdio>
#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif
#include "cache/cache.h"
#include "clock/clockConfig.h"
#include "lifecycle/StaticBsp.h"

#include <async/Config.h>
#include <lifecycle/LifecycleManager.h>
#include <safeSupervisor/SafeSupervisor.h>
#include <safeWatchdog/SafeWatchdog.h>
#include <watchdog/Watchdog.h>

extern void app_main();

extern safety::SafeWatchdog safeWatchdog;

extern "C"
{
void ExceptionHandler()
{
    while (true)
    {
        ::etl::print("ExceptionHandler :(");
    }
}

void boardInit()
{
    /* Disables the watchdog early before the timeout occurs, which is then enabled later in the
       main function. */
    ::safety::bsp::Watchdog::disableWatchdog();
    configurPll();
    cacheEnable();
}

void setupApplicationsIsr(void)
{
#if 0
    // interrupts
    SYS_SetPriority(CAN0_ORed_0_15_MB_IRQn, 8);  // can0 buffer 0 - 15
    SYS_SetPriority(CAN0_ORed_16_31_MB_IRQn, 8); // can0 buffer 16 - 32

    SYS_EnableIRQ(CAN0_ORed_0_15_MB_IRQn);
    SYS_EnableIRQ(CAN0_ORed_16_31_MB_IRQn);

    ENABLE_INTERRUPTS();
#endif
}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

::etl::typed_storage<::systems::BspSystem> bspSystem;

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::systems::CanSystem> canSystem;
#endif // PLATFORM_SUPPORT_CAN

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
    if (level == 2U)
    {
#ifdef PLATFORM_SUPPORT_CAN
        lifecycleManager.addComponent("can", canSystem.create(TASK_CAN, staticBsp), level);
#endif // PLATFORM_SUPPORT_CAN
    }
}
} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

extern "C"
{
int main()
{
    stdio_init_all();
    ::etl::println("main()");
    ::safety::safeSupervisorConstructor.construct();
    auto& safeSupervisor = safety::SafeSupervisor::getInstance();
    safeSupervisor.enterLimpHome();
    uint32_t const TIME_OUT_WD_FAST_TEST_US = 6000U;
    auto const watchdogTest = ::safety::bsp::Watchdog::executeFastTest(TIME_OUT_WD_FAST_TEST_US);
    if (watchdogTest)
    {
        safeSupervisor.leaveLimpHome();
    }
    else
    {
        safeSupervisor.watchdogStartupCheckMonitor.trigger();
    }
    safeWatchdog.enableMcuWatchdog();
    ::platform::staticBsp.init();
    ::etl::println("app_main()");
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}

} // extern "C"
