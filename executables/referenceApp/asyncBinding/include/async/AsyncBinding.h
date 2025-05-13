// Copyright 2024 Accenture.

#pragma once

#include <async/Config.h>
#include <async/FreeRtosAdapter.h>
#include <async/StaticContextHook.h>
#ifdef BENCHMARK
#include <runtime/RuntimeMonitorSimple.h>
#else
#include <runtime/RuntimeMonitor.h>
#endif
#include <runtime/RuntimeStatistics.h>

#include <platform/estdint.h>

namespace async
{
struct AsyncBinding : public Config
{
    static size_t const WAIT_EVENTS_TICK_COUNT = 100U;

    using AdapterType = FreeRtosAdapter<AsyncBinding>;

    using RuntimeMonitorType = ::runtime::declare::RuntimeMonitor<
        ::runtime::RuntimeStatistics,
        ::runtime::RuntimeStatistics,
        AdapterType::FREERTOS_TASK_COUNT,
        ISR_GROUP_COUNT>;

    using ContextHookType = StaticContextHook<RuntimeMonitorType>;
};

using AsyncBindingType = AsyncBinding;
} // namespace async
