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
 * \ingroup async
 */
#pragma once

#include "async/ContextHookMock.h"
#include "async/FreeRtosAdapter.h"
#include "async/TickHookMock.h"

namespace async
{
struct AsyncBinding
{
    static size_t const TASK_COUNT                 = 3U;
    static TickType_t const WAIT_EVENTS_TICK_COUNT = 100U;

    using AdapterType     = FreeRtosAdapter<AsyncBinding>;
    using ContextHookType = ContextHookMock;
    using TickHookType    = TickHookMock;
};

using AsyncBindingType = AsyncBinding;

} // namespace async
