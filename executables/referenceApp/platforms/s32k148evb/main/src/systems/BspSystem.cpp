// Copyright 2024 Accenture.

#include "systems/BspSystem.h"

#include "bsp/timer/SystemTimer.h"
#include "outputPwm/OutputPwm.h"

namespace systems
{

BspSystem::BspSystem(::async::ContextType const context, StaticBsp& staticBsp)
: ::lifecycle::SingleContextLifecycleComponent(context)
, _context(context)
, _staticBsp(staticBsp)
, _analogTester()
, _outputPwmTester()
, _digitalInputTester()
, _outputTester()
, _asyncCommandWrapper_for_analogTester(_analogTester, context)
, _asyncOutputPwmTester(_outputPwmTester, context)
, _asyncDigitalInputTester(_digitalInputTester, context)
, _asyncOutputTester(_outputTester, context)
{}

void BspSystem::init() { transitionDone(); }

void BspSystem::run()
{
    ::async::scheduleAtFixedRate(_context, *this, _timeout, 10u, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void BspSystem::shutdown()
{
    _timeout.cancel();
    transitionDone();
}

void BspSystem::execute()
{
    cyclic();
    _staticBsp.cyclic();
}

uint8_t bspSystemCycleCount = 0;

void BspSystem::cyclic() {}

} // namespace systems
