// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace async
{
class AsyncMock : public ::etl::singleton_base<AsyncMock>
{
public:
    AsyncMock() : ::etl::singleton_base<AsyncMock>(*this) {}

    MOCK_METHOD2(execute, void(ContextType contextType, RunnableType& runnableType));
    MOCK_METHOD5(
        schedule,
        void(
            ContextType contextType,
            RunnableType& runnableType,
            TimeoutType& timeoutType,
            uint32_t delay,
            TimeUnitType unit));
    MOCK_METHOD5(
        scheduleAtFixedRate,
        void(
            ContextType context,
            RunnableType& runnableType,
            TimeoutType& timeout,
            uint32_t period,
            TimeUnitType unit));
};

} // namespace async
