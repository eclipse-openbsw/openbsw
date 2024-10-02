// Copyright 2024 Accenture.

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
