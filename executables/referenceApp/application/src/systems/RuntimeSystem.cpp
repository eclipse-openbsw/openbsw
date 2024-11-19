// Copyright 2024 Accenture.

#include "systems/RuntimeSystem.h"

namespace
{
constexpr uint32_t SYSTEM_CYCLE_TIME = 1000;
}

namespace systems
{

RuntimeSystem::RuntimeSystem(
    ::async::ContextType const context, ::async::AsyncBinding::RuntimeMonitorType& runtimeMonitor)
: _context(context)
, _timeout()
, _statisticsCommand(runtimeMonitor)
, _asyncCommandWrapperForStatisticsCommand(_statisticsCommand, context)
{
    setTransitionContext(context);
}

void RuntimeSystem::init()
{
    _statisticsCommand.setTicksPerUs(0); // use hardware ticks rate

    transitionDone();
}

void RuntimeSystem::run()
{
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, SYSTEM_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void RuntimeSystem::shutdown()
{
    _timeout.cancel();

    transitionDone();
}

void RuntimeSystem::execute() { _statisticsCommand.cyclic_1000ms(); }

} // namespace systems
