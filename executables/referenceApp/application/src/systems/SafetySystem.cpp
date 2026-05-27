/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/SafetySystem.h"

#include <safeLifecycle/SafetyManager.h>
#ifdef PLATFORM_SUPPORT_WATCHDOG
#include <watchdog/Watchdog.h>
#endif

::safety::SafetyManager safetyManager;

namespace
{
constexpr uint32_t SYSTEM_CYCLE_TIME = 10;
}

namespace systems
{

SafetySystem::SafetySystem(::async::ContextType const context, ::lifecycle::ILifecycleManager&)
: _context(context)
, _timeout()
#ifdef PLATFORM_SUPPORT_IO
, _safetyCommand()
, _asyncCommandWrapperForSafetyCommand(_safetyCommand, context)
#endif
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
    ::safety::bsp::Watchdog::disableWatchdog();
#endif

    transitionDone();
}

void SafetySystem::execute() { cyclic(); }

void SafetySystem::cyclic() { safetyManager.cyclic(); }

} // namespace systems
