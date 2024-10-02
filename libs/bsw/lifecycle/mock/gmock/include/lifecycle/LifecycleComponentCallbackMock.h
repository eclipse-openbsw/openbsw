// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_A925CAF1_D979_497B_89E9_9CA3A0B039A6
#define GUARD_A925CAF1_D979_497B_89E9_9CA3A0B039A6

#include "lifecycle/ILifecycleComponentCallback.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleComponentCallbackMock : public ILifecycleComponentCallback
{
public:
    MOCK_METHOD1(transitionDone, void(ILifecycleComponent& component));
};

} // namespace lifecycle

#endif // GUARD_A925CAF1_D979_497B_89E9_9CA3A0B039A6
