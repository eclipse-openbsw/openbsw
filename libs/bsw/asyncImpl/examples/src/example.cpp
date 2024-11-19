// Copyright 2024 Accenture.

#include "example.h"

#include "async/Types.h"
#include "async/util/Call.h"

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
    printf("0b");
    for (size_t i = bits - 1; i > 0; i--)
    {
        printf("%d", (eventMask >> (i - 1)) & 1);
    }
}

void AsyncImplExample::execute(async::RunnableType& runnable)
{
    printf("AsyncImplExample::execute() called, Runnable is prepared for execution\n");
    _runnableExecutor.enqueue(runnable);
}

void AsyncImplExample::handlerEventA() { printf("AsyncImplExample::handlerEventA() is called.\n"); }

void AsyncImplExample::handlerEventB() { printf("AsyncImplExample::handlerEventB() is called.\n"); }

void AsyncImplExample::setEventA()
{
    printf("AsyncImplExample::setEventA() is called.\n");
    _eventPolicyA.setEvent();
}

void AsyncImplExample::setEventB()
{
    printf("AsyncImplExample::setEventB() is called.\n");
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

    printf("AsyncImplExample::setEvents() is called with eventMask:");
    printBitmask(eventMask);
    printf(" new eventMask:");
    printBitmask(_eventMask);
    printf("\n");
}

/**
 * This method is calling handlers for active events.
 * Should be placed in the main body of task/context.
 */
void AsyncImplExample::dispatch()
{
    printf("AsyncImplExample::dispatch() is called, eventMask:");
    printBitmask(_eventMask);
    printf("\n");

    handleEvents(_eventMask);
    _eventMask = 0;

    printf("AsyncImplExample::dispatch() reset eventMask, eventMask:");
    printBitmask(_eventMask);
    printf("\n");
}
} // namespace asyncNewPlatform

void exampleRunnableA() { printf("exampleRunnableA is called.\n"); }

void exampleRunnableB() { printf("exampleRunnableB is called.\n"); }

int main()
{
    auto eventManager = asyncNewPlatform::AsyncImplExample();
    async::Function runnableA(::estd::function<void()>::create<&exampleRunnableA>());
    async::Function runnableB(::estd::function<void()>::create<&exampleRunnableB>());
    eventManager.setEventA();
    eventManager.setEventB();
    eventManager.execute(runnableA);
    eventManager.execute(runnableB);
    eventManager.dispatch();
    return 0;
}

// EXAMPLE_END AsyncImplExample
