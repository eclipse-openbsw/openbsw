// Copyright 2024 Accenture.

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
