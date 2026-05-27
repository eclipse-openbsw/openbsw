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

#include "lifecycle/LifecycleComponent.h"

#include <etl/array.h>

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

protected:
    ~AsyncLifecycleComponent() = default;

    /// Set the async `context` for all transitions to be run in.
    void setTransitionContext(::async::ContextType context);
    /// Set the async `context` for a single transitions of a single type to be run in.
    void setTransitionContext(Transition::Type transition, ::async::ContextType context);

private:
    ::etl::array<::async::ContextType, Transition::COUNT> _contexts{};
};

} // namespace lifecycle
