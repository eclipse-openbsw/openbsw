// Copyright 2025 Accenture.

#include "app/app.h"

#include "bsp/uart/UartConfig.h"
#include "console/console.h"
#include "lifecycle/StaticBsp.h"
#include "logger/logger.h"
#include "reset/softwareSystemReset.h"
#include "systems/DemoSystem.h"
#include "systems/RuntimeSystem.h"
#include "systems/SafetySystem.h"
#include "systems/SysAdminSystem.h"
#if defined(SUPPORT_FREERTOS) && defined(TRACING)
#include "runtime/Tracer.h"
#endif

#include <app/appConfig.h>
#include <etl/alignment.h>
#include <etl/print.h>
#include <etl/singleton.h>
#ifdef PLATFORM_SUPPORT_ROM_CHECK
#include <safeMemory/RomCheck.h>
#endif

#ifdef PLATFORM_SUPPORT_TRANSPORT
#include "busid/BusId.h"
#include "systems/TransportSystem.h"
#endif // PLATFORM_SUPPORT_TRANSPORT

#ifdef PLATFORM_SUPPORT_ETHERNET
#include "systems/EthernetSystem.h"
#ifdef PLATFORM_SUPPORT_TRANSPORT
#include "systems/DoIpServerSystem.h"
#endif // PLATFORM_SUPPORT_TRANSPORT
#endif // PLATFORM_SUPPORT_ETHERNET
       //
#if defined(PLATFORM_SUPPORT_CAN) && defined(PLATFORM_SUPPORT_TRANSPORT)
#include "systems/DoCanSystem.h"
#endif // defined(PLATFORM_SUPPORT_CAN) && defined(PLATFORM_SUPPORT_TRANSPORT)

#if defined(PLATFORM_SUPPORT_TRANSPORT) && defined(PLATFORM_SUPPORT_UDS)
#include "systems/UdsSystem.h"
#endif // defined(PLATFORM_SUPPORT_TRANSPORT) && defined(PLATFORM_SUPPORT_UDS)

#ifdef PLATFORM_SUPPORT_STORAGE
#include "systems/StorageSystem.h"
#endif // PLATFORM_SUPPORT_STORAGE

#include <async/AsyncBinding.h>
#include <lifecycle/LifecycleLogger.h>
#include <lifecycle/LifecycleManager.h>

#include <cstdio>

#ifdef BUILD_RUST
#include <rust_hello_world.h>
#endif

alignas(32)::async::internal::Stack<safety_task_stackSize> safetyStack;

#ifdef PLATFORM_SUPPORT_CAN
#include <systems/ICanSystem.h>

namespace platform
{
namespace systems
{
extern ::can::ICanSystem& getCanSystem();
} // namespace systems
} // namespace platform
#endif

#ifdef PLATFORM_SUPPORT_ETHERNET
#include <systems/IEthernetSystem.h>

namespace platform
{
namespace systems
{
extern ::ethernet::IEthernetSystem& getEthernetSystem();
} // namespace systems
} // namespace platform
#endif

#include <platform/estdint.h>

#ifdef PLATFORM_SUPPORT_ROM_CHECK
::safety::RomCheck RomCheck;
#endif

namespace platform
{
// TODO: move declaration to header file
extern void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t level);
extern StaticBsp& getStaticBsp();
} // namespace platform

