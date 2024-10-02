// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_926AFEEE_3650_4951_BB77_8F960EC23323
#define GUARD_926AFEEE_3650_4951_BB77_8F960EC23323

#include <async/Async.h>

#include <cstddef>

namespace lifecycle
{
class ILifecycleComponentCallback;

class ILifecycleComponent
{
public:
    struct Transition
    {
        enum class Type : uint8_t
        {
            INIT     = 0,
            RUN      = 1,
            SHUTDOWN = 2
        };

        static size_t const COUNT = 3U;
    };

    /// Register the callback on which to call `transitionDone()`.
    ///
    /// When adding this component to a LifecycleManager, it uses this method to set itself as the
    /// callback.
    virtual void initCallback(ILifecycleComponentCallback& callback)               = 0;
    /// Transition context to be used for each transition.
    virtual ::async::ContextType getTransitionContext(Transition::Type transition) = 0;
    /// Perform a transition.
    ///
    /// Called by the LifecycleManager. Once the transition is completed, `transitionDone()` will be
    /// called on the callback
    virtual void startTransition(Transition::Type transition)                      = 0;

    virtual ~ILifecycleComponent() = default;

private:
    ILifecycleComponent& operator=(ILifecycleComponent const&) = delete;
};

} // namespace lifecycle

#endif // GUARD_926AFEEE_3650_4951_BB77_8F960EC23323
