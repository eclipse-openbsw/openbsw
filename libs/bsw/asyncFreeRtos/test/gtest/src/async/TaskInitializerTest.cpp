// Copyright 2024 Accenture.

#include "async/TaskInitializer.h"

#include <etl/delegate.h>
#include <etl/singleton_base.h>
#include <etl/span.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::async::internal;
using namespace ::testing;

struct AdapterMock : public ::etl::singleton_base<AdapterMock>
{
    using StackSliceType   = ::etl::span<StackType_t>;
    using TaskFunctionType = ::etl::delegate<void()>;
    using TaskObjectType   = uint32_t*;
    using TaskConfigType   = uint32_t;

    AdapterMock() : ::etl::singleton_base<AdapterMock>(*this) {}

    static void initTask(TaskInitializer<AdapterMock>& taskInitializer)
    {
        instance().initTaskCall(taskInitializer);
    }

    MOCK_METHOD1(initTaskCall, void(TaskInitializer<AdapterMock>& taskInitializer));

    static ContextType const TASK_IDLE  = 0U;
    static ContextType const TASK_TIMER = 10U;
};

using Cut = TaskInitializer<AdapterMock>;

struct TaskInitializerData
{
    Cut::StackSliceType _stack;
    Cut::TaskFunctionType _taskFunction;
    Cut::TaskObjectType* _task  = nullptr;
    char const* _name           = nullptr;
    ContextType _context        = CONTEXT_INVALID;
    Cut::TaskConfigType _config = 0U;
};

struct TaskInitializerTest : public Test
{
    void taskFunction() {}

    AdapterMock _adapterMock;
    Cut::TaskFunctionType _taskFunction
        = Cut::TaskFunctionType::create<TaskInitializerTest, &TaskInitializerTest::taskFunction>(
            *this);
};

ACTION_P(CopyTaskInitializerData, dest)
{
    dest->_stack        = arg0._stack;
    dest->_taskFunction = arg0._taskFunction;
    dest->_task         = &arg0._task;
    dest->_name         = arg0._name;
    dest->_context      = arg0._context;
    dest->_config       = arg0._config;
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTaskInitializer
 * \desc: The test is to prove that IdleTask object is storing/sending the correct data
 */
TEST_F(TaskInitializerTest, testIdleTask)
{
    // with allocated stack
    {
        char const* name = "idle";
        IdleTask<AdapterMock, 256> idleTask(name, _taskFunction, 7U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(256, data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(_taskFunction, data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(ContextType(AdapterMock::TASK_IDLE), data._context);
        EXPECT_EQ(7U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with allocated stack, without function
    {
        char const* name = "idle";
        IdleTask<AdapterMock, 256> idleTask(name, 7U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(256, data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(Cut::TaskFunctionType(), data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(ContextType(AdapterMock::TASK_IDLE), data._context);
        EXPECT_EQ(7U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with input stack
    {
        char const* name = "idle";
        Stack<256> stack;
        IdleTask<AdapterMock> idleTask(name, stack, _taskFunction, 7U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(&stack[0], data._stack.data());
        EXPECT_EQ(sizeof(stack), data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(_taskFunction, data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(ContextType(AdapterMock::TASK_IDLE), data._context);
        EXPECT_EQ(7U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with input stack, without task function
    {
        char const* name = "idle";
        Stack<256> stack;
        IdleTask<AdapterMock> idleTask(name, stack, 7U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(&stack[0], data._stack.data());
        EXPECT_EQ(sizeof(stack), data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(Cut::TaskFunctionType(), data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(ContextType(AdapterMock::TASK_IDLE), data._context);
        EXPECT_EQ(7U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTaskInitializer
 * \desc: The test is to prove that TimerTask object is storing/sending the correct data
 */
TEST_F(TaskInitializerTest, testTimerTask)
{
    // with allocated stack
    {
        char const* name = "timer";
        TimerTask<AdapterMock, 512> timerTask(name, 9U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(512, data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(AdapterMock::TaskFunctionType(), data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(ContextType(AdapterMock::TASK_TIMER), data._context);
        EXPECT_EQ(9U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with input stack
    {
        char const* name = "timer";
        Stack<386> stack;
        TimerTask<AdapterMock> timerTask(name, stack, 8U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(&stack[0], data._stack.data());
        EXPECT_EQ(sizeof(stack), data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(AdapterMock::TaskFunctionType(), data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(ContextType(AdapterMock::TASK_TIMER), data._context);
        EXPECT_EQ(8U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTaskInitializer
 * \desc: The test is to prove that Task object is storing/sending the correct data
 */
TEST_F(TaskInitializerTest, testTask)
{
    // with allocated stack and default function
    {
        char const* name = "any";
        Task<AdapterMock, 5U, 512> task(name, 9U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(512, data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(AdapterMock::TaskFunctionType(), data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(5U, data._context);
        EXPECT_EQ(9U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with input stack and default function
    {
        char const* name = "any";
        Stack<386> stack;
        Task<AdapterMock, 6U> task(name, stack, 8U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(&stack[0], data._stack.data());
        EXPECT_EQ(sizeof(stack), data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(AdapterMock::TaskFunctionType(), data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(6U, data._context);
        EXPECT_EQ(8U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with allocated stack and user defined function
    {
        char const* name = "any";
        Task<AdapterMock, 5U, 512> task(name, _taskFunction, 9U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(512, data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(_taskFunction, data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(5U, data._context);
        EXPECT_EQ(9U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
    // with input stack and user defined function
    {
        char const* name = "any";
        Stack<386> stack;
        Task<AdapterMock, 6U> task(name, stack, _taskFunction, 8U);

        TaskInitializerData data;
        EXPECT_CALL(_adapterMock, initTaskCall(_)).WillOnce(CopyTaskInitializerData(&data));
        Cut::TaskInitializer::run();
        EXPECT_EQ(name, data._name);
        EXPECT_EQ(&stack[0], data._stack.data());
        EXPECT_EQ(sizeof(stack), data._stack.size() * sizeof(StackType_t));
        EXPECT_EQ(_taskFunction, data._taskFunction);
        EXPECT_TRUE(data._task != nullptr);
        EXPECT_EQ(6U, data._context);
        EXPECT_EQ(8U, data._config);
        Mock::VerifyAndClearExpectations(&_adapterMock);
    }
}

} // namespace
