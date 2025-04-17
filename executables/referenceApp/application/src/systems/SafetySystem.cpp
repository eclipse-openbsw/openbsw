// Copyright 2024 Accenture.

#include "systems/SafetySystem.h"

#include <safeLifecycle/SafetyLogger.h>
#include <safeLifecycle/SafetyManager.h>
#ifdef PLATFORM_SUPPORT_WATCHDOG
#include "watchdog/Watchdog.h"
#endif

::safety::SafetyManager safetyManager;

namespace
{
constexpr uint32_t SYSTEM_CYCLE_TIME = 10;
}

namespace systems
{
using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafetySystem::SafetySystem(
    ::async::ContextType const context, ::lifecycle::ILifecycleManager& lifecycleManager)
: _context(context)
, _timeout()
, _lifecycleControlCommand(lifecycleManager)
, _asyncCommandWrapperForLifecycleControlCommand(_lifecycleControlCommand, context)
{
    setTransitionContext(context);
}

void SafetySystem::init()
{
    safetyManager.init();
    transitionDone();
}

void SafetySystem::run()
{
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, SYSTEM_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void SafetySystem::shutdown()
{
    _timeout.cancel();
#ifdef PLATFORM_SUPPORT_WATCHDOG
    bsp::Watchdog::disableWatchdog();
#endif

    transitionDone();
}

void SafetySystem::execute() { cyclic(); }

void SafetySystem::cyclic() { safetyManager.cyclic(); }

} // namespace systems
