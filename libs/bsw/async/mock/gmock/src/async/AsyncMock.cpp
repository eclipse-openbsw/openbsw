// Copyright 2024 Accenture.

#include "async/AsyncMock.h"

namespace async
{
void execute(ContextType const context, RunnableType& runnable)
{
    ::estd::singleton<AsyncMock>::instance().execute(context, runnable);
}

void schedule(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    ::async::TimeUnitType const unit)
{
    ::estd::singleton<AsyncMock>::instance().schedule(context, runnable, timeout, delay, unit);
}

void scheduleAtFixedRate(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const period,
    ::async::TimeUnitType const unit)
{
    ::estd::singleton<AsyncMock>::instance().scheduleAtFixedRate(
        context, runnable, timeout, period, unit);
}

} // namespace async
