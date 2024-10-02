// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_8F9082A6_3076_437F_B927_FDBE7F31EA4F
#define GUARD_8F9082A6_3076_437F_B927_FDBE7F31EA4F

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

#endif // GUARD_8F9082A6_3076_437F_B927_FDBE7F31EA4F
