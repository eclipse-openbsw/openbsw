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
    typename RegisterType,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class RegisterMock
{
public:
    struct Entry
    {
        uintptr_t address;
        RegisterType valueMask;
        RegisterType expectedValue;
    };

    template<size_t N>
    RegisterMock(Handler&, Event const&, Entry const (&)[N])
    {}

    MOCK_METHOD(void, check, ());
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor
