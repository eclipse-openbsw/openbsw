// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleListener.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleListenerMock : public ILifecycleListener
{
public:
    MOCK_METHOD2(
        lifecycleLevelReached,
        void(uint8_t level, ILifecycleComponent::Transition::Type transition));
};

} // namespace lifecycle
