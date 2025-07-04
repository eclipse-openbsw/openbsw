// Copyright 2025 Accenture.

#pragma once

#include <async/Config.h>
#include <async/StaticContextHook.h>
#include <async/ThreadXAdapter.h>
#include <runtime/RuntimeMonitor.h>
#include <runtime/RuntimeStatistics.h>

#include <platform/estdint.h>

namespace async
{
struct AsyncBinding
{
    static size_t const WAIT_EVENTS_US = 100U;

    static size_t const TASK_COUNT = static_cast<size_t>(ASYNC_CONFIG_TASK_COUNT);

    using AdapterType = ThreadXAdapter<AsyncBinding>;

    using RuntimeMonitorType = ::runtime::declare::RuntimeMonitor<
        ::runtime::RuntimeStatistics,
        ::runtime::RuntimeStatistics,
        AdapterType::TASK_COUNT,
        ISR_GROUP_COUNT>;

    using ContextHookType = StaticContextHook<RuntimeMonitorType>;
};

using AsyncBindingType = AsyncBinding;
} // namespace async
