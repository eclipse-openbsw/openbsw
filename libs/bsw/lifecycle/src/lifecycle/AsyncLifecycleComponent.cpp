/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/AsyncLifecycleComponent.h"

namespace lifecycle
{
AsyncLifecycleComponent::AsyncLifecycleComponent()
{
    setTransitionContext(::async::CONTEXT_INVALID);
}

::async::ContextType
AsyncLifecycleComponent::getTransitionContext(Transition::Type const transition)
{
    return _contexts[static_cast<size_t>(transition)];
}

void AsyncLifecycleComponent::setTransitionContext(::async::ContextType const context)
{
    _contexts.fill(context);
}

void AsyncLifecycleComponent::setTransitionContext(
    Transition::Type const transition, ::async::ContextType const context)
{
    _contexts[static_cast<size_t>(transition)] = context;
}

} // namespace lifecycle
