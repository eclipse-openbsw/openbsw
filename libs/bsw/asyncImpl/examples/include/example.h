// Copyright 2024 Accenture.

#pragma once
#include "async/EventDispatcher.h"
#include "async/EventPolicy.h"
#include "async/RunnableExecutor.h"
#include "async/Types.h"

namespace asyncNewPlatform
{
// EXAMPLE_BEGIN AsyncImplExample

class AsyncImplExample : public async::EventDispatcher<3U, async::LockType>
{
public:
    AsyncImplExample();
    void setEvents(async::EventMaskType eventMask);
    void setEventA();
    void setEventB();
    void execute(async::RunnableType& runnable);
    void dispatch();

private:
    void handlerEventA();
    void handlerEventB();
    void printBitmask(async::EventMaskType const eventMask);
    friend class async::EventPolicy<AsyncImplExample, 0U>;
    friend class async::EventPolicy<AsyncImplExample, 1U>;
    friend class async::EventPolicy<AsyncImplExample, 2U>;

    async::EventMaskType _eventMask;
    async::EventPolicy<AsyncImplExample, 0U> _eventPolicyA;
    async::EventPolicy<AsyncImplExample, 1U> _eventPolicyB;
    async::RunnableExecutor<async::RunnableType, async::EventPolicy<AsyncImplExample, 2U>, LockType>
        _runnableExecutor;
};

// EXAMPLE_END AsyncImplExample

} // namespace asyncNewPlatform
