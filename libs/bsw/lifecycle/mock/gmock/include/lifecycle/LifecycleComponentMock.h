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
