// Copyright 2025 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "ThreadXConfig.h"
#include "async/TaskContext.h"
#include "async/TaskInitializer.h"
#include "interrupts/suspendResumeAllInterrupts.h"
#include "tx_api.h"

#include <etl/array.h>

extern "C"
{
extern TX_THREAD _tx_timer_thread;
}

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

template<size_t N, typename TaskConfig = ASYNC_TASK_CONFIG_TYPE>
class TaskConfigHolder
{
public:
    using TaskConfigType = TaskConfig;

    static void setTaskConfig(size_t taskIdx, TaskConfigType const& taskConfig);

    static TaskConfigType const* getTaskConfig(size_t taskIdx);

private:
    static ::etl::array<TaskConfigType, N> _taskConfigs;
};

template<size_t N>
class TaskConfigHolder<N, void>
{
public:
    struct TaskConfigType
    {};

    static void setTaskConfig(size_t taskIdx, TaskConfigType const& taskConfig);

    static TaskConfigType const* getTaskConfig(size_t taskIdx);
};
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
    static size_t const TASK_COUNT            = Binding::TASK_COUNT;
    static size_t const OS_TASK_COUNT         = TASK_COUNT + 1U;
    static ULONG const WAIT_EVENTS_TICK_COUNT = static_cast<ULONG>(Binding::WAIT_EVENTS_TICK_COUNT);
    static ContextType const TASK_IDLE        = 0U;
    static ContextType const TASK_TIMER       = static_cast<ContextType>(TASK_COUNT);

    using AdapterType = ThreadXAdapter<Binding>;

    using TaskContextType        = TaskContext<AdapterType>;
    using TaskFunctionType       = typename TaskContextType::TaskFunctionType;
    using StaticTaskFunctionType = typename TaskContextType::StaticTaskFunctionType;

    using TaskConfigsType = internal::TaskConfigHolder<OS_TASK_COUNT>;
    using TaskConfigType  = typename TaskConfigsType::TaskConfigType;

    using StartAppFunctionType = ::etl::delegate<void()>;

    template<size_t StackSize>
    using Stack           = internal::Stack<StackSize>;
    using TaskInitializer = internal::TaskInitializer<AdapterType>;

    template<size_t StackSize = 0U>
    using IdleTask = internal::IdleTask<AdapterType, StackSize>;
    template<size_t StackSize = 0U>
    using TimerTask = internal::TimerTask<AdapterType, StackSize>;
    template<ContextType Context, size_t StackSize = 0U>
    using Task = internal::Task<AdapterType, Context, StackSize>;
    template<ContextType Context>
    using TaskStack = internal::Task<AdapterType, Context>;

    /// Struct representing the stack usage for a specific task.
    struct StackUsage
    {
        StackUsage();

        uint32_t _stackSize;
        uint32_t _usedSize;
    };

public:
    static char const* getTaskName(size_t taskIdx);

    static TaskConfigType const* getTaskConfig(size_t taskIdx);

    static ContextType getCurrentTaskContext();

    static void run(StartAppFunctionType startApp);

    static bool getStackUsage(size_t taskIdx, StackUsage& stackUsage);

    static void callIdleTaskFunction();

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

    static void callInitFromThreadXKernel();

private:
    friend struct internal::TaskInitializer<AdapterType>;

    static void initTask(TaskInitializer& initializer);

    static void staticTaskFunction(ULONG param);

    static StartAppFunctionType _startApp;
    static TaskInitializer* _idleTaskInitializer;
    static TaskInitializer* _timerTaskInitializer;
    static ::etl::array<TaskContextType, OS_TASK_COUNT> _taskContexts;
    static ::etl::array<uint32_t, OS_TASK_COUNT> _stackSizes;
    static char const* _timerTaskName;
};

/**
 * Inline implementations.
 */
template<class Binding>
typename ThreadXAdapter<Binding>::StartAppFunctionType ThreadXAdapter<Binding>::_startApp;
template<class Binding>
typename ThreadXAdapter<Binding>::TaskInitializer* ThreadXAdapter<Binding>::_idleTaskInitializer
    = nullptr;
template<class Binding>
typename ThreadXAdapter<Binding>::TaskInitializer* ThreadXAdapter<Binding>::_timerTaskInitializer
    = nullptr;
template<class Binding>
::etl::
    array<typename ThreadXAdapter<Binding>::TaskContextType, ThreadXAdapter<Binding>::OS_TASK_COUNT>
        ThreadXAdapter<Binding>::_taskContexts;
template<class Binding>
::etl::array<uint32_t, ThreadXAdapter<Binding>::OS_TASK_COUNT> ThreadXAdapter<Binding>::_stackSizes;
template<class Binding>
char const* ThreadXAdapter<Binding>::_timerTaskName;

template<class Binding>
inline char const* ThreadXAdapter<Binding>::getTaskName(size_t taskIdx)
{
    return _taskContexts[taskIdx].getName();
}

template<class Binding>
inline typename ThreadXAdapter<Binding>::TaskConfigType const*
ThreadXAdapter<Binding>::getTaskConfig(size_t const taskIdx)
{
    return TaskConfigsType::getTaskConfig(taskIdx);
}

template<class Binding>
ContextType ThreadXAdapter<Binding>::getCurrentTaskContext()
{
    TX_THREAD* current = tx_thread_identify();
    if (TX_NULL != current)
    {
        return static_cast<ContextType>(TASK_TIMER - current->tx_thread_priority);
    }
    else
    {
        return CONTEXT_INVALID;
    }
}

