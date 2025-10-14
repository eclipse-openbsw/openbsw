// Copyright 2024 Accenture.

#include "async/Async.h"

#include "async/AsyncBinding.h"
#include "async/ContextHookMock.h"
#include "async/Hook.h"
#include "async/RunnableMock.h"
#include "async/TickHookMock.h"
#include "async/TimeoutMock.h"
#include "freertos_tasks_c_additions.h"

#include <bsp/timer/SystemTimerMock.h>
#include <os/FreeRtosMock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

extern "C"
{
void vApplicationIdleHook();
void vApplicationGetIdleTaskMemory(
    StaticTask_t** ppxIdleTaskTCBBuffer,
    StackType_t** ppxIdleTaskStackBuffer,
    uint32_t* pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(
    StaticTask_t** ppxTimerTaskTCBBuffer,
    StackType_t** ppxTimerTaskStackBuffer,
    uint32_t* pulTimerTaskStackSize);
}

using AdapterType = AsyncBindingType::AdapterType;

class AsyncTest : public Test
{
public:
    MOCK_METHOD0(startApp, void());
    MOCK_METHOD1(taskFunction, void(AdapterType::TaskContextType& taskContext));

protected:
    StrictMock<::os::FreeRtosMock> _freeRtosMock;
    StrictMock<ContextHookMock> _contextHookMock;
    StrictMock<TickHookMock> _tickHookMock;
    StrictMock<RunnableMock> _runnableMock;
    TimeoutType _timeout;
    StrictMock<SystemTimerMock> _systemTimerMock;
};

/**
 * \req: [BSW_ASFR_58], [BSW_ASFR_59], [BSW_ASFR_60]
 * \refs: SMD_asyncFreeRtos_API
 * \desc: To test the schedule functionality and the corresponding call to xTaskNotify
 */
TEST_F(AsyncTest, testExecuteAndScheduleCalls)
{
    char const* name    = "test";
    uint32_t taskHandle = 12;
    EXPECT_CALL(
        _freeRtosMock,
        xTaskCreateStatic(
            NotNull(), name, 256U / sizeof(StackType_t), NotNull(), 1U, NotNull(), NotNull()))
        .WillOnce(Return(&taskHandle));
    EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());

    AdapterType::Task<1U, 256U> task(name);
    AdapterType::run(
        AdapterType::StartAppFunctionType::create<AsyncTest, &AsyncTest::startApp>(*this));
    {
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&taskHandle, _, eSetBits));
        execute(1U, _runnableMock);
    }
    {
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillOnce(Return(100U));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&taskHandle, _, eSetBits));
        schedule(1U, _runnableMock, _timeout, 100U, TimeUnit::MICROSECONDS);
        _timeout.cancel();
    }
    {
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillOnce(Return(100U));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&taskHandle, _, eSetBits));
        scheduleAtFixedRate(1U, _runnableMock, _timeout, 90U, TimeUnit::MICROSECONDS);
        _timeout.cancel();
    }
    {
        _timeout._runnable = 0L;
        _timeout.expired();
    }
}

/**
 * \req: [BSW_ASFR_41]
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTask, SMD_asyncFreeRtos_FreeRtosAdapterIdleTask
 * \desc: To test the idle task init
 */
