// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_DB9C7111_F66E_4D50_B724_5DF992A19ED0
#define GUARD_DB9C7111_F66E_4D50_B724_5DF992A19ED0

#include "async/TaskContext.h"
#include "async/TaskInitializer.h"

#ifndef ASYNC_LOOPER_DISABLE
#include <looper/looper.h>
#endif // ASYNC_LOOPER_DISABLE

#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
#include <util/timeout/AsyncTimeoutManager.h>
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE

#include <estd/array.h>
#include <estd/singleton.h>

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
    static ::estd::array<TaskConfigType, N> _taskConfigs;
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

template<class Binding>
class FreeRtosAdapter
{
public:
    static size_t const TASK_COUNT          = Binding::TASK_COUNT;
    static size_t const FREERTOS_TASK_COUNT = TASK_COUNT + 1U;
    static TickType_t const WAIT_EVENTS_TICK_COUNT
        = static_cast<TickType_t>(Binding::WAIT_EVENTS_TICK_COUNT);
    static ContextType const TASK_IDLE  = 0U;
    static ContextType const TASK_TIMER = static_cast<ContextType>(TASK_COUNT);

    using AdapterType = FreeRtosAdapter<Binding>;

    using TaskContextType  = TaskContext<AdapterType>;
    using TaskFunctionType = typename TaskContextType::TaskFunctionType;

    using TaskConfigsType = internal::TaskConfigHolder<FREERTOS_TASK_COUNT>;
    using TaskConfigType  = typename TaskConfigsType::TaskConfigType;

    template<size_t StackSize>
    using Stack           = internal::Stack<StackSize>;
    using TaskInitializer = internal::TaskInitializer<AdapterType>;

    template<size_t StackSize = 0U>
    using IdleTask = internal::IdleTask<AdapterType, StackSize>;
    template<size_t StackSize = 0U>
    using TimerTask = internal::TimerTask<AdapterType, StackSize>;
    template<ContextType Context, size_t StackSize = 0U>
    using Task = internal::Task<AdapterType, Context, StackSize>;

    struct StackUsage
    {
        StackUsage();

        uint32_t _stackSize;
        uint32_t _usedSize;
    };

    template<ContextType Context>
    static void getTaskMemory(
        StaticTask_t** ppxTaskTCBBuffer,
        StackType_t** ppxTaskStackBuffer,
        uint32_t* pulTaskStackSize);

    static char const* getTaskName(size_t taskIdx);

    static TaskHandle_t getTaskHandle(size_t taskIdx);

    static TaskConfigType const* getTaskConfig(size_t taskIdx);

    static ContextType getCurrentTaskContext();

#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
    static ::common::ITimeoutManager2& getTimeoutManager(ContextType context);
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE

#ifndef ASYNC_LOOPER_DISABLE
    static ::loop::looper& getLooper(ContextType context);
#endif // ASYNC_LOOPER_DISABLE

    static void init();

    static void run();

    static void runningHook();

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

    static void enterIsr();

    static void leaveIsr();

    static bool leaveIsrNoYield();

    static BaseType_t* getHigherPriorityTaskWoken();

private:
    friend struct internal::TaskInitializer<AdapterType>;

    static void initTask(TaskInitializer& initializer);

    static void initTask(ContextType context, char const* name, StaticTimer_t& timer);

    static void TaskFunction(void* param);

    static TaskInitializer* _idleTaskInitializer;
    static TaskInitializer* _timerTaskInitializer;
    static ::estd::array<TaskContextType, TASK_COUNT> _taskContexts;
#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
    static ::estd::array<::common::AsyncTimeoutManager, TASK_COUNT> _timeoutManagers;
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE
#ifndef ASYNC_LOOPER_DISABLE
    static ::estd::array<::loop::looper, TASK_COUNT> _loopers;
#endif // ASYNC_LOOPER_DISABLE
    static ::estd::array<uint32_t, FREERTOS_TASK_COUNT> _stackSizes;
    static char const* _timerTaskName;
    static BaseType_t _higherPriorityTaskWokenFlag;
    static BaseType_t* _higherPriorityTaskWoken;
    static uint8_t _nestedInterruptCount;
};

/**
 * Inline implementations.
 */
template<class Binding>
typename FreeRtosAdapter<Binding>::TaskInitializer* FreeRtosAdapter<Binding>::_idleTaskInitializer
    = nullptr;
template<class Binding>
typename FreeRtosAdapter<Binding>::TaskInitializer* FreeRtosAdapter<Binding>::_timerTaskInitializer
    = nullptr;
