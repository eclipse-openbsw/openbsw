// Copyright 2025 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "ThreadXConfig.h"
#include "async/EventDispatcher.h"
#include "async/EventPolicy.h"
#include "async/RunnableExecutor.h"
#include "async/Types.h"
#include "tx_api.h"

#include <bsp/timer/SystemTimer.h>
#include <etl/delegate.h>
#include <etl/error_handler.h>
#include <etl/span.h>
#include <timer/Timer.h>

namespace async
{
template<class Binding>
class TaskContext : public EventDispatcher<2U, LockType>
{
public:
    using TaskFunctionType       = ::etl::delegate<void(TaskContext<Binding>&)>;
    using StaticTaskFunctionType = void (*)(ULONG);
    using StackType              = ::etl::span<ULONG>;

    TaskContext();

    void initTask(ContextType context, char const* const name, TX_THREAD& taskHandle);

    void createTask(
        ContextType const context,
        TX_THREAD& task,
        char const* const name,
        uint16_t priority,
        StackType const& stack,
        size_t const stackSize,
        TaskFunctionType taskFunction,
        StaticTaskFunctionType staticTaskFunction);

    void startTask();

    char const* getName() const;

    TX_THREAD& getTaskHandle() const;

    void execute(RunnableType& runnable);
    void schedule(RunnableType& runnable, TimeoutType& timeout, uint32_t delay, TimeUnitType unit);
    void scheduleAtFixedRate(
        RunnableType& runnable, TimeoutType& timeout, uint32_t period, TimeUnitType unit);
    void cancel(TimeoutType& timeout);

    void callTaskFunction();
    void dispatch();
    void stopDispatch();
    void dispatchWhileWork();

    static void defaultTaskFunction(TaskContext<Binding>& taskContext);

private:
    friend class EventPolicy<TaskContext<Binding>, 0U>;
    friend class EventPolicy<TaskContext<Binding>, 1U>;

    using ExecuteEventPolicyType = EventPolicy<TaskContext<Binding>, 0U>;
    using TimerEventPolicyType   = EventPolicy<TaskContext<Binding>, 1U>;
    using TimerType              = ::timer::Timer<LockType>;

    static EventMaskType const STOP_EVENT_MASK = static_cast<EventMaskType>(
        static_cast<EventMaskType>(1U) << static_cast<EventMaskType>(EVENT_COUNT));
    static EventMaskType const WAIT_EVENT_MASK = (STOP_EVENT_MASK << 1U) - 1U;

    void setEvents(EventMaskType eventMask);
    EventMaskType waitEvents();
    EventMaskType peekEvents();

    void handleTimeout();

    RunnableExecutor<RunnableType, ExecuteEventPolicyType, LockType> _runnableExecutor;
    TimerType _timer;
    TimerEventPolicyType _timerEventPolicy;
    TaskFunctionType _taskFunction;
    TX_THREAD* _taskHandle;
    TX_EVENT_FLAGS_GROUP _eventObject;
    char const* _name;
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
, _name(nullptr)
, _context(CONTEXT_INVALID)
{
    _timerEventPolicy.setEventHandler(
        HandlerFunctionType::create<TaskContext, &TaskContext::handleTimeout>(*this));
    _runnableExecutor.init();
}

template<class Binding>
void TaskContext<Binding>::initTask(
    ContextType const context, char const* const name, TX_THREAD& taskHandle)
{
    _context    = context;
    _name       = name;
    _taskHandle = &taskHandle;
}

template<class Binding>
void TaskContext<Binding>::createTask(
    ContextType const context,
    TX_THREAD& task,
    char const* const name,
    uint16_t priority,
    StackType const& stack,
    size_t stackSize,
    TaskFunctionType const taskFunction,
    StaticTaskFunctionType const staticTaskFunction)
{
    _context      = context;
    _name         = name;
    _taskFunction = taskFunction.is_valid()
                        ? taskFunction
                        : TaskFunctionType::template create<&TaskContext::defaultTaskFunction>();

    UINT status = tx_thread_create(
        &task,
        const_cast<CHAR*>(name),
        staticTaskFunction,
        _context,
        stack.data(),
        stackSize,
        priority,
        priority,
        TX_NO_TIME_SLICE,
        TX_DONT_START);
    ETL_ASSERT(status == TX_SUCCESS, ETL_ERROR_GENERIC("tx_thread_create must return success"));

    status = tx_event_flags_create(
        &_eventObject,
        const_cast<CHAR*>(name) // use same name as for the task
    );
    ETL_ASSERT(
        status == TX_SUCCESS, ETL_ERROR_GENERIC("tx_event_flags_create must return success"));
    _taskHandle = &task;
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
    if (_name != nullptr)
    {
        return _name;
    }
    else
    {
        return "<undefined>";
    }
}

template<class Binding>
inline TX_THREAD& TaskContext<Binding>::getTaskHandle() const
{
    return *_taskHandle;
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
    EventMaskType eventMask = 0U;
    uint32_t ticks          = Binding::WAIT_EVENTS_TICK_COUNT;
    uint32_t nextDelta;
    bool const hasDelta = _timer.getNextDelta(getSystemTimeUs32Bit(), nextDelta);
    if (hasDelta)
    {
        ticks = static_cast<uint32_t>((nextDelta + (Config::TICK_IN_US - 1U)) / Config::TICK_IN_US);
    }
    auto const result = tx_event_flags_get(
        &_eventObject,
        WAIT_EVENT_MASK,
        TX_OR_CLEAR, // wait for any event, clear active events
        &eventMask,
        ticks);

    if (result == TX_SUCCESS)
    {
        return eventMask;
    }
    else if (hasDelta)
    {
        return TimerEventPolicyType::EVENT_MASK;
    }
    else
    {
        return 0U;
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
void TaskContext<Binding>::handleTimeout()
{
    while (_timer.processNextTimeout(getSystemTimeUs32Bit())) {}
}

} // namespace async
