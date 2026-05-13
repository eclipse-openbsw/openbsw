// Copyright 2024 Accenture.

#pragma once

#include <async/Config.h>
#include <async/StaticContextHook.h>
#include <runtime/RuntimeMonitor.h>
#include <runtime/RuntimeStatistics.h>

#include <platform/estdint.h>

#if defined(SUPPORT_FREERTOS)
#include <async/FreeRtosAdapter.h>
#elif defined(SUPPORT_THREADX)
#include <async/ThreadXAdapter.h>
#endif

namespace async
{
struct AsyncBinding : public Config
{
    static size_t const WAIT_EVENTS_TICK_COUNT = 100U;

#if defined(SUPPORT_FREERTOS)
    using AdapterType = FreeRtosAdapter<AsyncBinding>;
#elif defined(SUPPORT_THREADX)
    using AdapterType = ThreadXAdapter<AsyncBinding>;
#endif

    using RuntimeMonitorType = ::runtime::declare::RuntimeMonitor<
        ::runtime::RuntimeStatistics,
        ::runtime::RuntimeStatistics,
        AdapterType::OS_TASK_COUNT,
        ISR_GROUP_COUNT>;

    using ContextHookType = StaticContextHook<RuntimeMonitorType>;
};

using AsyncBindingType = AsyncBinding;
} // namespace async
