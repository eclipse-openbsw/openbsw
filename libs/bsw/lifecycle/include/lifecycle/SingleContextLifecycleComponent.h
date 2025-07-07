// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/LifecycleComponent.h"

namespace lifecycle
{
/// A LifecycleComponent base class that allows to specify a single async context in which all
/// transitions are run.
///
/// A component implementing `SingleContextLifecycleComponent` must provide implementations for the
/// three state transition functions `init`, `run` and `shutdown`. After registering a
/// LifecycleComponent to the LifecycleManager, the LifecycleManager will make sure that these are
/// executed at the appropriate time.
class SingleContextLifecycleComponent : public LifecycleComponent
{
public:
    explicit SingleContextLifecycleComponent(::async::ContextType const context) : _context(context)
    {}

    ::async::ContextType getTransitionContext(Transition::Type transition) override;

protected:
    ~SingleContextLifecycleComponent() = default;

private:
    ::async::ContextType const _context;
};

} // namespace lifecycle