namespace app
{
using bsp::Uart;
using ::util::logger::LIFECYCLE;
using ::util::logger::Logger;

using AsyncAdapter        = ::async::AsyncBinding::AdapterType;
using AsyncRuntimeMonitor = ::async::AsyncBinding::RuntimeMonitorType;
using AsyncContextHook    = ::async::AsyncBinding::ContextHookType;

constexpr size_t MaxNumComponents         = 16;
constexpr size_t MaxNumLevels             = 9;
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
::etl::typed_storage<::systems::SysAdminSystem> sysAdminSystem;
::etl::typed_storage<::systems::DemoSystem> demoSystem;
::etl::typed_storage<::systems::SafetySystem> safetySystem;
#ifdef PLATFORM_SUPPORT_ETHERNET
::etl::typed_storage<::systems::EthernetSystem> ethernetSystem;
#endif // PLATFORM_SUPPORT_ETHERNET

#ifdef PLATFORM_SUPPORT_TRANSPORT
::etl::typed_storage<::transport::TransportSystem> transportSystem;
#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::docan::DoCanSystem> doCanSystem;
#endif // PLATFORM_SUPPORT_CAN
#ifdef PLATFORM_SUPPORT_ETHERNET
::etl::typed_storage<::doip::DoIpServerSystem> doipServerSystem;
#endif // PLATFORM_SUPPORT_ETHERNET
#endif // PLATFORM_SUPPORT_TRANSPORT

#if defined(PLATFORM_SUPPORT_TRANSPORT) and defined(PLATFORM_SUPPORT_UDS)
::etl::typed_storage<::uds::UdsSystem> udsSystem;
#endif // defined(PLATFORM_SUPPORT_TRANSPORT) and defined(PLATFORM_SUPPORT_UDS)

#ifdef PLATFORM_SUPPORT_STORAGE
::etl::typed_storage<::systems::StorageSystem> storageSystem;
#endif

class LifecycleMonitor : private ::lifecycle::ILifecycleListener
{
public:
    explicit LifecycleMonitor(LifecycleManager& manager) { manager.addLifecycleListener(*this); }

    bool isReadyForReset() const { return _isReadyForReset; }

private:
    void lifecycleLevelReached(
        uint8_t const level,
        ::lifecycle::ILifecycleComponent::Transition::Type const /* transition */) override
    {
        if (0 == level)
        {
            _isReadyForReset = true;
        }
    }

    bool _isReadyForReset = false;
};

LifecycleMonitor lifecycleMonitor(lifecycleManager);

class IdleHandler : private ::async::RunnableType
{
public:
    void init()
    {
        ::logger::init();

        ::console::init();
        ::console::enable();
    }

    void start() { ::async::execute(AsyncAdapter::TASK_IDLE, *this); }

private:
    void execute() override
    {
        ::logger::run();
        ::console::run();
#ifdef PLATFORM_SUPPORT_ROM_CHECK
        RomCheck.idle();
#endif
        if (lifecycleMonitor.isReadyForReset())
        {
            shutdown();
        }
        else
        {
            ::async::execute(AsyncAdapter::TASK_IDLE, *this);
        }
    }

