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
    MOCK_METHOD(void, initCallback, (ILifecycleComponentCallback & callback), (override));
    MOCK_METHOD(
        ::async::ContextType, getTransitionContext, (Transition::Type transition), (override));
    MOCK_METHOD(void, startTransition, (Transition::Type transition), (override));
};

} // namespace lifecycle
