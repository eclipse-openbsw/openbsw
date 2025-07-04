// Copyright 2025 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/EventDispatcher.h"
#include "async/EventPolicy.h"
#include "async/RunnableExecutor.h"
#include "async/Types.h"
#include "tx_api.h"

#include <bsp/timer/SystemTimer.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <timer/Timer.h>
#include <util/estd/assert.h>

#define TX_TIMER_TICK_IN_US (1000000 / TX_TIMER_TICKS_PER_SECOND)
#define TX_TIMER_TICKS_FROM_US(us) \
    (static_cast<uint32_t>((us + (TX_TIMER_TICK_IN_US - 1U)) / TX_TIMER_TICK_IN_US))

namespace async
{
template<class Binding>
class TaskContext : public EventDispatcher<2U, LockType>
{
public:
    using TaskFunctionType = ::etl::delegate<void(TaskContext<Binding>&)>;

    struct StackUsage
    {
        StackUsage();

        uint32_t _stackSize;
        uint32_t _usedSize;
    };

    TaskContext();

    void initTask(ContextType context, TaskFunctionType taskFunction);
    void createTask(
        ContextType const context,
        TX_THREAD& task,
        char const* const name,
        int priority,
        void* stack,
        size_t stackSize,
        VOID (*entryFunction)(ULONG),
        ULONG entryInput);
    void
    createTimer(TX_TIMER& timer, char const* name, VOID (*entryFunction)(ULONG), ULONG entryInput);
    void startTask();

    char const* getName() const;
    bool getStackUsage(StackUsage& stackUsage) const;

    void execute(RunnableType& runnable);
    void schedule(RunnableType& runnable, TimeoutType& timeout, uint32_t delay, TimeUnitType unit);
    void scheduleAtFixedRate(
        RunnableType& runnable, TimeoutType& timeout, uint32_t period, TimeUnitType unit);
    void cancel(TimeoutType& timeout);

    void callTaskFunction();
    void handleTimerFunction();
    void dispatch();
    void stopDispatch();

    void setTimeout(uint32_t timeInUs);

    static void defaultTaskFunction(TaskContext<Binding>& taskContext);

private:
    friend class EventPolicy<TaskContext<Binding>, 0U>;
    friend class EventPolicy<TaskContext<Binding>, 1U>;

    void setEvents(EventMaskType eventMask);
    EventMaskType waitEvents();

private:
    using TimerType = ::timer::Timer<LockType>;

    static EventMaskType const STOP_EVENT_MASK = static_cast<EventMaskType>(
        static_cast<EventMaskType>(1U) << static_cast<EventMaskType>(EVENT_COUNT));
    static EventMaskType const WAIT_EVENT_MASK = (STOP_EVENT_MASK << 1U) - 1U;

    void handleTimerEventPolicy();

