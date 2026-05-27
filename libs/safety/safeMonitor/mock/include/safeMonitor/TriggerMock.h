/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "safeMonitor/common.h"

#include <gmock/gmock.h>

namespace safeMonitor
{
template<
    typename Handler,
    typename Event,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class TriggerMock
{
public:
    TriggerMock(Handler&, Event const&) {}

    MOCK_METHOD(void, trigger, ());
    MOCK_METHOD(void, trigger, (Context const&));
    MOCK_METHOD(Context&, getContext, (), (const));
};
} // namespace safeMonitor
