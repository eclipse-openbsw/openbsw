// Copyright 2025 Accenture.

#include "app/app.h"
#include "console/console.h"
#include "logger/logger.h"
#include "systems/BackgroundSystem.h"
#include "systems/DemoSystem.h"
#include "systems/RuntimeSystem.h"
#include "systems/SafetySystem.h"
#include "systems/SysAdminSystem.h"

#include <app/appConfig.h>
#include <etl/alignment.h>
#include <etl/singleton.h>

#ifdef PLATFORM_SUPPORT_UDS
#include "busid/BusId.h"
#include "systems/TransportSystem.h"
#include "systems/UdsSystem.h"
#endif // PLATFORM_SUPPORT_UDS

#ifdef PLATFORM_SUPPORT_CAN
#include "systems/DoCanSystem.h"
#endif // PLATFORM_SUPPORT_CAN

#include <async/AsyncBinding.h>
#include <lifecycle/LifecycleLogger.h>
#include <lifecycle/LifecycleManager.h>

#include <cstdio>

#ifdef PLATFORM_SUPPORT_CAN
#include <systems/ICanSystem.h>

namespace systems
{
extern ::can::ICanSystem& getCanSystem();
} // namespace systems
#endif

#include <platform/estdint.h>

namespace platform
{
// TODO: move declaration to header file
extern void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t level);
} // namespace platform

namespace app
{
using ::util::logger::LIFECYCLE;
using ::util::logger::Logger;

using AsyncAdapter        = ::async::AsyncBinding::AdapterType;
using AsyncRuntimeMonitor = ::async::AsyncBinding::RuntimeMonitorType;
using AsyncContextHook    = ::async::AsyncBinding::ContextHookType;

constexpr size_t MaxNumComponents         = 16;
constexpr size_t MaxNumLevels             = 8;
constexpr size_t MaxNumComponentsPerLevel = MaxNumComponents;

using LifecycleManager = ::lifecycle::declare::
    LifecycleManager<MaxNumComponents, MaxNumLevels, MaxNumComponentsPerLevel>;

char const* const isrGroupNames[ISR_GROUP_COUNT] = {"test"};

AsyncRuntimeMonitor runtimeMonitor{
    AsyncContextHook::InstanceType::GetNameType::create<&AsyncAdapter::getTaskName>(),
    isrGroupNames};

LifecycleManager lifecycleManager{
    TASK_SYSADMIN,
    ::lifecycle::LifecycleManager::GetTimestampType::create<&getSystemTimeUs32Bit>()};

::etl::typed_storage<::systems::RuntimeSystem> runtimeSystem;
::etl::typed_storage<::systems::BackgroundSystem> backgroundSystem;
::etl::typed_storage<::systems::SafetySystem> safetySystem;
::etl::typed_storage<::systems::SysAdminSystem> sysAdminSystem;
::etl::typed_storage<::systems::DemoSystem> demoSystem;

#ifdef PLATFORM_SUPPORT_UDS
::etl::typed_storage<::transport::TransportSystem> transportSystem;
#endif

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::docan::DoCanSystem> doCanSystem;
#endif

#ifdef PLATFORM_SUPPORT_UDS
::etl::typed_storage<::uds::UdsSystem> udsSystem;
#endif

void staticInit()
{
    ::logger::init();

    ::console::init();
    ::console::enable();
}

void run()
{
    printf("hello\r\n");
    staticInit();
    tx_kernel_enter();
}

extern "C"
{
void tx_application_define(void* first_unused_memory)
{
    (void)first_unused_memory;

    AsyncAdapter::init();

    /* runlevel 1 */
    lifecycleManager.addComponent(
        "background", backgroundSystem.create(TASK_BACKGROUND, lifecycleManager), 1U);

    /* runlevel 2 */
    ::platform::platformLifecycleAdd(lifecycleManager, 2U);
    lifecycleManager.addComponent(
        "runtime", runtimeSystem.create(TASK_BACKGROUND, runtimeMonitor), 2U);

    /* runlevel 3 */
    ::platform::platformLifecycleAdd(lifecycleManager, 3U);

    /* runlevel 4 */
#ifdef PLATFORM_SUPPORT_UDS
    lifecycleManager.addComponent("transport", transportSystem.create(TASK_UDS), 4U);
#endif

    /* runlevel 5 */
#ifdef PLATFORM_SUPPORT_CAN
    lifecycleManager.addComponent(
        "docan", doCanSystem.create(*transportSystem, ::systems::getCanSystem(), TASK_CAN), 5U);
#endif

    /* runlevel 6 */
#ifdef PLATFORM_SUPPORT_UDS
    lifecycleManager.addComponent(
        "uds", udsSystem.create(lifecycleManager, *transportSystem, TASK_UDS, LOGICAL_ADDRESS), 6U);
#endif

    /* runlevel 7 */
    lifecycleManager.addComponent(
        "sysadmin", sysAdminSystem.create(TASK_SYSADMIN, lifecycleManager), 7U);

    /* runlevel 8 */
    ::platform::platformLifecycleAdd(lifecycleManager, 8U);
    lifecycleManager.addComponent(
        "demo",
        demoSystem.create(
            TASK_DEMO,
            lifecycleManager
#ifdef PLATFORM_SUPPORT_CAN
            ,
            ::systems::getCanSystem()
#endif
                ),
        8U);

    lifecycleManager.addComponent("safety", safetySystem.create(TASK_SAFETY, lifecycleManager), 8U);

    lifecycleManager.transitionToLevel(MaxNumLevels);

    runtimeMonitor.start();
    AsyncAdapter::run();
}
}

using UdsTask = AsyncAdapter::Task<TASK_UDS, 1024 * 2>;
UdsTask udsTask{"uds"};

using CanTask = AsyncAdapter::Task<TASK_CAN, 1024 * 2>;
CanTask canTask{"can"};

using BspTask = AsyncAdapter::Task<TASK_BSP, 1024 * 2>;
BspTask bspTask{"bsp"};

using SysadminTask = AsyncAdapter::Task<TASK_SYSADMIN, 1024 * 2>;
SysadminTask sysadminTask{"sysadmin"};

using DemoTask = AsyncAdapter::Task<TASK_DEMO, 1024 * 2>;
DemoTask demoTask{"demo"};

using SafetyTask = AsyncAdapter::Task<TASK_SAFETY, 1024 * 2>;
SafetyTask safetyTask{"safety"};

using BackgroundTask = AsyncAdapter::Task<TASK_BACKGROUND, 1024 * 2>;
BackgroundTask backgroundTask{"background"};

AsyncContextHook contextHook{runtimeMonitor};

} // namespace app
