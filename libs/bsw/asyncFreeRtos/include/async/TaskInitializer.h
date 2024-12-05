// Copyright 2024 Accenture.

#ifndef GUARD_0F3859C6_65E3_4B0E_A12F_392C07B839AD
#define GUARD_0F3859C6_65E3_4B0E_A12F_392C07B839AD

#include "async/StaticRunnable.h"
#include "async/Types.h"
#include "estd/array.h"
#include "estd/assert.h"
#include "estd/memory.h"
#include "estd/slice.h"

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

namespace async
{
namespace internal
{

constexpr size_t adjustStackSize(size_t const stackSize)
{
#if (defined(MINIMUM_STACK_SIZE)) && (MINIMUM_STACK_SIZE != 0)
    return (stackSize != 0U) && (stackSize < MINIMUM_STACK_SIZE) ? MINIMUM_STACK_SIZE : stackSize;
#else
    return stackSize;
#endif // (defined(MINIMUM_STACK_SIZE)) && (MINIMUM_STACK_SIZE != 0)
}

template<size_t StackSize>
using Stack = ::estd::array<
    StackType_t,
    (adjustStackSize(StackSize) + sizeof(StackType_t) - 1) / sizeof(StackType_t)>;

/**
 * The TaskInitializer struct centralizes the initialization of tasks within the application.
 *
 * This struct provides a standardized way to set up tasks, defining key types and handling
 * configuration and setup of task and timer objects. It uses a specified Adapter type
 * for flexibility, allowing for different configurations.
 *
 * \tparam Adapter The adapter type used to provide specific task configuration types and functions.
 */
template<typename Adapter>
struct TaskInitializer : public StaticRunnable<TaskInitializer<Adapter>>
{
    using AdapterType      = Adapter;
    using StackSliceType   = ::estd::slice<StackType_t>;
    using TaskConfigType   = typename AdapterType::TaskConfigType;
    using TaskFunctionType = typename AdapterType::TaskFunctionType;
    using TaskObjectType   = StaticTask_t;
    using TimerObjectType  = StaticTimer_t;

    /**
     * Creates and initializes a task with the specified parameters.
     *
     * This function handles task creation by allocating memory for the object,
     * setting up the context, stack, and configuration,
     * and linking them to the specified task and timer objects.
     *
     * \tparam T The type of the stack used by the task.
     * \param context The execution context of the task.
     * \param name The name of the task.
     * \param task The static task object to initialize.
     * \param timer The static timer object associated with the task.
     * \param stack The stack to use for the task's execution.
     * \param taskFunction The function that the task will execute.
     * \param config Configuration settings for the task.
     */
    template<typename T>
    static void create(
        ContextType context,
        char const* name,
        TaskObjectType& task,
        TimerObjectType& timer,
        T& stack,
        TaskFunctionType taskFunction,
        TaskConfigType const& config);

private:
    TaskInitializer(
        ContextType context,
        char const* name,
        TaskObjectType& task,
        TimerObjectType& timer,
        StackSliceType const& stack,
        TaskFunctionType taskFunction,
        TaskConfigType const& config);

public:
    /// Executes task object initialization.
    void execute();

    /// The stack slice used for task execution.
    StackSliceType _stack;

    /// The function assigned for the task's execution.
    TaskFunctionType _taskFunction;

    /// Reference to the static task object.
    TaskObjectType& _task;

    /// Reference to the static timer object associated with the task.
    TimerObjectType& _timer;

    /// The name of the task.
    char const* _name;

    /// The context in which the task will execute.
    ContextType _context;

    /// The configuration settings for the task.
    TaskConfigType _config;
};

/**
 * Primary template class that serves as a container for task-related objects and
 * configuration.
 *
 * TaskImpl provides a structure to hold essential components for tasks, such as the stack, task
 * object, and timer object, along with configuration settings. It serves as a base for derived task
 * classes (e.g., Task, IdleTask) and is designed for integration with FreeRTOS, using the Adapter
 * type for flexibility.
 *
 * \tparam Adapter The adapter type that supplies specific task configuration types and functions.
 * \tparam Context The context in which the task operates.
 * \tparam StackSize The size of the stack allocated for the task.
 */
template<class Adapter, ContextType Context, size_t StackSize = 0U>
class TaskImpl
{
public:
    using AdapterType      = Adapter;
    using StackSliceType   = ::estd::slice<StackType_t>;
    using TaskConfigType   = typename AdapterType::TaskConfigType;
    using TaskFunctionType = typename AdapterType::TaskFunctionType;
    using TaskObjectType   = StaticTask_t;
    using TimerObjectType  = StaticTimer_t;

