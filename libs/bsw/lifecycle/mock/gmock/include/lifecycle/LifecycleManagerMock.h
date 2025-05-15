// Copyright 2024 Accenture.

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
