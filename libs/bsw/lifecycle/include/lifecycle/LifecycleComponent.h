// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleComponent.h"

namespace lifecycle
{
/// A more convenient base class for implementing a lifecycle component.
///
/// A component implementing `LifecycleComponent` must provide implementations for the three state
/// transition functions `init`, `run` and `shutdown`. After registering a LifecycleComponent to the
/// LifecycleManager, the LifecycleManager will make sure that these are executed at the appropriate
/// time.
///
/// A component must also provide an implementation of `getTransitionContext()`. There are several
/// classes that should be used for implementing a lifecycle component, `AsyncLifecycleComponent`,
/// `SingleContextLifecycleComponent` and `SimpleLifecycleComponent`. They handle the most common
/// async context use cases and implement `getTransitionContext()` accordingly.
class LifecycleComponent : public ILifecycleComponent
{
public:
    LifecycleComponent() = default;

    /// See ILifecycleComponentCallback::initCallback()
    void initCallback(ILifecycleComponentCallback& callback) override;
    /// See ILifecycleComponentCallback::startTransition()
    void startTransition(Transition::Type transition) override;

    ~LifecycleComponent() override = default;

protected:
    void transitionDone();

    virtual void init()     = 0;
    virtual void run()      = 0;
    virtual void shutdown() = 0;

private:
    ILifecycleComponentCallback* _callback                   = nullptr;
    LifecycleComponent& operator=(LifecycleComponent const&) = delete;
};

} // namespace lifecycle

