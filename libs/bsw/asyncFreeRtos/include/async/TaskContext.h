// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/EventDispatcher.h"
#include "async/EventPolicy.h"
#include "async/RunnableExecutor.h"
#include "async/Types.h"

#include <bsp/timer/SystemTimer.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <timer/Timer.h>

#include <FreeRTOS.h>
#include <task.h>

namespace async
{
/**
 * Provides an interface between application-specific Tasks and Timers
 * and the FreeRTOS framework, managing FreeRTOS* task and timer callbacks.
 *
 * The TaskContext class facilitates seamless integration between the application's
 * task and timer logic and FreeRTOS by invoking necessary FreeRTOS functions
 * for task creation, scheduling and processing callbacks.
 *
 * \tparam Binding The specific binding type associated with the TaskContext.
 */
template<class Binding>
class TaskContext : public EventDispatcher<2U, LockType>
{
public:
    using TaskFunctionType = ::etl::delegate<void(TaskContext<Binding>&)>;
    using StackType        = ::etl::span<StackType_t>;

    TaskContext();

    /**
     * Initializes a task with a specified context and task function.
     * \param context The context associated with this task.
     * \param name The name of this task.
     * \param taskFunction The function to execute in this task.
     */
    void initTask(ContextType context, char const* const name, TaskFunctionType taskFunction);

    /**
     * Sets the FreeRTOS task handle for this context.
     * \param taskHandle The handle to associate with the FreeRTOS task.
     */
    void initTaskHandle(TaskHandle_t taskHandle);

    /**
     * Creates a FreeRTOS task with specific configuration.
     * \param context The task's context.
     * \param task The static task storage structure.
     * \param name The name of the task.
     * \param priority The priority of the task.
     * \param stack The stack allocated for the task.
     * \param taskFunction The function that the task will execute.
     */
    void createTask(
        ContextType context,
        StaticTask_t& task,
        char const* name,
        UBaseType_t priority,
        StackType const& stack,
        TaskFunctionType taskFunction);

    char const* getName() const;

    TaskHandle_t getTaskHandle() const;

    uint32_t getUnusedStackSize() const;

    /**
     * Executes asynchronously the specified runnable within this task context.
     * \param runnable The runnable to execute.
     */
    void execute(RunnableType& runnable);

    /**
     * Schedules a runnable to execute after a delay.
     * \param runnable The runnable to schedule.
     * \param timeout The timeout associated with the runnable.
     * \param delay The delay before execution.
     * \param unit The time unit for the delay.
     */
    void schedule(RunnableType& runnable, TimeoutType& timeout, uint32_t delay, TimeUnitType unit);

    /**
     * Schedules a runnable to execute at a fixed rate.
     * \param runnable The runnable to schedule.
     * \param timeout The timeout associated with the runnable.
     * \param period The period between executions.
     * \param unit The time unit for the period.
     */
    void scheduleAtFixedRate(
        RunnableType& runnable, TimeoutType& timeout, uint32_t period, TimeUnitType unit);

    /**
     * Cancels a scheduled runnable.
     * \param timeout The timeout associated with the runnable to cancel.
     */
    void cancel(TimeoutType& timeout);

    /// Calls the task's assigned function.
    void callTaskFunction();

    /// Dispatches events for processing within this context.
    void dispatch();

    /// Stops event dispatching.
    void stopDispatch();

    /// Dispatches events while runnable is executing.
    void dispatchWhileWork();

    /**
     * Retrieves the amount of unused stack space for a specified task.
     * \param taskHandle The handle of the task.
     * \return The amount of unused stack space.
     */
    static uint32_t getUnusedStackSize(TaskHandle_t taskHandle);

    /**
     * Default function to be executed by a task within this context.
     * \param taskContext The context in which the task executes.
     */
    static void defaultTaskFunction(TaskContext<Binding>& taskContext);

    /**
     * Default function to be executed by the idle task.
     * \param taskContext The context in which the task executes.
     */
    static void defaultIdleFunction(TaskContext<Binding>& taskContext);

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

    static void staticTaskFunction(void* param);

    RunnableExecutor<RunnableType, ExecuteEventPolicyType, LockType> _runnableExecutor;
    TimerType _timer;
    TimerEventPolicyType _timerEventPolicy;
    TaskFunctionType _taskFunction;
    TaskHandle_t _taskHandle;
    char const* _name;
    ContextType _context;
};

/**
 * Inline implementations.
 */
template<class Binding>
inline TaskContext<Binding>::TaskContext()
: _runnableExecutor(*this)
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
    ContextType const context, char const* const name, TaskFunctionType const taskFunction)
{
    _context      = context;
    _name         = name;
    _taskFunction = taskFunction.is_valid()
                        ? taskFunction
                        : TaskFunctionType::template create<&TaskContext::defaultIdleFunction>();
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
    _name         = name;
    _taskFunction = taskFunction.is_valid()
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
inline char const* TaskContext<Binding>::getName() const
{
    return _name;
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
    uint32_t ticks          = Binding::WAIT_EVENTS_TICK_COUNT;
    uint32_t nextDelta;
    bool const hasDelta = _timer.getNextDelta(getSystemTimeUs32Bit(), nextDelta);
    if (hasDelta)
    {
        ticks = static_cast<uint32_t>((nextDelta + (Config::TICK_IN_US - 1U)) / Config::TICK_IN_US);
    }
    if (xTaskNotifyWait(0U, WAIT_EVENT_MASK, &eventMask, ticks) != 0)
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
inline EventMaskType TaskContext<Binding>::peekEvents()
{
    EventMaskType eventMask = 0U;
    (void)xTaskNotifyWait(0U, WAIT_EVENT_MASK, &eventMask, 0U);
    return eventMask;
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
    setEvents(STOP_EVENT_MASK);
}

template<class Binding>
void TaskContext<Binding>::dispatchWhileWork()
{
    while (true)
    {
        handleTimeout();
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
void TaskContext<Binding>::defaultIdleFunction(TaskContext<Binding>& taskContext)
{
    taskContext.dispatchWhileWork();
}

template<class Binding>
void TaskContext<Binding>::handleTimeout()
{
    while (_timer.processNextTimeout(getSystemTimeUs32Bit())) {}
}

template<class Binding>
void TaskContext<Binding>::staticTaskFunction(void* const param)
{
    TaskContext& taskContext = *reinterpret_cast<TaskContext*>(param);
    taskContext.callTaskFunction();
}

} // namespace async
