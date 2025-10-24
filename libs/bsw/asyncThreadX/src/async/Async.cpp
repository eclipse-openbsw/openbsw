// Copyright 2025 Accenture.

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
// This function will be called by Threadx::tx_kernel_enter()
void tx_application_define(void* first_unused_memory)
{
    (void)first_unused_memory;

    ::async::AdapterType::callInitFromThreadXKernel();
}

} // extern "C"
