// Copyright 2024 Accenture.

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
