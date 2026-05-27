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