    void shutdown()
    {
        Logger::info(LIFECYCLE, "Lifecycle shutdown complete");
        ::logger::flush();

        softwareSystemReset();
    }
};

IdleHandler idleHandler;

void startApp();

void staticInit()
{
    Uart::getInstance(Uart::Id::TERMINAL).init();
    Uart::getInstance(Uart::Id::TERMINAL).waitForTxReady();
}

void run()
{
    staticInit();
    etl::print("hello\r\n");
#ifdef BUILD_RUST
    etl::print("Hello Rust!\r\n");
    etl::print("Rust add(3, 4) = {}\r\n", static_cast<unsigned int>(rust_add(3U, 4U)));
#endif
    idleHandler.init();
    AsyncAdapter::run(AsyncAdapter::StartAppFunctionType::create<&startApp>());
}

void startApp()
{
#if TRACING
    runtime::Tracer::init();
    runtime::Tracer::start();
#endif

    /* runlevel 1 */
    ::platform::platformLifecycleAdd(lifecycleManager, 1U);

    lifecycleManager.addComponent(
        "runtime", runtimeSystem.create(TASK_BACKGROUND, runtimeMonitor), 1U);
    lifecycleManager.addComponent("safety", safetySystem.create(TASK_SAFETY, lifecycleManager), 1U);

    /* runlevel 2 */
    ::platform::platformLifecycleAdd(lifecycleManager, 2U);

    /* runlevel 3 */
    ::platform::platformLifecycleAdd(lifecycleManager, 3U);

    /* runlevel 4 */
#ifdef PLATFORM_SUPPORT_TRANSPORT
    lifecycleManager.addComponent("transport", transportSystem.create(TASK_UDS), 4U);
#endif

    /* runlevel 5 */
#if defined(PLATFORM_SUPPORT_TRANSPORT) && defined(PLATFORM_SUPPORT_CAN)
    lifecycleManager.addComponent(
        "docan", doCanSystem.create(*transportSystem, ::platform::systems::getCanSystem(), TASK_CAN), 5U);
#endif

#ifdef PLATFORM_SUPPORT_ETHERNET
    lifecycleManager.addComponent(
        "ethernet", ethernetSystem.create(TASK_ETHERNET, ::platform::systems::getEthernetSystem()), 5U);
#endif

#ifdef PLATFORM_SUPPORT_STORAGE
    lifecycleManager.addComponent(
        "storage",
        storageSystem.create(TASK_BSP, TASK_DEMO, ::platform::getStaticBsp().getEepromDriver()),
        5U);
#endif

    /* runlevel 6 */
#if defined(PLATFORM_SUPPORT_ETHERNET) && defined(PLATFORM_SUPPORT_TRANSPORT)
    lifecycleManager.addComponent(
        "doipServer",
        doipServerSystem.create(
            *transportSystem,
            ethernetSystem->netifConfigRegistry,
            TASK_ETHERNET,
            ::busid::ETH_0,
            LOGICAL_ADDRESS,
            ::ethX::MAC_ADDRESS,
            ethernetSystem->netifs.networkInterfaceConfigsIp4[0].broadcastAddress()), // ETH0
        6U);
#endif

    /* runlevel 7 */
#if defined(PLATFORM_SUPPORT_TRANSPORT) && defined(PLATFORM_SUPPORT_UDS)
    lifecycleManager.addComponent(
        "uds", udsSystem.create(lifecycleManager, *transportSystem, TASK_UDS, LOGICAL_ADDRESS), 7U);
#endif

    /* runlevel 8 */
    lifecycleManager.addComponent(
        "sysadmin", sysAdminSystem.create(TASK_SYSADMIN, lifecycleManager), 8U);

    /* runlevel 9 */
    ::platform::platformLifecycleAdd(lifecycleManager, 9U);
    // clang-format off
    lifecycleManager.addComponent(
        "demo",
        demoSystem.create(
            TASK_DEMO,
            lifecycleManager
#ifdef PLATFORM_SUPPORT_CAN
            , ::platform::systems::getCanSystem()
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
            , (*storageSystem).getStorage()
#endif
        ),
        9U);
    // clang-format on

    lifecycleManager.transitionToLevel(MaxNumLevels);

    runtimeMonitor.start();
    idleHandler.start();
}

using TimerTask = AsyncAdapter::TimerTask<1024 * 1>;
TimerTask timerTask{"timer"};

using IdleTask = AsyncAdapter::IdleTask<1024 * 2>;
IdleTask idleTask{"idle"};

using UdsTask = AsyncAdapter::Task<TASK_UDS, 1024 * 2>;
UdsTask udsTask{"uds"};

using SysadminTask = AsyncAdapter::Task<TASK_SYSADMIN, 1024 * 2>;
SysadminTask sysadminTask{"sysadmin"};

using CanTask = AsyncAdapter::Task<TASK_CAN, 1024 * 2>;
CanTask canTask{"can"};

using EthernetTask = AsyncAdapter::Task<TASK_ETHERNET, 1024 * 2>;
EthernetTask ethernetTask{"ethernet"};

using BspTask = AsyncAdapter::Task<TASK_BSP, 1024 * 2>;
BspTask bspTask{"bsp"};

using DemoTask = AsyncAdapter::Task<TASK_DEMO, 1024 * 2>;
DemoTask demoTask{"demo"};

using BackgroundTask = AsyncAdapter::Task<TASK_BACKGROUND, 1024 * 2>;
BackgroundTask backgroundTask{"background"};

using SafetyTask = AsyncAdapter::TaskStack<TASK_SAFETY>;
SafetyTask safetyTask{"safety", safetyStack};

AsyncContextHook contextHook{runtimeMonitor};

} // namespace app
