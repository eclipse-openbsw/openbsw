// Copyright 2025 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/StaticRunnable.h"
#include "async/TaskContext.h"
#include "interrupts/suspendResumeAllInterrupts.h"
#include "tx_api.h"

#include <etl/array.h>

namespace async
{
namespace internal
{
template<bool HasNestedInterrupts = (ASYNC_CONFIG_NESTED_INTERRUPTS != 0)>
class NestedInterruptLock : public LockType
{};

template<>
class NestedInterruptLock<false>
{};
} // namespace internal

/**
 * Adapter class bridging ThreadX functionalities with the application's binding.
 *
 * The `ThreadXAdapter` class serves as a centralized interface for managing tasks(threads), timers,
 * and scheduling functionalities within a ThreadX environment. It utilizes the specified
 * `Binding` type to adapt and configure task-related components, such as `TaskContext`,
 * `TaskConfig`, and backgroud task.
 *
 * \tparam Binding The binding type specifying application-specific configurations.
 */
template<class Binding>
class ThreadXAdapter
{
public:
    static size_t const TASK_COUNT           = Binding::TASK_COUNT;
    static size_t const OS_TASK_COUNT        = TASK_COUNT;
    static size_t const WAIT_EVENTS_US       = Binding::WAIT_EVENTS_US;
    static ContextType const TASK_BACKGROUND = (OS_TASK_COUNT - 1);

    using TaskContextType  = TaskContext<ThreadXAdapter>;
    using TaskFunctionType = typename TaskContextType::TaskFunctionType;
    using StackUsage       = typename TaskContextType::StackUsage;

    struct TaskConfigType
    {};

public:
    template<ContextType Context, size_t StackSize>
    struct Task
    {
    public:
        explicit Task(char const* name);
        ~Task() = default;

    private:
        TX_THREAD _task;
        TX_TIMER _timer;
        uint32_t _stack[StackSize / 4];
    };

    static char const* getTaskName(size_t taskIdx);

    static char const* getTaskName(ContextType const context);

    static ContextType getCurrentTaskContext();

    static void init();

    static void run();

    static bool getStackUsage(size_t taskIdx, StackUsage& stackUsage);

    static void execute(ContextType context, RunnableType& runnable);

    static void schedule(
        ContextType context,
        RunnableType& runnable,
        TimeoutType& timeout,
        uint32_t delay,
        TimeUnitType unit);

    static void scheduleAtFixedRate(
        ContextType context,
        RunnableType& runnable,
        TimeoutType& timeout,
        uint32_t delay,
        TimeUnitType unit);

    static void cancel(TimeoutType& timeout);

private:
    struct TaskInitializer : public StaticRunnable<TaskInitializer>
    {
        TaskInitializer(
            ContextType context,
            char const* name,
            TX_THREAD& task,
            TX_TIMER& timer,
            void* stack,
            size_t stackSize);

        void execute();

        void* _stack;
        size_t _stackSize;
        TX_THREAD& _task;
        TX_TIMER& _timer;
        char const* _name;
        ContextType _context;
    };

    static void initTask(TaskInitializer& initializer);

    static void staticTaskFunction(ULONG param);
    static void staticTimerFunction(ULONG param);

