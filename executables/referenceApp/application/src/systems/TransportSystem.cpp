/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/TransportSystem.h"

#include <lifecycle/ILifecycleManager.h>

#include <platform/estdint.h>

namespace transport
{
TransportSystem::TransportSystem(::async::ContextType transitionContext)
: ::etl::singleton_base<TransportSystem>(*this)
{
    // Tell the lifecycle manager in which context to execute init/run/shutdown
    setTransitionContext(transitionContext);
}

char const* TransportSystem::getName() const { return "Transport"; }

void TransportSystem::init()
{
    _transportRouter.init();

    // Inform the lifecycle manager that the transition has been completed
    transitionDone();
}

void TransportSystem::run()
{
    // Inform the lifecycle manager that the transition has been completed
    transitionDone();
}

void TransportSystem::shutdown()
{
    // Inform the lifecycle manager that the transition has been completed
    transitionDone();
}

void TransportSystem::dump() const {}

void TransportSystem::addTransportLayer(AbstractTransportLayer& layer)
{
    _transportRouter.addTransportLayer(layer);
}

void TransportSystem::removeTransportLayer(AbstractTransportLayer& layer)
{
    _transportRouter.removeTransportLayer(layer);
}

ITransportMessageProvider& TransportSystem::getTransportMessageProvider()
{
    return _transportRouter;
}

} // namespace transport