template<class Binding>
void ThreadXAdapter<Binding>::initTask(TaskInitializer& initializer)
{
    ContextType const context = initializer._context;
    _stackSizes[static_cast<size_t>(context)]
        = static_cast<uint32_t>(static_cast<size_t>(initializer._stack.size()) * sizeof(ULONG));
    TaskFunctionType const taskFunction = initializer._taskFunction;
    char const* const taskName          = initializer._name;
    TaskConfigsType::setTaskConfig(context, initializer._config);
    TaskContextType& taskContext = _taskContexts[static_cast<size_t>(context)];

    if (context == TASK_TIMER)
    {
        taskContext.initTask(context, taskName, _tx_timer_thread);
    }
    else
    {
        taskContext.createTask(
            context,
            initializer._task,
            taskName,
            static_cast<uint16_t>(TASK_TIMER - context),
            initializer._stack,
            _stackSizes[static_cast<size_t>(context)],
            taskFunction,
            staticTaskFunction);
    }
}

template<class Binding>
void ThreadXAdapter<Binding>::callInitFromThreadXKernel()
{
    // ThreadX is initialized and api functions can be called!
    setThreadXInitialized();
    TaskInitializer::run();
    _startApp();

    for (size_t i = 0U; i < TASK_TIMER; i++)
    {
        _taskContexts[i].startTask();
    }
}

template<class Binding>
void ThreadXAdapter<Binding>::run(StartAppFunctionType startApp)
{
    static_assert(TASK_COUNT <= TX_MAX_PRIORITIES, "TASK_COUNT exceeds TX_MAX_PRIORITIES");
    estd_assert(startApp.is_valid());
    _startApp = startApp;
    tx_kernel_enter();
}

template<class Binding>
bool ThreadXAdapter<Binding>::getStackUsage(size_t const taskIdx, StackUsage& stackUsage)
{
    if (taskIdx < OS_TASK_COUNT)
    {
        TX_THREAD& taskHandle = _taskContexts[taskIdx].getTaskHandle();
        stackUsage._stackSize = static_cast<uint32_t>(taskHandle.tx_thread_stack_size);

#ifdef TX_ENABLE_STACK_CHECKING
        // Downward stack growth
        if (taskHandle.tx_thread_stack_end >= taskHandle.tx_thread_stack_start)
        {
            stackUsage._usedSize = static_cast<ULONG>(
                reinterpret_cast<UCHAR*>(taskHandle.tx_thread_stack_end)
                - reinterpret_cast<UCHAR*>(taskHandle.tx_thread_stack_highest_ptr));
        }
        else
        {
            stackUsage._usedSize = static_cast<ULONG>(
                reinterpret_cast<UCHAR*>(taskHandle.tx_thread_stack_start)
                - reinterpret_cast<UCHAR*>(taskHandle.tx_thread_stack_highest_ptr));
        }
#else
        // no information available
        stackUsage._usedSize = 0;
#endif
        return true;
    }

    return false;
}

template<class Binding>
inline void ThreadXAdapter<Binding>::execute(ContextType const context, RunnableType& runnable)
{
    _taskContexts[static_cast<size_t>(context)].execute(runnable);
}

template<class Binding>
inline void ThreadXAdapter<Binding>::schedule(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    _taskContexts[static_cast<size_t>(context)].schedule(runnable, timeout, delay, unit);
}

template<class Binding>
inline void ThreadXAdapter<Binding>::scheduleAtFixedRate(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    _taskContexts[static_cast<size_t>(context)].scheduleAtFixedRate(runnable, timeout, delay, unit);
}

template<class Binding>
inline void ThreadXAdapter<Binding>::cancel(TimeoutType& timeout)
{
    LockType const lock;
    ContextType const context = timeout._context;
    if (context != CONTEXT_INVALID)
    {
        timeout._context = CONTEXT_INVALID;
        _taskContexts[static_cast<size_t>(context)].cancel(timeout);
    }
}

template<class Binding>
void ThreadXAdapter<Binding>::staticTaskFunction(ULONG param)
{
    TaskContextType& taskContext = _taskContexts[static_cast<size_t>(param)];
    taskContext.callTaskFunction();
}

template<class Binding>
ThreadXAdapter<Binding>::StackUsage::StackUsage() : _stackSize(0U), _usedSize(0U)
{}

namespace internal
{
template<size_t N, typename TaskConfig>
::etl::array<TaskConfig, N> TaskConfigHolder<N, TaskConfig>::_taskConfigs;

template<size_t N, typename TaskConfig>
void TaskConfigHolder<N, TaskConfig>::setTaskConfig(
    size_t const taskIdx, TaskConfigType const& taskConfig)
{
    _taskConfigs[taskIdx] = taskConfig;
}

template<size_t N, typename TaskConfig>
TaskConfig const* TaskConfigHolder<N, TaskConfig>::getTaskConfig(size_t const taskIdx)
{
    return &_taskConfigs[taskIdx];
}

template<size_t N>
void TaskConfigHolder<N, void>::setTaskConfig(
    size_t const /*taskIdx*/,
    typename TaskConfigHolder<N, void>::TaskConfigType const& /*taskConfig*/)
{}

template<size_t N>
typename TaskConfigHolder<N, void>::TaskConfigType const*
TaskConfigHolder<N, void>::getTaskConfig(size_t const /*taskIdx*/)
{
    estd_assert(false);
    return nullptr;
}

} // namespace internal

} // namespace async
