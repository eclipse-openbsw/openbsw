// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_411C64C8_772E_4E3F_B36C_98DDE7692D3A
#define GUARD_411C64C8_772E_4E3F_B36C_98DDE7692D3A

#include "lifecycle/LifecycleComponent.h"

#include <estd/array.h>

namespace lifecycle
{
/// A LifecycleComponent base class that allows to specify an async context for each transition to
/// run in.
///
/// A component implementing `AsyncLifecycleComponent` must provide implementations for the three
/// state transition functions `init`, `run` and `shutdown`. After registering a LifecycleComponent
/// to the LifecycleManager, the LifecycleManager will make sure that these are executed at the
/// appropriate time.
class AsyncLifecycleComponent : public LifecycleComponent
{
public:
    AsyncLifecycleComponent();

    AsyncLifecycleComponent(AsyncLifecycleComponent const&)            = delete;
    AsyncLifecycleComponent& operator=(AsyncLifecycleComponent const&) = delete;

    ::async::ContextType getTransitionContext(Transition::Type transition) override;

    ~AsyncLifecycleComponent() override = default;

protected:
    /// Set the async `context` for all transitions to be run in.
    void setTransitionContext(::async::ContextType context);
    /// Set the async `context` for a single transitions of a single type to be run in.
    void setTransitionContext(Transition::Type transition, ::async::ContextType context);

private:
    ::estd::array<::async::ContextType, Transition::COUNT> _contexts{};
};

} // namespace lifecycle

#endif // GUARD_411C64C8_772E_4E3F_B36C_98DDE7692D3A
