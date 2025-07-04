// Copyright 2025 Accenture.

#include "systems/BackgroundSystem.h"

#include "app/BackgroundLogger.h"
#include "console/console.h"
#include "logger/logger.h"
#include "reset/softwareSystemReset.h"

#include <async/AsyncBinding.h>
#include <bsp/SystemTime.h>
#include <lifecycle/ILifecycleManager.h>

namespace
{
constexpr uint32_t BACKGROUND_CYCLE_TIME = 10;
}

namespace systems
{
using ::util::logger::BACKGROUND;
using ::util::logger::Logger;

BackgroundSystem::BackgroundSystem(
    ::async::ContextType const context, ::lifecycle::ILifecycleManager& lifecycleManager)
: _context(context)
{
    lifecycleManager.addLifecycleListener(*this);
    setTransitionContext(context);
}

void BackgroundSystem::init() { transitionDone(); }

void BackgroundSystem::run()
{
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, BACKGROUND_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void BackgroundSystem::shutdown()
{
    _timeout.cancel();
    transitionDone();
}

void BackgroundSystem::execute() { cyclic(); }

void BackgroundSystem::cyclic()
{
    ::logger::run();
    ::console::run();
}

void BackgroundSystem::lifecycleLevelReached(
    uint8_t const level, ::lifecycle::ILifecycleComponent::Transition::Type const /* transition */)
{
    if (0 == level)
    {
        Logger::info(BACKGROUND, "Lifecycle shutdown complete");
        ::logger::flush();
        ::softwareSystemReset();
    }
}

} // namespace systems