    TaskImpl(char const* name, TaskFunctionType taskFunction, TaskConfigType const& taskConfig);

protected:
    ~TaskImpl() = default;

private:
    Stack<StackSize> _stack;
    TaskObjectType _task;
    TimerObjectType _timer;
};

template<class Adapter, ContextType Context>
class TaskImpl<Adapter, Context, 0U>
{
public:
    using AdapterType      = Adapter;
    using StackSliceType   = ::estd::slice<StackType_t>;
    using TaskConfigType   = typename AdapterType::TaskConfigType;
    using TaskFunctionType = typename AdapterType::TaskFunctionType;
    using TaskObjectType   = StaticTask_t;
    using TimerObjectType  = StaticTimer_t;

    template<typename T>
    TaskImpl(
        char const* name,
        T& stack,
        TaskFunctionType taskFunction,
        TaskConfigType const& taskConfig);

protected:
    ~TaskImpl() = default;

private:
    TaskObjectType _task;
    TimerObjectType _timer;
};

template<class Adapter, size_t StackSize = 0U>
struct IdleTask : public TaskImpl<Adapter, Adapter::TASK_IDLE, StackSize>
{
    using TaskFunctionType = typename Adapter::TaskFunctionType;
    using TaskConfigType   = typename Adapter::TaskConfigType;

    IdleTask(
        char const* name,
        TaskFunctionType taskFunction,
        TaskConfigType const& taskConfig = TaskConfigType());
};

template<class Adapter>
struct IdleTask<Adapter, 0U> : public TaskImpl<Adapter, Adapter::TASK_IDLE>
{
    using TaskFunctionType = typename Adapter::TaskFunctionType;
    using TaskConfigType   = typename Adapter::TaskConfigType;

    template<typename T>
    IdleTask(
        char const* name,
        T& stack,
        TaskFunctionType taskFunction,
        TaskConfigType const& taskConfig = TaskConfigType());
};

template<class Adapter, size_t StackSize = 0U>
struct TimerTask : public TaskImpl<Adapter, Adapter::TASK_TIMER, StackSize>
{
    using TaskConfigType = typename Adapter::TaskConfigType;

    explicit TimerTask(char const* name, TaskConfigType const& taskConfig = TaskConfigType());
};

template<class Adapter>
struct TimerTask<Adapter, 0U> : public TaskImpl<Adapter, Adapter::TASK_TIMER>
{
    using TaskConfigType = typename Adapter::TaskConfigType;

    template<typename T>
    TimerTask(char const* name, T& stack, TaskConfigType const& taskConfig = TaskConfigType());
};

template<class Adapter, ContextType Context, size_t StackSize = 0U>
struct Task : public TaskImpl<Adapter, Context, StackSize>
{
    using TaskFunctionType = typename Adapter::TaskFunctionType;
    using TaskConfigType   = typename Adapter::TaskConfigType;

    explicit Task(char const* name, TaskConfigType const& taskConfig = TaskConfigType());

    Task(
        char const* name,
        TaskFunctionType taskFunction,
        TaskConfigType const& taskConfig = TaskConfigType());
};

template<class Adapter, ContextType Context>
struct Task<Adapter, Context, 0U> : public TaskImpl<Adapter, Context>
{
    using TaskFunctionType = typename Adapter::TaskFunctionType;
    using TaskConfigType   = typename Adapter::TaskConfigType;

    template<typename T>
    explicit Task(char const* name, T& stack, TaskConfigType const& taskConfig = TaskConfigType());

