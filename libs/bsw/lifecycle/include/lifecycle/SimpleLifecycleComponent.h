// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/LifecycleComponent.h"

namespace lifecycle
{
/// A LifecycleComponent base class where all transitions will be run in the LifecycleManager's
/// async context.
///
/// A component implementing `SimpleLifecycleComponent` must provide implementations for the three
/// state transition functions `init`, `run` and `shutdown`. After registering a LifecycleComponent
/// to the LifecycleManager, the LifecycleManager will make sure that these are executed at the
/// appropriate time.
class SimpleLifecycleComponent : public LifecycleComponent
{
public:
    /// The transition context of this SimpleLifecycleComponent.
    ///
    /// Returns CONTEXT_INVALID, which means that the LifecycleManager will run all transitions in
    /// its own context.
    ::async::ContextType getTransitionContext(Transition::Type transition) override;
    ~SimpleLifecycleComponent() override = default;

private:
    SimpleLifecycleComponent& operator=(SimpleLifecycleComponent const&) = delete;
};

} // namespace lifecycle