template<class Binding>
::estd::
    array<typename FreeRtosAdapter<Binding>::TaskContextType, FreeRtosAdapter<Binding>::TASK_COUNT>
        FreeRtosAdapter<Binding>::_taskContexts;
#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
template<class Binding>
::estd::array<::common::AsyncTimeoutManager, FreeRtosAdapter<Binding>::TASK_COUNT>
    FreeRtosAdapter<Binding>::_timeoutManagers;
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE
#ifndef ASYNC_LOOPER_DISABLE
template<class Binding>
::estd::array<::loop::looper, FreeRtosAdapter<Binding>::TASK_COUNT>
    FreeRtosAdapter<Binding>::_loopers;
#endif // ASYNC_LOOPER_DISABLE
template<class Binding>
::estd::array<uint32_t, FreeRtosAdapter<Binding>::FREERTOS_TASK_COUNT>
    FreeRtosAdapter<Binding>::_stackSizes;
template<class Binding>
char const* FreeRtosAdapter<Binding>::_timerTaskName;
template<class Binding>
BaseType_t FreeRtosAdapter<Binding>::_higherPriorityTaskWokenFlag = 0;
template<class Binding>
BaseType_t* FreeRtosAdapter<Binding>::_higherPriorityTaskWoken = nullptr;
template<class Binding>
uint8_t FreeRtosAdapter<Binding>::_nestedInterruptCount = 0U;

template<class Binding>
template<ContextType Context>
void FreeRtosAdapter<Binding>::getTaskMemory(
    StaticTask_t** const ppxTaskTCBBuffer,
    StackType_t** const ppxTaskStackBuffer,
    uint32_t* const pulTaskStackSize)
{
    TaskInitializer& initializer
        = *((Context == TASK_IDLE) ? _idleTaskInitializer : _timerTaskInitializer);
    estd_assert(ppxTaskTCBBuffer != nullptr);
    estd_assert(ppxTaskStackBuffer != nullptr);
    estd_assert(pulTaskStackSize != nullptr);
    *ppxTaskTCBBuffer   = &initializer._task;
    *ppxTaskStackBuffer = initializer._stack.data();
    // Conversion to uint32_t is OK, stack will not exceed 4GB.
    *pulTaskStackSize   = static_cast<uint32_t>(initializer._stack.size());
}

template<class Binding>
inline char const* FreeRtosAdapter<Binding>::getTaskName(size_t const taskIdx)
{
    return (taskIdx < _taskContexts.size()) ? _taskContexts[taskIdx].getName() : _timerTaskName;
}

template<class Binding>
inline TaskHandle_t FreeRtosAdapter<Binding>::getTaskHandle(size_t const taskIdx)
{
    return _taskContexts[taskIdx].getTaskHandle();
}

template<class Binding>
inline typename FreeRtosAdapter<Binding>::TaskConfigType const*
FreeRtosAdapter<Binding>::getTaskConfig(size_t const taskIdx)
{
    return TaskConfigsType::getTaskConfig(taskIdx);
}

template<class Binding>
ContextType FreeRtosAdapter<Binding>::getCurrentTaskContext()
{
    if (_higherPriorityTaskWoken == nullptr)
    {
        return static_cast<ContextType>(uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle()));
    }
    return CONTEXT_INVALID;
}

template<class Binding>
void FreeRtosAdapter<Binding>::initTask(TaskInitializer& initializer)
{
    ContextType const context                 = initializer._context;
    _stackSizes[static_cast<size_t>(context)] = static_cast<uint32_t>(
        static_cast<size_t>(initializer._stack.size()) * sizeof(BaseType_t));
    TaskConfigsType::setTaskConfig(context, initializer._config);
    if (context == TASK_TIMER)
    {
        _timerTaskInitializer = &initializer;
        _timerTaskName        = initializer._name;
    }
    else
    {
        initTask(context, initializer._name, initializer._timer);
        TaskContextType& taskContext = _taskContexts[static_cast<size_t>(context)];
        if (context == TASK_IDLE)
        {
            _idleTaskInitializer = &initializer;
            taskContext.initTask(TASK_IDLE, initializer._taskFunction);
        }
        else
        {
            taskContext.createTask(
                context,
                initializer._task,
                initializer._name,
                static_cast<UBaseType_t>(context),
                initializer._stack,
                initializer._taskFunction);
        }
    }
}

template<class Binding>
void FreeRtosAdapter<Binding>::initTask(
    ContextType const context, char const* const name, StaticTimer_t& timer)
{
    TaskContextType& taskContext = _taskContexts[static_cast<size_t>(context)];
    taskContext.createTimer(timer, name);
#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
    _timeoutManagers[static_cast<size_t>(context)].init(context);
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE
#ifndef ASYNC_LOOPER_DISABLE
    _loopers[static_cast<size_t>(context)].init(context);
#endif // ASYNC_LOOPER_DISABLE
}

