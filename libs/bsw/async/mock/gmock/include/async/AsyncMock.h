// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_775E51F3_FB99_4636_9C9E_CA09F2349B71
#define GUARD_775E51F3_FB99_4636_9C9E_CA09F2349B71

#include "async/Async.h"

#include <estd/singleton.h>

#include <gmock/gmock.h>

namespace async
{
class AsyncMock : public ::estd::singleton<AsyncMock>
{
public:
    AsyncMock() : ::estd::singleton<AsyncMock>(*this) {}

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

#endif // GUARD_775E51F3_FB99_4636_9C9E_CA09F2349B71