    RunnableExecutor<RunnableType, EventPolicy<TaskContext<Binding>, 0U>, LockType>
        _runnableExecutor;
    TimerType _timer;
    EventPolicy<TaskContext<Binding>, 1U> _timerEventPolicy;
    TaskFunctionType _taskFunction;
    TX_THREAD* _taskHandle;
    TX_TIMER* _timerHandle;
    TX_EVENT_FLAGS_GROUP _eventObject;
    ContextType _context;
};

/**
 * Inline implementations.
 */
template<class Binding>
inline TaskContext<Binding>::TaskContext()
: _runnableExecutor(*this)
, _timer()
, _timerEventPolicy(*this)
, _taskFunction()
, _taskHandle(nullptr)
, _timerHandle(nullptr)
, _context(CONTEXT_INVALID)
{
    _timerEventPolicy.setEventHandler(
        HandlerFunctionType::create<TaskContext, &TaskContext::handleTimerEventPolicy>(*this));
    _runnableExecutor.init();
}

template<class Binding>
void TaskContext<Binding>::initTask(ContextType const context, TaskFunctionType const taskFunction)
{
    _context      = context;
    _taskFunction = taskFunction;
}

template<class Binding>
void TaskContext<Binding>::createTask(
    ContextType const context,
    TX_THREAD& task,
    char const* const name,
    int priority,
    void* stack,
    size_t stackSize,
    VOID (*entryFunction)(ULONG),
    ULONG entryInput)
{
    _context      = context;
    _taskFunction = TaskFunctionType::template create<&TaskContext::defaultTaskFunction>();

    UINT status = tx_thread_create(
        &task,
        const_cast<CHAR*>(name),
        entryFunction,
        entryInput,
        stack,
        stackSize,
        priority,
        priority,
        TX_NO_TIME_SLICE,
        TX_DONT_START);
    estd_assert(status == TX_SUCCESS);

    status = tx_event_flags_create(
        &_eventObject,
        const_cast<CHAR*>(name) // use same name as for the task
    );
    estd_assert(status == TX_SUCCESS);
    _taskHandle = &task;
}

template<class Binding>
void TaskContext<Binding>::createTimer(
    TX_TIMER& timer, char const* const name, VOID (*entryFunction)(ULONG), ULONG entryInput)
{
    UINT status = tx_timer_create(
        &timer,
        const_cast<CHAR*>(name),
        entryFunction,
        entryInput,
        1, // minimum value
        0, // one-shot
        TX_NO_ACTIVATE);
    estd_assert(status == TX_SUCCESS);
    _timerHandle = &timer;
}

template<class Binding>
void TaskContext<Binding>::startTask()
{
    if (_taskHandle != nullptr)
    {
        tx_thread_resume(_taskHandle);
    }
}

template<class Binding>
inline char const* TaskContext<Binding>::getName() const
{
    if (_taskHandle != nullptr)
    {
        CHAR* name;
        if (TX_SUCCESS == tx_thread_info_get(_taskHandle, &name, 0, 0, 0, 0, 0, 0, 0))
        {
            return name;
        }
        else
        {
            return "<error>";
        }
    }
    else
    {
        return "<undefined>";
    }
}

template<class Binding>
inline bool TaskContext<Binding>::getStackUsage(StackUsage& stackUsage) const
{
    stackUsage._stackSize = static_cast<uint32_t>(_taskHandle->tx_thread_stack_size);
#ifdef TX_ENABLE_STACK_CHECKING
    size_t unused = 0;
    if (_taskHandle->tx_thread_stack_highest_ptr >= _taskHandle->tx_thread_stack_start)
    {
        unused = (uint8_t*)(_taskHandle->tx_thread_stack_highest_ptr)
                 - (uint8_t*)_taskHandle->tx_thread_stack_start;
    }
    stackUsage._usedSize = stackUsage._stackSize - unused;
#else
    // no information available
    stackUsage._usedSize = 0;
#endif
    return true;
}

template<class Binding>
inline void TaskContext<Binding>::execute(RunnableType& runnable)
{
    _runnableExecutor.enqueue(runnable);
}

template<class Binding>
inline void TaskContext<Binding>::schedule(
    RunnableType& runnable, TimeoutType& timeout, uint32_t const delay, TimeUnitType const unit)
{
    if (!_timer.isActive(timeout))
    {
        timeout._runnable = &runnable;
        timeout._context  = _context;
        if (_timer.set(timeout, delay * static_cast<uint32_t>(unit), getSystemTimeUs32Bit()))
        {
            _timerEventPolicy.setEvent();
        }
    }
}

template<class Binding>
inline void TaskContext<Binding>::scheduleAtFixedRate(
    RunnableType& runnable, TimeoutType& timeout, uint32_t const period, TimeUnitType const unit)
{
    if (!_timer.isActive(timeout))
    {
        timeout._runnable = &runnable;
        timeout._context  = _context;

        if (_timer.setCyclic(timeout, period * static_cast<uint32_t>(unit), getSystemTimeUs32Bit()))
        {
            _timerEventPolicy.setEvent();
        }
    }
}

template<class Binding>
inline void TaskContext<Binding>::cancel(TimeoutType& timeout)
{
    _timer.cancel(timeout);
}

template<class Binding>
inline void TaskContext<Binding>::setEvents(EventMaskType const eventMask)
{
    tx_event_flags_set(
        &_eventObject,
        eventMask,
        TX_OR // set eventMask bits in addition (OR)
    );
}

template<class Binding>
inline EventMaskType TaskContext<Binding>::waitEvents()
{
    ULONG events = 0;
    tx_event_flags_get(
        &_eventObject,
        WAIT_EVENT_MASK,
        TX_OR_CLEAR, // wait for any event, clear active events
        &events,
        // TX_TIMER_TICKS_FROM_US(Binding::WAIT_EVENTS_US)
        TX_WAIT_FOREVER);

    return events;
}

template<class Binding>
void TaskContext<Binding>::setTimeout(uint32_t const timeInUs)
{
    auto const tick
        = static_cast<uint32_t>((timeInUs + (TX_TIMER_TICK_IN_US - 1U)) / TX_TIMER_TICK_IN_US);

    if (tick > 0U)
    {
        // assume the timer is not active (first time or expired)
        // otherwise we would have to deactivate the timer first
        tx_timer_change(_timerHandle, tick, 0 /* one shot: period 0 */);
        tx_timer_activate(_timerHandle);
    }
    else
    {
        (void)_timerEventPolicy.setEvent();
    }
}

template<class Binding>
void TaskContext<Binding>::callTaskFunction()
{
    _taskFunction(*this);
}

template<class Binding>
void TaskContext<Binding>::dispatch()
{
    EventMaskType eventMask = 0U;
    while ((eventMask & STOP_EVENT_MASK) == 0U)
    {
        eventMask = waitEvents();
        handleEvents(eventMask);
    }
}

template<class Binding>
inline void TaskContext<Binding>::stopDispatch()
{
    _runnableExecutor.shutdown();
    setEvents(STOP_EVENT_MASK);
}

template<class Binding>
void TaskContext<Binding>::defaultTaskFunction(TaskContext<Binding>& taskContext)
{
    taskContext.dispatch();
}

template<class Binding>
void TaskContext<Binding>::handleTimerEventPolicy()
{
    while (_timer.processNextTimeout(getSystemTimeUs32Bit())) {}
    uint32_t nextDelta;
    if (_timer.getNextDelta(getSystemTimeUs32Bit(), nextDelta))
    {
        setTimeout(nextDelta);
    }
}

template<class Binding>
void TaskContext<Binding>::handleTimerFunction()
{
    _timerEventPolicy.setEvent();
}

template<class Binding>
TaskContext<Binding>::StackUsage::StackUsage() : _stackSize(0U), _usedSize(0U)
{}

} // namespace async
