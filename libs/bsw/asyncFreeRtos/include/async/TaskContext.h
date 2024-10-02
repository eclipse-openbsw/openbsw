// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_60674D25_51E0_4A73_8CD1_9C6C6138C877
#define GUARD_60674D25_51E0_4A73_8CD1_9C6C6138C877

#include "async/EventDispatcher.h"
#include "async/EventPolicy.h"
#include "async/RunnableExecutor.h"
#include "async/Types.h"

#include <bsp/timer/SystemTimer.h>
#include <timer/Timer.h>

#include <estd/functional.h>
#include <estd/slice.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

namespace async
{
template<class Binding>
class TaskContext : public EventDispatcher<2U, LockType>
{
public:
    using TaskFunctionType = ::estd::function<void(TaskContext<Binding>&)>;
    using StackType        = ::estd::slice<StackType_t>;

    TaskContext();

    void initTask(ContextType context, TaskFunctionType taskFunction);
    void initTaskHandle(TaskHandle_t taskHandle);
    void createTask(
        ContextType context,
        StaticTask_t& task,
        char const* name,
        UBaseType_t priority,
        StackType const& stack,
        TaskFunctionType taskFunction);
    void createTimer(StaticTimer_t& timer, char const* name);

    char const* getName() const;
    TaskHandle_t getTaskHandle() const;
    uint32_t getUnusedStackSize() const;

    void execute(RunnableType& runnable);
    void schedule(RunnableType& runnable, TimeoutType& timeout, uint32_t delay, TimeUnitType unit);
    void scheduleAtFixedRate(
        RunnableType& runnable, TimeoutType& timeout, uint32_t period, TimeUnitType unit);
    void cancel(TimeoutType& timeout);

    void callTaskFunction();
    void dispatch();
    void stopDispatch();
    void dispatchWhileWork();

    void setTimeout(uint32_t timeInUs);

    static uint32_t getUnusedStackSize(TaskHandle_t taskHandle);
    static void defaultTaskFunction(TaskContext<Binding>& taskContext);

private:
    friend class EventPolicy<TaskContext<Binding>, 0U>;
    friend class EventPolicy<TaskContext<Binding>, 1U>;

    void setEvents(EventMaskType eventMask);
    EventMaskType waitEvents();
    EventMaskType peekEvents();

private:
    using TimerType = ::timer::Timer<LockType>;

    static EventMaskType const STOP_EVENT_MASK = static_cast<EventMaskType>(
        static_cast<EventMaskType>(1U) << static_cast<EventMaskType>(EVENT_COUNT));
    static EventMaskType const WAIT_EVENT_MASK = (STOP_EVENT_MASK << 1U) - 1U;

    void handleTimeout();

    static void staticTaskFunction(void* param);
    static void staticTimerFunction(TimerHandle_t handle);

    RunnableExecutor<RunnableType, EventPolicy<TaskContext<Binding>, 0U>, LockType>
        _runnableExecutor;
    TimerType _timer;
    EventPolicy<TaskContext<Binding>, 1U> _timerEventPolicy;
    TaskFunctionType _taskFunction;
    TaskHandle_t _taskHandle;
    TimerHandle_t _timerHandle;
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
        HandlerFunctionType::create<TaskContext, &TaskContext::handleTimeout>(*this));
    _runnableExecutor.init();
}

template<class Binding>
void TaskContext<Binding>::initTask(ContextType const context, TaskFunctionType const taskFunction)
{
    _context      = context;
    _taskFunction = taskFunction;
}

template<class Binding>
void TaskContext<Binding>::initTaskHandle(TaskHandle_t const taskHandle)
{
    _taskHandle = taskHandle;
}

