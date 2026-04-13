// Copyright 2024 Accenture.

#include "example.h"

#include "async/Types.h"
#include "async/util/Call.h"

#include <etl/delegate.h>
#include <etl/print.h>

#include <cstdio>

extern "C" void etl_putchar(int c) { std::putchar(c); }

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
    ::etl::print("0b");
    for (size_t i = bits; i > 0U; --i)
    {
        ::etl::print("{}", static_cast<unsigned int>((eventMask >> (i - 1U)) & 1U));
    }
}

void AsyncImplExample::execute(async::RunnableType& runnable)
{
    ::etl::println("AsyncImplExample::execute() called, Runnable is prepared for execution");
    _runnableExecutor.enqueue(runnable);
}

void AsyncImplExample::handlerEventA()
{
    ::etl::println("AsyncImplExample::handlerEventA() is called.");
}

void AsyncImplExample::handlerEventB()
{
    ::etl::println("AsyncImplExample::handlerEventB() is called.");
}

void AsyncImplExample::setEventA()
{
    ::etl::println("AsyncImplExample::setEventA() is called.");
    _eventPolicyA.setEvent();
}

void AsyncImplExample::setEventB()
{
    ::etl::println("AsyncImplExample::setEventB() is called.");
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

    ::etl::print("AsyncImplExample::setEvents() is called with eventMask:");
    printBitmask(eventMask);
    ::etl::print(" new eventMask:");
    printBitmask(_eventMask);
    ::etl::println();
}

/**
 * This method is calling handlers for active events.
 * Should be placed in the main body of task/context.
 */
void AsyncImplExample::dispatch()
{
    ::etl::print("AsyncImplExample::dispatch() is called, eventMask:");
    printBitmask(_eventMask);
    ::etl::println();

    handleEvents(_eventMask);
    _eventMask = 0;

    ::etl::print("AsyncImplExample::dispatch() reset eventMask, eventMask:");
    printBitmask(_eventMask);
    ::etl::println();
}
} // namespace asyncNewPlatform

void exampleRunnableA() { ::etl::println("exampleRunnableA is called."); }

void exampleRunnableB() { ::etl::println("exampleRunnableB is called."); }

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
