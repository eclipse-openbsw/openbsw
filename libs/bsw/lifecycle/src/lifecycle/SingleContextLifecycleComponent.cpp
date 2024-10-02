// Copyright 2024 Accenture.

#include "lifecycle/SingleContextLifecycleComponent.h"

namespace lifecycle
{
::async::ContextType
SingleContextLifecycleComponent::getTransitionContext(Transition::Type /*transition*/)
{
    return _context;
}

} // namespace lifecycle
