// Copyright 2025 Accenture.

#include "systems/BspSystem.h"

#include <etl/alignment.h>

#include <cinttypes>
#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif
#ifdef PLATFORM_SUPPORT_ETHERNET
#include "systems/S32K148EvbEthernetSystem.h"
#endif
#include "cache/cache.h"
#include "clock/clockConfig.h"
#include "commonDebug.h"
#include "interrupt_manager.h"
#include "lifecycle/StaticBsp.h"
#include "systems/IEthernetDriverSystem.h"
#include "watchdog/Watchdog.h"

#include <lifecycle/LifecycleManager.h>
#include <safeLifecycle/SafeSupervisor.h>
#include <safeWatchdog/SafeWatchdog.h>
#include <watchdog/Watchdog.h>
#include <watchdogManager/watchdogManager.h>

extern void app_main();

extern safety::SafeWatchdog safeWatchdog;

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
    /* Disables the watchdog early before the timeout occurs, which is then enabled later in the
       main function. */
    bsp::Watchdog::disableWatchdog();
    configurPll();
    cacheEnable();
}

void setupApplicationsIsr(void)
{
    // interrupts
    SYS_SetPriority(CAN0_ORed_0_15_MB_IRQn, 8);  // can0 buffer 0 - 15
    SYS_SetPriority(CAN0_ORed_16_31_MB_IRQn, 8); // can0 buffer 16 - 32

    SYS_EnableIRQ(CAN0_ORed_0_15_MB_IRQn);
    SYS_EnableIRQ(CAN0_ORed_16_31_MB_IRQn);

    // Data transfer done, Transmit Buffer Done for Ring/Queue 0, Transmit Frame Done for Ring/Queue
    // 0
    SYS_SetPriority(ENET_TX_Buffer_IRQn, 9);
    // Receive Buffer Done for Ring/Queue 0, Receive Frame Done for Ring/Queue 0
    SYS_SetPriority(ENET_RX_Buffer_IRQn, 9);
    // Payload receive error, Collision retry limit reached, Late collision detected, AXI Bus Error
    // detected, Babbling transmit error, Babbling receive error, Transmit FIFO underrun
    //    SYS_SetPriority(ENET_PRE_IRQn, 9);
    SYS_EnableIRQ(ENET_TX_Buffer_IRQn);
    SYS_EnableIRQ(ENET_RX_Buffer_IRQn);
    //    SYS_EnableIRQ(ENET_PRE_IRQn);
    ENABLE_INTERRUPTS();
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

#ifdef PLATFORM_SUPPORT_ETHERNET
::etl::typed_storage<::systems::S32K148EvbEthernetSystem> s32kEthernetSystem;
#endif // PLATFORM_SUPPORT_ETHERNET

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
#ifdef PLATFORM_SUPPORT_ETHERNET
        lifecycleManager.addComponent(
            "s32k-eth", s32kEthernetSystem.create(TASK_ETHERNET, lifecycleManager), level);
#endif // PLATFORM_SUPPORT_ETHERNET
    }
}
} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

#ifdef PLATFORM_SUPPORT_ETHERNET
namespace systems
{
::ethernet::IEthernetDriverSystem& getEthernetSystem() { return *::platform::s32kEthernetSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_ETHERNET

int main()
{
    auto& safeSupervisor = safety::SafeSupervisor::getInstance();
    safeSupervisor.enterLimpHome();
    bool watchdogTest = safety::WatchdogManager::startTest();
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
    printf("main(RCM::SRS 0x%" PRIx32 ")\r\n", *reinterpret_cast<uint32_t volatile*>(0x4007F008));
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
