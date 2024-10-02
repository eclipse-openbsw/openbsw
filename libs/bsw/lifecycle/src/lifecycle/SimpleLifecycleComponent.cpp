// Copyright 2024 Accenture.

#include "lifecycle/SimpleLifecycleComponent.h"

namespace lifecycle
{
::async::ContextType SimpleLifecycleComponent::getTransitionContext(Transition::Type /*transition*/)
{
    return ::async::CONTEXT_INVALID;
}

} // namespace lifecycle
