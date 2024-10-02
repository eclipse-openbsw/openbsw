// Copyright 2024 Accenture.

#ifndef GUARD_B096509F_26BB_48E2_9310_07C84197883C
#define GUARD_B096509F_26BB_48E2_9310_07C84197883C

#include <async/Config.h>
#include <async/FreeRtosAdapter.h>
#include <async/StaticContextHook.h>
#include <runtime/RuntimeMonitor.h>
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

#endif /* GUARD_B096509F_26BB_48E2_9310_07C84197883C */
