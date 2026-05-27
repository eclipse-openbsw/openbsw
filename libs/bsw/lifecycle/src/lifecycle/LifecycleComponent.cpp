/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/LifecycleComponent.h"

#include "lifecycle/ILifecycleComponentCallback.h"

namespace lifecycle
{
void LifecycleComponent::initCallback(ILifecycleComponentCallback& callback)
{
    // Lifetime is guaranteed from outside
    _callback = &callback;
}

void LifecycleComponent::transitionDone()
{
    if (_callback != nullptr)
    {
        _callback->transitionDone(*this);
    }
}

void LifecycleComponent::startTransition(Transition::Type const transition)
{
    switch (transition)
    {
        case Transition::Type::INIT:
        {
            init();
            break;
        }
        case Transition::Type::RUN:
        {
            run();
            break;
        }
        // case Transition::SHUTDOWN:
        default:
        {
            shutdown();
            break;
        }
    }
}

} // namespace lifecycle
