/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include <async/Async.h>

#include <cstddef>
#include <cstdint>

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

    ILifecycleComponent& operator=(ILifecycleComponent const&) = delete;

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

protected:
    ~ILifecycleComponent() = default;
};

} // namespace lifecycle