TEST_F(AsyncTest, testInitIdleTask)
{
    char const* name = "idle";
    AdapterType::IdleTask<384U> idleTask(
        name, AdapterType::TaskFunctionType::create<AsyncTest, &AsyncTest::taskFunction>(*this));
    {
        EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
        AdapterType::run(
            AdapterType::StartAppFunctionType::create<AsyncTest, &AsyncTest::startApp>(*this));
        EXPECT_EQ(name, AdapterType::getTaskName(AdapterType::TASK_IDLE));
    }
    {
        // expect stack to be returned on API call
        StaticTask_t* task = 0L;
        StackType_t* stack = 0L;
        uint32_t stackSize = 0L;

        vApplicationGetIdleTaskMemory(&task, &stack, &stackSize);
        EXPECT_TRUE(task != 0L);
        EXPECT_TRUE(stack != 0L);
        EXPECT_EQ(384U / sizeof(StackType_t), stackSize);
    }
    {
        uint32_t taskHandle = 12;
        EXPECT_CALL(_freeRtosMock, xTaskGetIdleTaskHandle()).WillOnce(Return(&taskHandle));
        EXPECT_CALL(*this, startApp());
        asyncInitialized();
        ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
        EXPECT_THROW({ asyncGetTaskConfig(taskHandle); }, ::estd::assert_exception);
        // check idle handle
        EXPECT_EQ(&taskHandle, AdapterType::getTaskHandle(AdapterType::TASK_IDLE));
    }
    {
        // expect task function to be called on vApplicationIdleHook();
        EXPECT_CALL(*this, taskFunction(_));
        vApplicationIdleHook();
    }
}

/**
 * \req: [BSW_ASFR_42], [BSW_ASFR_43]
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTask, SMD_asyncFreeRtos_FreeRtosAdapterTimerTask
 * \desc: To test the timer task init
 */
TEST_F(AsyncTest, testInitTimerTask)
{
    char const* name = "timer";
    AdapterType::TimerTask<768U> timerTask(name);
    {
        EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
        AdapterType::run(
            AdapterType::StartAppFunctionType::create<AsyncTest, &AsyncTest::startApp>(*this));
        EXPECT_EQ(name, AdapterType::getTaskName(AdapterType::TASK_TIMER));
    }
    {
        // expect stack to be returned on API call
        StaticTask_t* task = 0L;
        StackType_t* stack = 0L;
        uint32_t stackSize = 0L;

        vApplicationGetTimerTaskMemory(&task, &stack, &stackSize);
        EXPECT_TRUE(task != 0L);
        EXPECT_TRUE(stack != 0L);
        EXPECT_EQ(768U / sizeof(StackType_t), stackSize);
    }
}

/**
 * \req: [BSW_ASFR_49]
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterInitialization, SMD_asyncFreeRtos_FreeRtosAdapterIdle,
 * \desc: To test if the async hooks are called
 */
TEST_F(AsyncTest, testHooks)
{
    BaseType_t* getHigherPriorityTaskWokenFlag;
    {
        EXPECT_CALL(_contextHookMock, doEnterTask(123U));
        asyncEnterTask(123U);
        Mock::VerifyAndClearExpectations(&_contextHookMock);
    }
    {
        EXPECT_CALL(_contextHookMock, doLeaveTask(125U));
        asyncLeaveTask(125U);
        Mock::VerifyAndClearExpectations(&_contextHookMock);
    }
    {
        EXPECT_FALSE(AdapterType::getHigherPriorityTaskWoken() != 0L);
        EXPECT_CALL(_contextHookMock, doEnterIsrGroup(23U));
        asyncEnterIsrGroup(23U);
        Mock::VerifyAndClearExpectations(&_contextHookMock);
        EXPECT_TRUE(AdapterType::getHigherPriorityTaskWoken() != 0L);
        getHigherPriorityTaskWokenFlag = AdapterType::getHigherPriorityTaskWoken();
    }
    {
        EXPECT_CALL(_contextHookMock, doLeaveIsrGroup(24U));
        asyncLeaveIsrGroup(24U);
        Mock::VerifyAndClearExpectations(&_contextHookMock);
        EXPECT_FALSE(AdapterType::getHigherPriorityTaskWoken() != 0L);
    }
    {
        EXPECT_CALL(_tickHookMock, doHandleTick());
        asyncTickHook();
        Mock::VerifyAndClearExpectations(&_tickHookMock);
    }
    {
        *getHigherPriorityTaskWokenFlag = pdTRUE;
        EXPECT_CALL(_tickHookMock, doHandleTick());
        asyncTickHook();
        Mock::VerifyAndClearExpectations(&_tickHookMock);
    }
}

} // namespace*/
