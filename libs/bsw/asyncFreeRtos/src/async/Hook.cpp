// Copyright 2024 Accenture.

#include "async/Hook.h"

#include "FreeRTOSConfig.h"
#include "async/AsyncBinding.h"
#include "freertos_tasks_c_additions.h"

namespace
{
using AdapterType     = ::async::AsyncBindingType::AdapterType;
using ContextHookType = ::async::AsyncBindingType::ContextHookType;
} // namespace

extern "C"
{
void asyncInitialized() { AdapterType::runningHook(); }

void const* asyncGetTaskConfig(size_t const taskIdx) { return AdapterType::getTaskConfig(taskIdx); }

void asyncEnterTask(size_t const taskIdx) { ContextHookType::enterTask(taskIdx); }

void asyncLeaveTask(size_t const taskIdx) { ContextHookType::leaveTask(taskIdx); }

void asyncEnterIsrGroup(size_t const isrGroupIdx)
{
    AdapterType::enterIsr();
    ContextHookType::enterIsrGroup(isrGroupIdx);
}

void asyncLeaveIsrGroup(size_t const isrGroupIdx)
{
    ContextHookType::leaveIsrGroup(isrGroupIdx);
    AdapterType::leaveIsr();
}

#if ASYNC_CONFIG_TICK_HOOK
uint32_t asyncTickHook()
{
    AdapterType::enterIsr();
    ::async::AsyncBindingType::TickHookType::handleTick();
    return AdapterType::leaveIsrNoYield() ? 1U : 0U;
}
#endif // ASYNC_CONFIG_TICK_HOOK

} // extern "C"