template<class Binding>
void TaskContext<Binding>::createTask(
    ContextType const context,
    StaticTask_t& task,
    char const* const name,
    UBaseType_t const priority,
    StackType const& stack,
    TaskFunctionType const taskFunction)
{
    _context      = context;
    _taskFunction = taskFunction.has_value()
                        ? taskFunction
                        : TaskFunctionType::template create<&TaskContext::defaultTaskFunction>();
    _taskHandle   = xTaskCreateStatic(
        &staticTaskFunction,
        name,
        static_cast<uint32_t>(stack.size()),
        this,
        priority,
        stack.data(),
        &task);
}

template<class Binding>
void TaskContext<Binding>::createTimer(StaticTimer_t& timer, char const* const name)
{
    _timerHandle = xTimerCreateStatic(
        name, 1U, static_cast<UBaseType_t>(pdFALSE), this, &staticTimerFunction, &timer);
}

template<class Binding>
inline char const* TaskContext<Binding>::getName() const
{
    return (_timerHandle != nullptr) ? pcTimerGetName(_timerHandle) : nullptr;
}

template<class Binding>
inline TaskHandle_t TaskContext<Binding>::getTaskHandle() const
{
    return _taskHandle;
}

template<class Binding>
inline uint32_t TaskContext<Binding>::getUnusedStackSize() const
{
    return getUnusedStackSize(_taskHandle);
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
    BaseType_t* const higherPriorityTaskHasWoken = Binding::getHigherPriorityTaskWoken();
    if (higherPriorityTaskHasWoken != nullptr)
    {
        xTaskNotifyFromISR(_taskHandle, eventMask, eSetBits, higherPriorityTaskHasWoken);
    }
    else
    {
        xTaskNotify(_taskHandle, eventMask, eSetBits);
    }
}

template<class Binding>
inline EventMaskType TaskContext<Binding>::waitEvents()
{
    EventMaskType eventMask = 0U;
    xTaskNotifyWait(0U, WAIT_EVENT_MASK, &eventMask, Binding::WAIT_EVENTS_TICK_COUNT);
    return eventMask;
}

template<class Binding>
inline EventMaskType TaskContext<Binding>::peekEvents()
{
    EventMaskType eventMask = 0U;
    xTaskNotifyWait(0U, WAIT_EVENT_MASK, &eventMask, 0U);
    return eventMask;
}

template<class Binding>
void TaskContext<Binding>::setTimeout(uint32_t const timeInUs)
{
    if (timeInUs > 0U)
    {
        uint32_t const ticks
            = static_cast<uint32_t>((timeInUs + (Config::TICK_IN_US - 1U)) / Config::TICK_IN_US);
        (void)xTimerChangePeriod(_timerHandle, ticks, 0U);
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
void TaskContext<Binding>::dispatchWhileWork()
{
    while (true)
    {
        EventMaskType const eventMask = peekEvents();
        if (eventMask != 0U)
        {
            handleEvents(eventMask);
        }
        else
        {
            break;
        }
    }
}

template<class Binding>
uint32_t TaskContext<Binding>::getUnusedStackSize(TaskHandle_t const taskHandle)
{
    return static_cast<uint32_t>(uxTaskGetStackHighWaterMark(taskHandle))
           * static_cast<uint32_t>(sizeof(StackType_t));
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
    uint32_t nextDelta;
    if (_timer.getNextDelta(getSystemTimeUs32Bit(), nextDelta))
    {
        setTimeout(nextDelta);
    }
}

template<class Binding>
void TaskContext<Binding>::staticTaskFunction(void* const param)
{
    TaskContext& taskContext = *reinterpret_cast<TaskContext*>(param);
    taskContext.callTaskFunction();
}

template<class Binding>
void TaskContext<Binding>::staticTimerFunction(TimerHandle_t const handle)
{
    TaskContext& taskContext = *reinterpret_cast<TaskContext*>(pvTimerGetTimerID(handle));
    taskContext._timerEventPolicy.setEvent();
}

} // namespace async

#endif // GUARD_60674D25_51E0_4A73_8CD1_9C6C6138C877
