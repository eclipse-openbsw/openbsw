// Copyright 2024 Accenture.

#include "async/FreeRtosAdapter.h"

#include "async/RunnableMock.h"
#include "async/TimeoutMock.h"

#include <bsp/timer/SystemTimerMock.h>
#include <os/FreeRtosMock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

struct TestStatistics
{};

struct TestBinding
{
    static size_t const TASK_COUNT             = 3U;
    static size_t const WAIT_EVENTS_TICK_COUNT = 100U;

    using ContextStatisticsType  = TestStatistics;
    using FunctionStatisticsType = TestStatistics;
};

class FreeRtosAdapterTest : public Test
{
public:
    using CutType = FreeRtosAdapter<TestBinding>;

    MOCK_METHOD0(startApp, void());
    MOCK_METHOD1(taskFunction, void(CutType::TaskContextType& taskContext));

protected:
    StrictMock<::os::FreeRtosMock> _freeRtosMock;
    StrictMock<RunnableMock> _runnableMock;
    TimeoutType _timeout;
    StrictMock<SystemTimerMock> _systemTimerMock;
};

/**
 * \req: [BSW_ASFR_39], [BSW_ASFR_44], [BSW_ASFR_56]
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapter, SMD_asyncFreeRtos_FreeRtosAdapterInitialization,
 * \desc: The test is to prove that corresponding
 * xTaskCreateStatic are called when init is executed
 */
TEST_F(FreeRtosAdapterTest, testCreateTask)
{
    {
        char const* name = "test";
        CutType::Task<1U, 256> task(name);
        uint32_t taskHandle = 12;

        Sequence s;
        EXPECT_CALL(
            _freeRtosMock,
            xTaskCreateStatic(
                NotNull(), name, 256U / sizeof(StackType_t), NotNull(), 1U, NotNull(), NotNull()))
            .WillOnce(Return(&taskHandle));
        EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
        CutType::run(CutType::StartAppFunctionType::
                         create<FreeRtosAdapterTest, &FreeRtosAdapterTest::startApp>(*this));
    }
    {
        char const* name = "test2";
        CutType::Task<2U, 512U> task(
            name,
            CutType::TaskFunctionType::
                create<FreeRtosAdapterTest, &FreeRtosAdapterTest::taskFunction>(*this));

        uint32_t taskHandle = 13U;
        EXPECT_CALL(
            _freeRtosMock,
            xTaskCreateStatic(
                NotNull(), name, 512U / sizeof(StackType_t), NotNull(), 2U, NotNull(), NotNull()))
            .WillOnce(Return(&taskHandle));
        EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
        CutType::run(CutType::StartAppFunctionType::
                         create<FreeRtosAdapterTest, &FreeRtosAdapterTest::startApp>(*this));
        {
            // Test get name
            EXPECT_EQ(name, CutType::getTaskName(2U));
        }
    }
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterConfig, SMD_asyncFreeRtos_FreeRtosAdapterInformational
 * \desc: To test the stack usage allotment for different tasks created
 */
TEST_F(FreeRtosAdapterTest, testStackUsage)
{
    {
        CutType::StackUsage stackUsage;
        char const* name = "test2";
        CutType::Task<2U, 128U> task(
            name,
            CutType::TaskFunctionType::
                create<FreeRtosAdapterTest, &FreeRtosAdapterTest::taskFunction>(*this));

        char const* name3 = "test3";
        CutType::Task<3U, 512U> task3(
            name3,
            CutType::TaskFunctionType::
                create<FreeRtosAdapterTest, &FreeRtosAdapterTest::taskFunction>(*this));
        uint32_t taskHandle = 13U;

        EXPECT_CALL(
            _freeRtosMock,
            xTaskCreateStatic(
                NotNull(), name, 128U / sizeof(StackType_t), NotNull(), 2U, NotNull(), NotNull()))
            .WillOnce(Return(&taskHandle));
        EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
        CutType::run(CutType::StartAppFunctionType::
                         create<FreeRtosAdapterTest, &FreeRtosAdapterTest::startApp>(*this));
        {
            EXPECT_EQ(name, CutType::getTaskName(2U));

            EXPECT_CALL(_freeRtosMock, uxTaskGetStackHighWaterMark(&taskHandle))
                .WillOnce(Return(1U));
            EXPECT_TRUE(CutType::getStackUsage(2U, stackUsage));
            EXPECT_EQ(stackUsage._usedSize, 56U);
            stackUsage._usedSize = 0;

            EXPECT_CALL(_freeRtosMock, xTimerGetTimerDaemonTaskHandle())
                .WillOnce(Return(&taskHandle));
            EXPECT_CALL(_freeRtosMock, uxTaskGetStackHighWaterMark(_)).WillOnce(Return(1U));
            EXPECT_TRUE(CutType::getStackUsage(CutType::TASK_COUNT, stackUsage));
            EXPECT_EQ(stackUsage._usedSize, 248);

            EXPECT_FALSE(CutType::getStackUsage(CutType::TASK_COUNT + 1, stackUsage));
        }
    }
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterInterrupts
 * \desc: To test the context switch during interrupt and high priority tasks
 */
TEST_F(FreeRtosAdapterTest, testInterruptScope)
{
    EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() == 0L);
    {
        // set interrupt
        CutType::enterIsr();
        EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() != 0L);
        EXPECT_EQ(pdFALSE, *CutType::getHigherPriorityTaskWoken());
        {
            // set nested interrupt
            CutType::enterIsr();
            EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() != 0L);
            EXPECT_EQ(pdFALSE, *CutType::getHigherPriorityTaskWoken());
            EXPECT_EQ(CONTEXT_INVALID, CutType::getCurrentTaskContext());
            // set task woken
            *CutType::getHigherPriorityTaskWoken() = pdTRUE;
            EXPECT_EQ(pdTRUE, *CutType::getHigherPriorityTaskWoken());
            CutType::leaveIsr();
        }
        // expect task woken value to be retained
        EXPECT_EQ(pdTRUE, *CutType::getHigherPriorityTaskWoken());
        // expect call to port yield due to flag being set
        EXPECT_CALL(_freeRtosMock, portYIELD_FROM_ISR(pdTRUE));
        CutType::leaveIsr();
    }
    Mock::VerifyAndClearExpectations(&_freeRtosMock);
    {
        // set interrupt
        CutType::enterIsr();
        EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() != 0L);
        EXPECT_EQ(pdFALSE, *CutType::getHigherPriorityTaskWoken());
        // expect nothing to happen on return
        CutType::leaveIsr();
    }
    Mock::VerifyAndClearExpectations(&_freeRtosMock);
    EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() == 0L);
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterInterrupts
 * \desc: To test the context switch during interrupt and high priority tasks
 */
