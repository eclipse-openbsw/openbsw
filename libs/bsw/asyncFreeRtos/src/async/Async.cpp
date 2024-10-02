// Copyright 2024 Accenture.

#include "async/AsyncBinding.h"

namespace async
{
using AdapterType = AsyncBindingType::AdapterType;

void execute(ContextType const context, RunnableType& runnable)
{
    AdapterType::execute(context, runnable);
}

void schedule(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    AdapterType::schedule(context, runnable, timeout, delay, unit);
}

void scheduleAtFixedRate(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const period,
    TimeUnitType const unit)
{
    AdapterType::scheduleAtFixedRate(context, runnable, timeout, period, unit);
}

} // namespace async

extern "C"
{
void vApplicationIdleHook() { ::async::AdapterType::callIdleTaskFunction(); }

void vApplicationGetIdleTaskMemory(
    StaticTask_t** const ppxIdleTaskTCBBuffer,
    StackType_t** const ppxIdleTaskStackBuffer,
    uint32_t* const pulIdleTaskStackSize)
{
    ::async::AdapterType::getTaskMemory<::async::AdapterType::TASK_IDLE>(
        ppxIdleTaskTCBBuffer, ppxIdleTaskStackBuffer, pulIdleTaskStackSize);
}

void vApplicationGetTimerTaskMemory(
    StaticTask_t** const ppxTimerTaskTCBBuffer,
    StackType_t** const ppxTimerTaskStackBuffer,
    uint32_t* const pulTimerTaskStackSize)
{
    ::async::AdapterType::getTaskMemory<::async::AdapterType::TASK_TIMER>(
        ppxTimerTaskTCBBuffer, ppxTimerTaskStackBuffer, pulTimerTaskStackSize);
}

} // extern "C"
