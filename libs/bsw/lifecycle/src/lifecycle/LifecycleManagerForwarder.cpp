// Copyright 2024 Accenture.

#include "lifecycle/LifecycleManagerForwarder.h"

namespace lifecycle
{

void LifecycleManagerForwarder::init(ILifecycleManager& lifecycleManager)
{
    _lifecycleManager = &lifecycleManager;
}

uint8_t LifecycleManagerForwarder::getLevelCount() const
{
    return _lifecycleManager != nullptr ? _lifecycleManager->getLevelCount() : 0U;
}

void LifecycleManagerForwarder::transitionToLevel(uint8_t const level)
{
    if (_lifecycleManager != nullptr)
    {
        _lifecycleManager->transitionToLevel(level);
    }
}

void LifecycleManagerForwarder::addLifecycleListener(ILifecycleListener& listener)
{
    if (_lifecycleManager != nullptr)
    {
        _lifecycleManager->addLifecycleListener(listener);
    }
}

void LifecycleManagerForwarder::removeLifecycleListener(ILifecycleListener& listener)
{
    if (_lifecycleManager != nullptr)
    {
        _lifecycleManager->removeLifecycleListener(listener);
    }
}

} // namespace lifecycle