template<class Binding>
void FreeRtosAdapter<Binding>::init()
{
    TaskInitializer::run();
}

template<class Binding>
void FreeRtosAdapter<Binding>::run()
{
    vTaskStartScheduler();
}

template<class Binding>
void FreeRtosAdapter<Binding>::runningHook()
{
    _taskContexts[TASK_IDLE].initTaskHandle(xTaskGetIdleTaskHandle());
}

template<class Binding>
bool FreeRtosAdapter<Binding>::getStackUsage(size_t const taskIdx, StackUsage& stackUsage)
{
    if (taskIdx < FREERTOS_TASK_COUNT)
    {
        stackUsage._stackSize = _stackSizes[taskIdx];
        uint32_t const unusedSize
            = (taskIdx == TASK_TIMER)
                  ? TaskContextType::getUnusedStackSize(xTimerGetTimerDaemonTaskHandle())
                  : _taskContexts[taskIdx].getUnusedStackSize();
        stackUsage._usedSize = stackUsage._stackSize - unusedSize;
        return true;
    }
    return false;
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::callIdleTaskFunction()
{
    _taskContexts[TASK_IDLE].callTaskFunction();
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::execute(ContextType const context, RunnableType& runnable)
{
    _taskContexts[static_cast<size_t>(context)].execute(runnable);
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::schedule(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    _taskContexts[static_cast<size_t>(context)].schedule(runnable, timeout, delay, unit);
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::scheduleAtFixedRate(
    ContextType const context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t const delay,
    TimeUnitType const unit)
{
    _taskContexts[static_cast<size_t>(context)].scheduleAtFixedRate(runnable, timeout, delay, unit);
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::cancel(TimeoutType& timeout)
{
    LockType const lock;
    ContextType const context = timeout._context;
    if (context != CONTEXT_INVALID)
    {
        timeout._context = CONTEXT_INVALID;
        _taskContexts[static_cast<size_t>(context)].cancel(timeout);
    }
}

#ifndef ASYNC_LOOPER_DISABLE
template<class Binding>
::loop::looper& FreeRtosAdapter<Binding>::getLooper(ContextType const context)
{
    return _loopers[static_cast<size_t>(context)];
}
#endif // ASYNC_LOOPER_DISABLE

#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
template<class Binding>
inline ::common::ITimeoutManager2&
FreeRtosAdapter<Binding>::getTimeoutManager(ContextType const context)
{
    return _timeoutManagers[static_cast<size_t>(context)];
}
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE

template<class Binding>
inline BaseType_t* FreeRtosAdapter<Binding>::getHigherPriorityTaskWoken()
{
    return _higherPriorityTaskWoken;
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::enterIsr()
{
    internal::NestedInterruptLock<> const lock;
    ++_nestedInterruptCount;
    _higherPriorityTaskWoken = &FreeRtosAdapter::_higherPriorityTaskWokenFlag;
}

template<class Binding>
inline void FreeRtosAdapter<Binding>::leaveIsr()
{
    internal::NestedInterruptLock<> const lock;
    --_nestedInterruptCount;
    if (_nestedInterruptCount == 0U)
    {
        FreeRtosAdapter::_higherPriorityTaskWoken = nullptr;
        if (FreeRtosAdapter::_higherPriorityTaskWokenFlag != pdFALSE)
        {
            FreeRtosAdapter::_higherPriorityTaskWokenFlag = pdFALSE;
            portYIELD_FROM_ISR(pdTRUE);
        }
    }
}

template<class Binding>
inline bool FreeRtosAdapter<Binding>::leaveIsrNoYield()
{
    internal::NestedInterruptLock<> const lock;
    --_nestedInterruptCount;
    if (_nestedInterruptCount == 0U)
    {
        FreeRtosAdapter::_higherPriorityTaskWoken = nullptr;
        if (FreeRtosAdapter::_higherPriorityTaskWokenFlag != pdFALSE)
        {
            FreeRtosAdapter::_higherPriorityTaskWokenFlag = pdFALSE;
            return true;
        }
    }
    return false;
}

template<class Binding>
FreeRtosAdapter<Binding>::StackUsage::StackUsage() : _stackSize(0U), _usedSize(0U)
{}

namespace internal
{
template<size_t N, typename TaskConfig>
::estd::array<TaskConfig, N> TaskConfigHolder<N, TaskConfig>::_taskConfigs;

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

#endif // GUARD_DB9C7111_F66E_4D50_B724_5DF992A19ED0