TEST_F(FreeRtosAdapterTest, testInterruptScopeNoYield)
{
    EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() == 0L);
    {
        // set interrupt
        CutType::enterIsr();
        EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() != 0L);
        EXPECT_EQ(pdFALSE, *CutType::getHigherPriorityTaskWoken());
        {
            // set nested interrupt
            CutType::enterIsr();
            EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() != 0L);
            EXPECT_EQ(pdFALSE, *CutType::getHigherPriorityTaskWoken());
            // set task woken
            *CutType::getHigherPriorityTaskWoken() = pdTRUE;
            EXPECT_EQ(pdTRUE, *CutType::getHigherPriorityTaskWoken());
            CutType::leaveIsrNoYield();
        }
        // expect task woken value to be retained
        EXPECT_EQ(pdTRUE, *CutType::getHigherPriorityTaskWoken());
        // expect no call to port yield due to flag being set
        CutType::leaveIsrNoYield();
    }
    Mock::VerifyAndClearExpectations(&_freeRtosMock);
    EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() == 0L);
    {
        // expect flag to be reset
        CutType::enterIsr();
        EXPECT_TRUE(CutType::getHigherPriorityTaskWoken() != 0L);
        EXPECT_EQ(pdFALSE, *CutType::getHigherPriorityTaskWoken());
        // expect no call to port yield due to flag being set
        CutType::leaveIsrNoYield();
    }
    Mock::VerifyAndClearExpectations(&_freeRtosMock);
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterInitialization
 * \desc: To test the run function after init
 */
TEST_F(FreeRtosAdapterTest, testRun)
{
    EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
    CutType::run(
        CutType::StartAppFunctionType::create<FreeRtosAdapterTest, &FreeRtosAdapterTest::startApp>(
            *this));
}

/**
 * \req: [BSW_ASFR_58], [BSW_ASFR_59], [BSW_ASFR_60]
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterAsyncApi, SMD_asyncFreeRtos_TimeoutType
 * \desc: To test the schedule functionality and the corresponding call to xTaskNotify
 */
TEST_F(FreeRtosAdapterTest, testExecuteAndScheduleCalls)
{
    char const* name = "test";
    CutType::Task<1U, 256U> task(name);
    uint32_t taskHandle = 12;
    EXPECT_CALL(
        _freeRtosMock,
        xTaskCreateStatic(
            NotNull(), name, 256U / sizeof(StackType_t), NotNull(), 1U, NotNull(), NotNull()))
        .WillOnce(Return(&taskHandle));
    EXPECT_CALL(_freeRtosMock, vTaskStartScheduler());
    CutType::run(
        CutType::StartAppFunctionType::create<FreeRtosAdapterTest, &FreeRtosAdapterTest::startApp>(
            *this));
    {
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&taskHandle, _, eSetBits));
        CutType::execute(1U, _runnableMock);
    }
    {
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillOnce(Return(100U));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&taskHandle, _, eSetBits));
        CutType::schedule(1U, _runnableMock, _timeout, 100U, TimeUnit::MICROSECONDS);
        CutType::cancel(_timeout);
        // cancel again should be neutral
        CutType::cancel(_timeout);
    }
    {
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillOnce(Return(100U));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&taskHandle, _, eSetBits));
        CutType::scheduleAtFixedRate(1U, _runnableMock, _timeout, 90U, TimeUnit::MICROSECONDS);
        CutType::cancel(_timeout);
    }
}

/**
 * \refs: SMD_asyncFreeRtos_TaskContextInformational
 * \desc: To test the minimum stack size
 */
TEST_F(FreeRtosAdapterTest, testMinStackSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    using TaskType = CutType::Task<1U, 4U>;

    EXPECT_THROW({ TaskType task("test"); }, ::estd::assert_exception);
}

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTimerTask
 * \desc: To test user defined config for tasks
 */
TEST_F(FreeRtosAdapterTest, taskConfigUserDefined)
{
    size_t const TASK_COUNT = 3U;
    using TaskConfig        = async::internal::TaskConfigHolder<TASK_COUNT + 1, int>;
    int TaskConfig1         = 100;
    TaskConfig::setTaskConfig(1U, TaskConfig1);
    EXPECT_EQ(*TaskConfig::getTaskConfig(1U), 100U);
}

} // namespace
