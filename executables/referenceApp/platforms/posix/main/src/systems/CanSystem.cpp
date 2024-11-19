// Copyright 2024 Accenture.

#include "systems/CanSystem.h"

#include <can/CanLogger.h>

namespace systems
{
static uint32_t const TIMEOUT_CAN_SYSTEM_IN_MS = 1U;
static int const MAX_SENT_PER_RUN              = 3;
static int const MAX_RECEIVED_PER_RUN          = 3;

using ::util::logger::CAN;
using ::util::logger::Logger;

namespace
{

::can::SocketCanTransceiver::DeviceConfig canConfig{"vcan0", ::busid::CAN_0};

} // namespace

CanSystem::CanSystem(::async::ContextType context)
: _timeout(), _context(context), _canTransceiver(canConfig)
{
    setTransitionContext(context);
}

void CanSystem::init() { transitionDone(); }

void CanSystem::run()
{
    _canTransceiver.init();
    _canTransceiver.open();
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, TIMEOUT_CAN_SYSTEM_IN_MS, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void CanSystem::shutdown()
{
    _timeout.cancel();
    _canTransceiver.close();
    _canTransceiver.shutdown();
    transitionDone();
}

::can::ICanTransceiver* CanSystem::getCanTransceiver(uint8_t busId)
{
    if (busId == ::busid::CAN_0)
    {
        return &_canTransceiver;
    }
    return nullptr;
}

void CanSystem::execute() { _canTransceiver.run(MAX_SENT_PER_RUN, MAX_RECEIVED_PER_RUN); }

} // namespace systems
