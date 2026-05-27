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

#include "lifecycle/ILifecycleManager.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleManagerMock : public ILifecycleManager
{
public:
    MOCK_METHOD(uint8_t, getLevelCount, (), (const, override));
    MOCK_METHOD(void, transitionToLevel, (uint8_t level), (override));
    MOCK_METHOD(void, addLifecycleListener, (ILifecycleListener & listener), (override));
    MOCK_METHOD(void, removeLifecycleListener, (ILifecycleListener & listener), (override));
};

} // namespace lifecycle