    template<typename T>
    Task(
        char const* name,
        T& stack,
        TaskFunctionType taskFunction,
        TaskConfigType const& taskConfig = TaskConfigType());
};

template<class Adapter>
template<typename T>
void TaskInitializer<Adapter>::create(
    ContextType const context,
    char const* const name,
    TaskObjectType& task,
    TimerObjectType& timer,
    T& stack,
    TaskFunctionType const taskFunction,
    TaskConfigType const& config)
{
    ::estd::slice<uint8_t> bytes = ::estd::make_slice(stack).template reinterpret_as<uint8_t>();
    ::estd::memory::align(alignof(StackType_t), bytes);
    StackSliceType const stackSlice = bytes.template reinterpret_as<StackType_t>();
    estd_assert((stackSlice.size() * sizeof(StackType_t)) >= sizeof(TaskInitializer));
    new (stackSlice.data())
        TaskInitializer(context, name, task, timer, stackSlice, taskFunction, config);
}

template<class Adapter>
TaskInitializer<Adapter>::TaskInitializer(
    ContextType const context,
    char const* const name,
    TaskObjectType& task,
    TimerObjectType& timer,
    StackSliceType const& stack,
    TaskFunctionType const taskFunction,
    TaskConfigType const& config)
: _stack(stack)
, _taskFunction(taskFunction)
, _task(task)
, _timer(timer)
, _name(name)
, _context(context)
, _config(config)
{}

template<class Adapter>
void TaskInitializer<Adapter>::execute()
{
    Adapter::initTask(*this);
}

template<class Adapter, ContextType Context, size_t StackSize>
TaskImpl<Adapter, Context, StackSize>::TaskImpl(
    char const* const name, TaskFunctionType const taskFunction, TaskConfigType const& taskConfig)
{
    TaskInitializer<Adapter>::create(
        Context, name, _task, _timer, _stack, taskFunction, taskConfig);
}

template<class Adapter, ContextType Context>
template<typename T>
TaskImpl<Adapter, Context, 0U>::TaskImpl(
    char const* const name,
    T& stack,
    TaskFunctionType const taskFunction,
    TaskConfigType const& taskConfig)
{
    TaskInitializer<Adapter>::create(Context, name, _task, _timer, stack, taskFunction, taskConfig);
}

template<class Adapter, size_t StackSize>
IdleTask<Adapter, StackSize>::IdleTask(
    char const* const name, TaskFunctionType const taskFunction, TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Adapter::TASK_IDLE, StackSize>(name, taskFunction, taskConfig)
{}

template<class Adapter>
template<typename T>
IdleTask<Adapter, 0U>::IdleTask(
    char const* const name,
    T& stack,
    TaskFunctionType const taskFunction,
    TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Adapter::TASK_IDLE>(name, stack, taskFunction, taskConfig)
{}

template<class Adapter, size_t StackSize>
TimerTask<Adapter, StackSize>::TimerTask(char const* const name, TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Adapter::TASK_TIMER, StackSize>(
    name, typename Adapter::TaskFunctionType(), taskConfig)
{}

template<class Adapter>
template<typename T>
TimerTask<Adapter, 0U>::TimerTask(
    char const* const name, T& stack, TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Adapter::TASK_TIMER>(
    name, stack, typename Adapter::TaskFunctionType(), taskConfig)
{}

template<class Adapter, ContextType Context, size_t StackSize>
Task<Adapter, Context, StackSize>::Task(char const* const name, TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Context, StackSize>(name, TaskFunctionType(), taskConfig)
{}

template<class Adapter, ContextType Context, size_t StackSize>
Task<Adapter, Context, StackSize>::Task(
    char const* const name, TaskFunctionType const taskFunction, TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Context, StackSize>(name, taskFunction, taskConfig)
{}

template<class Adapter, ContextType Context>
template<typename T>
Task<Adapter, Context, 0U>::Task(char const* const name, T& stack, TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Context>(name, stack, typename Adapter::TaskFunctionType(), taskConfig)
{}

template<class Adapter, ContextType Context>
template<typename T>
Task<Adapter, Context, 0U>::Task(
    char const* const name,
    T& stack,
    TaskFunctionType const taskFunction,
    TaskConfigType const& taskConfig)
: TaskImpl<Adapter, Context>(name, stack, taskFunction, taskConfig)
{}

} // namespace internal
} // namespace async

#endif // GUARD_0F3859C6_65E3_4B0E_A12F_392C07B839AD
