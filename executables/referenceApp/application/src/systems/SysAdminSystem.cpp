/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/SysAdminSystem.h"

namespace
{
constexpr uint32_t SYSTEM_CYCLE_TIME = 10;
}

namespace systems
{

SysAdminSystem::SysAdminSystem(
    ::async::ContextType const context, ::lifecycle::ILifecycleManager& lifecycleManager)
: _context(context)
, _timeout()
, _lifecycleControlCommand(lifecycleManager)
, _asyncCommandWrapperForLifecycleControlCommand(_lifecycleControlCommand, context)
{
    setTransitionContext(context);
}

void SysAdminSystem::init() { transitionDone(); }

void SysAdminSystem::run()
{
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, SYSTEM_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void SysAdminSystem::shutdown()
{
    _timeout.cancel();

    transitionDone();
}

void SysAdminSystem::execute()
{
    //
}

} // namespace systems
