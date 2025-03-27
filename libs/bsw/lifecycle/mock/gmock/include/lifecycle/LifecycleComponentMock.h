// Copyright 2024 Accenture.

/**
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleComponent.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleComponentMock : public ILifecycleComponent
{
public:
    MOCK_METHOD1(initCallback, void(ILifecycleComponentCallback& callback));
    MOCK_METHOD1(getTransitionContext, ::async::ContextType(Transition::Type transition));
    MOCK_METHOD1(startTransition, void(Transition::Type transition));
};

} // namespace lifecycle
