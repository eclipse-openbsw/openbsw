// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleComponentCallback.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleComponentCallbackMock : public ILifecycleComponentCallback
{
public:
    MOCK_METHOD(void, transitionDone, (ILifecycleComponent & component), (override));
};

} // namespace lifecycle
