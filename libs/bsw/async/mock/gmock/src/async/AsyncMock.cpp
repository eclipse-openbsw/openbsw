// Copyright 2024 Accenture.

#include "async/AsyncMock.h"

#include <etl/singleton_base.h>

namespace async
{
void execute(ContextType const context, RunnableType& runnable)
{
    ::etl::singleton_base<AsyncMock>::instance().execute(context, runnable);
}

void schedule(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    ::async::TimeUnitType const unit)
{
    ::etl::singleton_base<AsyncMock>::instance().schedule(context, runnable, timeout, delay, unit);
}

void scheduleAtFixedRate(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const period,
    ::async::TimeUnitType const unit)
{
    ::etl::singleton_base<AsyncMock>::instance().scheduleAtFixedRate(
        context, runnable, timeout, period, unit);
}

} // namespace async
