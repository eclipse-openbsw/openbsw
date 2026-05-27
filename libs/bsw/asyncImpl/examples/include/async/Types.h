/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once
#include "async/IRunnable.h"

#include <platform/estdint.h>

namespace async
{
// EXAMPLE_BEGIN AsyncImplExample
struct ExampleLock
{};

using RunnableType  = IRunnable;
using ContextType   = uint8_t;
using EventMaskType = uint32_t;
using LockType      = ExampleLock;

struct TimeUnit
{
    enum Type
    {
        MICROSECONDS = 1,
        MILLISECONDS = 1000,
        SECONDS      = 1000000
    };
};

using TimeUnitType = TimeUnit::Type;

class TimeoutType
{
public:
    void cancel();
};

// EXAMPLE_END AsyncImplExample
} // namespace async