    static ::etl::array<TaskContextType, TASK_COUNT> _taskContexts;
};

/**
 * Inline implementations.
 */
template<class Binding>
::etl::array<typename ThreadXAdapter<Binding>::TaskContextType, ThreadXAdapter<Binding>::TASK_COUNT>
    ThreadXAdapter<Binding>::_taskContexts;

template<class Binding>
inline char const* ThreadXAdapter<Binding>::getTaskName(size_t const taskIdx)
{
    return _taskContexts[taskIdx].getName();
}

template<class Binding>
inline char const* ThreadXAdapter<Binding>::getTaskName(ContextType const context)
{
    return _taskContexts[context - 1].getName();
}

template<class Binding>
ContextType ThreadXAdapter<Binding>::getCurrentTaskContext()
{
    TX_THREAD* current = tx_thread_identify();
    if (TX_NULL != current)
    {
        UINT priority;
        tx_thread_info_get(current, 0, 0, 0, &priority, 0, 0, 0, 0);
        // priority equals context id
        return static_cast<ContextType>(priority);
        // TODO: find out if we are in interrupt context and
        // return CONTEXT_INVALID;
    }
    else
    {
        return CONTEXT_INVALID;
    }
}

template<class Binding>
void ThreadXAdapter<Binding>::initTask(TaskInitializer& initializer)
{
    ContextType const context    = initializer._context;
    TaskContextType& taskContext = _taskContexts[static_cast<size_t>(context) - 1];

    taskContext.createTimer(initializer._timer, initializer._name, &staticTimerFunction, context);

    taskContext.createTask(
        context,
        initializer._task,
        initializer._name,
        // assume context IDs start with 0, 0 is the highest priority
        static_cast<int>(context),
        initializer._stack,
        initializer._stackSize,
        &staticTaskFunction,
        context);
}

template<class Binding>
void ThreadXAdapter<Binding>::init()
{
    // ThreadX is initialized and api functions can be called!
    setThreadXInitialized();
    TaskInitializer::run();
}

template<class Binding>
void ThreadXAdapter<Binding>::run()
{
    for (size_t i = 0; i < _taskContexts.size(); i++)
    {
        _taskContexts[i].startTask();
    }
}

template<class Binding>
bool ThreadXAdapter<Binding>::getStackUsage(size_t const taskIdx, StackUsage& stackUsage)
{
    if (taskIdx < TASK_COUNT)
    {
        return _taskContexts[taskIdx].getStackUsage(stackUsage);
    }
    return false;
}

template<class Binding>
inline void ThreadXAdapter<Binding>::execute(ContextType const context, RunnableType& runnable)
{
    _taskContexts[static_cast<size_t>(context) - 1].execute(runnable);
}

template<class Binding>
inline void ThreadXAdapter<Binding>::schedule(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    _taskContexts[static_cast<size_t>(context) - 1].schedule(runnable, timeout, delay, unit);
}

template<class Binding>
inline void ThreadXAdapter<Binding>::scheduleAtFixedRate(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    _taskContexts[static_cast<size_t>(context) - 1].scheduleAtFixedRate(
        runnable, timeout, delay, unit);
}

template<class Binding>
inline void ThreadXAdapter<Binding>::cancel(TimeoutType& timeout)
{
    LockType const lock;
    ContextType const context = timeout._context;
    if (context != CONTEXT_INVALID)
    {
        timeout._context = CONTEXT_INVALID;
        _taskContexts[static_cast<size_t>(context) - 1].cancel(timeout);
    }
}

template<class Binding>
void ThreadXAdapter<Binding>::staticTaskFunction(ULONG param)
{
    TaskContextType& taskContext = _taskContexts[static_cast<size_t>(param) - 1];
    taskContext.callTaskFunction();
}

template<class Binding>
void ThreadXAdapter<Binding>::staticTimerFunction(ULONG param)
{
    TaskContextType& taskContext = _taskContexts[static_cast<size_t>(param) - 1];
    taskContext.handleTimerFunction();
}

template<class Binding>
ThreadXAdapter<Binding>::TaskInitializer::TaskInitializer(
    ContextType const context,
    char const* const name,
    TX_THREAD& task,
    TX_TIMER& timer,
    void* stack,
    size_t stackSize)
: _stack(stack), _stackSize(stackSize), _task(task), _timer(timer), _name(name), _context(context)
{}

template<class Binding>
void ThreadXAdapter<Binding>::TaskInitializer::execute()
{
    ThreadXAdapter<Binding>::initTask(*this);
}

template<class Binding>
template<ContextType Context, size_t StackSize>
ThreadXAdapter<Binding>::Task<Context, StackSize>::Task(char const* const name)
{
    estd_assert(sizeof(_stack) >= sizeof(TaskInitializer));
    new (_stack) TaskInitializer(Context, name, _task, _timer, _stack, StackSize);
}

} // namespace async
