// Copyright 2024 Accenture.

#include "example.h"

#include "async/Types.h"
#include "async/util/Call.h"

#include <etl/delegate.h>

#include <cstdio>

namespace asyncNewPlatform
{
// EXAMPLE_BEGIN AsyncImplExample
AsyncImplExample::AsyncImplExample()
: _eventMask(0), _eventPolicyA(*this), _eventPolicyB(*this), _runnableExecutor(*this)
{
    _eventPolicyA.setEventHandler(
        HandlerFunctionType::create<AsyncImplExample, &AsyncImplExample::handlerEventA>(*this));
    _eventPolicyB.setEventHandler(
        HandlerFunctionType::create<AsyncImplExample, &AsyncImplExample::handlerEventB>(*this));
    _runnableExecutor.init();
}

void AsyncImplExample::printBitmask(async::EventMaskType const eventMask)
{
    size_t const bits = 8;
    (void)fputs("0b", stdout);
    for (size_t i = bits - 1; i > 0; i--)
    {
        putchar(static_cast<int>('0' + ((eventMask >> (i - 1)) & 1)));
    }
}

void AsyncImplExample::execute(async::RunnableType& runnable)
{
    puts("AsyncImplExample::execute() called, Runnable is prepared for execution");
    _runnableExecutor.enqueue(runnable);
}

void AsyncImplExample::handlerEventA() { puts("AsyncImplExample::handlerEventA() is called."); }

void AsyncImplExample::handlerEventB() { puts("AsyncImplExample::handlerEventB() is called."); }

void AsyncImplExample::setEventA()
{
    puts("AsyncImplExample::setEventA() is called.");
    _eventPolicyA.setEvent();
}

void AsyncImplExample::setEventB()
{
    puts("AsyncImplExample::setEventB() is called.");
    _eventPolicyB.setEvent();
}

/**
 * This method is called everytime an event is set.
 * A logic to wakeup a task/context, which calls AsyncImplExample::dispatch() should be placed
 * here.
 */
void AsyncImplExample::setEvents(async::EventMaskType const eventMask)
{
    _eventMask |= eventMask;

    (void)fputs("AsyncImplExample::setEvents() is called with eventMask:", stdout);
    printBitmask(eventMask);
    (void)fputs(" new eventMask:", stdout);
    printBitmask(_eventMask);
    putchar('\n');
}

/**
 * This method is calling handlers for active events.
 * Should be placed in the main body of task/context.
 */
void AsyncImplExample::dispatch()
{
    (void)fputs("AsyncImplExample::dispatch() is called, eventMask:", stdout);
    printBitmask(_eventMask);
    putchar('\n');

    handleEvents(_eventMask);
    _eventMask = 0;

    (void)fputs("AsyncImplExample::dispatch() reset eventMask, eventMask:", stdout);
    printBitmask(_eventMask);
    putchar('\n');
}
} // namespace asyncNewPlatform

void exampleRunnableA() { puts("exampleRunnableA is called."); }

void exampleRunnableB() { puts("exampleRunnableB is called."); }

// NOLINTBEGIN(bugprone-exception-escape): This is just for testing purposes.
int main()
{
    auto eventManager = asyncNewPlatform::AsyncImplExample();
    async::Function runnableA(::etl::delegate<void()>::create<&exampleRunnableA>());
    async::Function runnableB(::etl::delegate<void()>::create<&exampleRunnableB>());
    eventManager.setEventA();
    eventManager.setEventB();
    eventManager.execute(runnableA);
    eventManager.execute(runnableB);
    eventManager.dispatch();
    return 0;
}

// NOLINTEND(bugprone-exception-escape)

// EXAMPLE_END AsyncImplExample
