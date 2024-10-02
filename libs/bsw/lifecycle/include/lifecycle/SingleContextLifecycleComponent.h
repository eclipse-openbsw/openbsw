// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_D884B0D0_5BF6_11EB_9102_8FC73864AB9A
#define GUARD_D884B0D0_5BF6_11EB_9102_8FC73864AB9A

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

    ~SingleContextLifecycleComponent() override = default;

private:
    ::async::ContextType const _context;
};

} // namespace lifecycle

#endif // GUARD_D884B0D0_5BF6_11EB_9102_8FC73864AB9A
