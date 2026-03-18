// Copyright 2025 Accenture.

#include "app/app.h"

#include "bsp/uart/UartConfig.h"
#include "console/console.h"
#include "logger/logger.h"
#include "reset/softwareSystemReset.h"
#include "systems/RuntimeSystem.h"
#include "systems/SysAdminSystem.h"
#ifdef TRACING
#include "runtime/Tracer.h"
#endif

#include <async/AsyncBinding.h>
#include <etl/alignment.h>
#include <etl/singleton.h>
#include <lifecycle/LifecycleLogger.h>

#include <platform/estdint.h>

#include <cstdio>

namespace app
{
using bsp::Uart;
using ::util::logger::LIFECYCLE;
using ::util::logger::Logger;

using AsyncAdapter        = ::async::AsyncBinding::AdapterType;
using AsyncRuntimeMonitor = ::async::AsyncBinding::RuntimeMonitorType;
using AsyncContextHook    = ::async::AsyncBinding::ContextHookType;

constexpr size_t MaxNumComponents         = 16;
constexpr size_t MaxNumLevels             = 5;
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

private:
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
    printf("hello\r\n");
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

    /* runlevel 2 */
    lifecycleManager.addComponent(
        "sysadmin", sysAdminSystem.create(TASK_SYSADMIN, lifecycleManager), 2U);

    lifecycleManager.transitionToLevel(MaxNumLevels);

    runtimeMonitor.start();
    idleHandler.start();
}

using IdleTask = AsyncAdapter::IdleTask<1024 * 2>;
IdleTask idleTask{"idle"};

using TimerTask = AsyncAdapter::TimerTask<1024 * 1>;
TimerTask timerTask{"timer"};

using SysadminTask = AsyncAdapter::Task<TASK_SYSADMIN, 1024 * 2>;
SysadminTask sysadminTask{"sysadmin"};

using BspTask = AsyncAdapter::Task<TASK_BSP, 1024 * 2>;
BspTask bspTask{"bsp"};

using BackgroundTask = AsyncAdapter::Task<TASK_BACKGROUND, 1024 * 2>;
BackgroundTask backgroundTask{"background"};

AsyncContextHook contextHook{runtimeMonitor};

} // namespace app
