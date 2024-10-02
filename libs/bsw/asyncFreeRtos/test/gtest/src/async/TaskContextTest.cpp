// Copyright 2024 Accenture.

#include "async/TaskContext.h"

#include "async/RunnableMock.h"
#include "async/TimeoutMock.h"

#include <bsp/timer/SystemTimerMock.h>
#include <os/FreeRtosMock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

ACTION_P(StopDispatch, cut) { cut->stopDispatch(); }

ACTION_P(CopyArgPointee2, pointer) { *arg2 = *pointer; }

class TestBindingMock : public ::estd::singleton<TestBindingMock>
{
public:
    static EventMaskType const WAIT_EVENTS_TICK_COUNT = 100U;
    using BaseType_t                                  = int32_t;

    TestBindingMock() : ::estd::singleton<TestBindingMock>(*this) {}

    static BaseType_t* getHigherPriorityTaskWoken()
    {
        return instance().getHigherPriorityTaskWokenFunc();
    }

    MOCK_METHOD0(getHigherPriorityTaskWokenFunc, BaseType_t*());

    MOCK_METHOD1(taskFunction, void(TaskContext<TestBindingMock>& taskContext));
};

class TaskContextTest : public Test
{
public:
    TaskContextTest() : _name("test"), _taskHandle(10U), _timerHandle(14U) {}

protected:
    StrictMock<::os::FreeRtosMock> _freeRtosMock;
    StrictMock<TestBindingMock> _bindingMock;
    StrictMock<RunnableMock> _runnableMock1;
    StrictMock<RunnableMock> _runnableMock2;
    TimeoutType _timeout1;
    TimeoutType _timeout2;
    StrictMock<SystemTimerMock> _systemTimerMock;
    StaticTask_t _task;
    StaticTimer_t _timer;
    StackType_t _stack[100];
    char const* _name;
    uint32_t _taskHandle;
    uint32_t _timerHandle;
};

/**
 * \req: [BSW_ASFR_39], [BSW_ASFR_52], [BSW_ASFR_54], [BSW_ASFR_57]
 * \refs: SMD_asyncFreeRtos_TaskContext, SMD_asyncFreeRtos_TaskContextEventHandling
 * \desc: To test task creation and execution
 */
TEST_F(TaskContextTest, testCreateAndRunTask)
{
    TaskContext<TestBindingMock> cut;
    {
        // With default function
        TaskFunction_t* osTaskFunction = 0L;
        void* param                    = 0L;
        EXPECT_CALL(
            _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
            .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), SaveArg<3>(&param), Return(&_taskHandle)));
        cut.createTask(
            1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());
        EXPECT_TRUE(osTaskFunction != 0L);
        // expect notify wait
        Sequence seq;
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(0U));
        // second time do nothing again
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut)));
        // trigger shutdown on next iteration
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .InSequence(seq)
            .WillOnce(SaveArg<1>(&eventMask));
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(CopyArgPointee2(&eventMask));
        osTaskFunction(param);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
    }
    {
        // With user defined function
        TaskFunction_t* osTaskFunction                              = 0L;
        void* param                                                 = 0L;
        TaskContext<TestBindingMock>::TaskFunctionType taskFunction = TaskContext<TestBindingMock>::
            TaskFunctionType::create<TestBindingMock, &TestBindingMock::taskFunction>(_bindingMock);
        EXPECT_CALL(
            _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
            .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), SaveArg<3>(&param), Return(&_taskHandle)));
        cut.createTask(1U, _task, _name, 12U, _stack, taskFunction);
        EXPECT_TRUE(osTaskFunction != 0L);
        // expect call to the task function
        EXPECT_CALL(_bindingMock, taskFunction(Ref(cut)));
        osTaskFunction(param);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
    }
}

/**
 * \refs: SMD_asyncFreeRtos_TaskContextInitialization
 * \desc: To test timer creation
 */
TEST_F(TaskContextTest, testCreateTimer)
{
    TaskContext<TestBindingMock> cut;
    EXPECT_EQ(0L, cut.getName());
    StaticTimer_t timer;
    char const* name     = "test";
    uint32_t timerHandle = 12U;
    EXPECT_CALL(_freeRtosMock, xTimerCreateStatic(name, 1U, 0U, NotNull(), NotNull(), &timer))
        .WillOnce(Return(&timerHandle));
    EXPECT_CALL(_freeRtosMock, pcTimerGetName(&timerHandle)).WillOnce(Return(name));
    cut.createTimer(timer, name);
    EXPECT_EQ(name, cut.getName());
}

/**
 * \req: [BSW_ASFR_51], [BSW_ASFR_52], [BSW_ASFR_53]
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi
 * \desc: To test task execution
 */
TEST_F(TaskContextTest, testExecute)
{
    TaskContext<TestBindingMock> cut;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(Return(&_taskHandle));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());
    {
        // from task context
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        uint32_t eventMask = 0U;
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.execute(_runnableMock1);
        Mock::VerifyAndClearExpectations(&_bindingMock);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);

        Sequence seq;
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(eventMask));
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(0U));
        EXPECT_CALL(_runnableMock1, execute());
        cut.dispatchWhileWork();
        Mock::VerifyAndClearExpectations(&_runnableMock1);
    }
    {
        BaseType_t higherPriorityTaskWoken = 0U;
        // from interrupt context
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(&higherPriorityTaskWoken));
        uint32_t eventMask = 0U;
        EXPECT_CALL(
            _freeRtosMock, xTaskNotifyFromISR(&_taskHandle, _, eSetBits, &higherPriorityTaskWoken))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.execute(_runnableMock1);
        Mock::VerifyAndClearExpectations(&_bindingMock);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);

        Sequence seq;
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(eventMask));
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(0U));
        EXPECT_CALL(_runnableMock1, execute());
        cut.dispatchWhileWork();
        Mock::VerifyAndClearExpectations(&_runnableMock1);
    }
}

/**
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi
 * \desc: To test timeout functionality
 */
TEST_F(TaskContextTest, testSetTimeout)
{
    TaskContext<TestBindingMock> cut;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(Return(&_taskHandle));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());
    {
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        uint32_t eventMask = 0U;
        // set timeout may be called with 0 value and should immediately set event
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.setTimeout(0U);
        EXPECT_TRUE(eventMask != 0U);
    }
}

/**
 * \req: [BSW_ASFR_51], [BSW_ASFR_66], [BSW_ASFR_67]
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi, SMD_asyncFreeRtos_TaskContextTaskLoop
 * \desc: To test task schedule functionality
 */
TEST_F(TaskContextTest, testSchedule)
{
    TaskContext<TestBindingMock> cut;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(Return(&_taskHandle));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());

    void* timerId                            = 0L;
    TimerCallbackFunction_t callbackFunction = 0L;
    EXPECT_CALL(_freeRtosMock, xTimerCreateStatic(_name, 1U, 0U, _, NotNull(), &_timer))
        .WillOnce(
            DoAll(SaveArg<3>(&timerId), SaveArg<4>(&callbackFunction), Return(&_timerHandle)));
    cut.createTimer(_timer, _name);
    // now trigger timer the first time
    EXPECT_CALL(_freeRtosMock, pvTimerGetTimerID(&_timerHandle)).WillRepeatedly(Return(timerId));

    {
        // expect task notify
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        callbackFunction(&_timerHandle);

        // set timer
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

        // dispatch and expect system time. Nothing should happen
        Sequence seq;
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(eventMask));
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(0U));
        cut.dispatchWhileWork();
    }

    {
        // Schedule first timer. Expect task notify
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.schedule(_runnableMock1, _timeout1, 150U, TimeUnit::MILLISECONDS);

        // Scheduling again shouldn't cause trouble
        cut.schedule(_runnableMock1, _timeout1, 300U, TimeUnit::MILLISECONDS);

        // Schedule second timer (to elapse laster). No task notify expected
        cut.schedule(_runnableMock2, _timeout2, 151U, TimeUnit::MILLISECONDS);

        {
            // timer expires too early
            EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(249000U));
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .WillOnce(SaveArg<1>(&eventMask));
            callbackFunction(&_timerHandle);

            // Cancel the second timer, nothing should happen.
            cut.cancel(_timeout2);

            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(eventMask));
            EXPECT_CALL(_freeRtosMock, xTimerChangePeriod(&_timerHandle, 10U, 0U));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(0U));
            cut.dispatchWhileWork();
        }

        {
            // timer expires at correct time
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .WillOnce(SaveArg<1>(&eventMask));
            callbackFunction(&_timerHandle);

            // Expect expired
            EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(250000U));

            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(eventMask));
            EXPECT_CALL(_runnableMock1, execute());
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(0U));
            cut.dispatchWhileWork();
        }
    }
}

/**
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi
 * \desc: To test task schedule at fixedrate functionality
 */
TEST_F(TaskContextTest, testScheduleAtFixedRate)
{
    TaskContext<TestBindingMock> cut;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(Return(&_taskHandle));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());

    void* timerId                            = 0L;
    TimerCallbackFunction_t callbackFunction = 0L;
    EXPECT_CALL(_freeRtosMock, xTimerCreateStatic(_name, 1U, 0U, _, NotNull(), &_timer))
        .WillOnce(
            DoAll(SaveArg<3>(&timerId), SaveArg<4>(&callbackFunction), Return(&_timerHandle)));
    cut.createTimer(_timer, _name);
    // now trigger timer the first time
    EXPECT_CALL(_freeRtosMock, pvTimerGetTimerID(&_timerHandle)).WillRepeatedly(Return(timerId));

    {
        // expect task notify
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        callbackFunction(&_timerHandle);

        // set timer
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

        // dispatch and expect system time. Nothing should happen
        Sequence seq;
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(eventMask));
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(SetArgPointee<2>(0U));
        cut.dispatchWhileWork();
    }

    {
        // Schedule first timer at fixed rate. Expect task notify
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.scheduleAtFixedRate(_runnableMock1, _timeout1, 10U, TimeUnit::MILLISECONDS);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);

        {
            EXPECT_CALL(_freeRtosMock, pvTimerGetTimerID(&_timerHandle))
                .WillRepeatedly(Return(timerId));
            // expect nothing if scheduled again
            cut.scheduleAtFixedRate(_runnableMock1, _timeout1, 10U, TimeUnit::MILLISECONDS);
            Mock::VerifyAndClearExpectations(&_freeRtosMock);
        }

        {
            EXPECT_CALL(_freeRtosMock, pvTimerGetTimerID(&_timerHandle))
                .WillRepeatedly(Return(timerId));
            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(eventMask));
            EXPECT_CALL(_freeRtosMock, xTimerChangePeriod(&_timerHandle, 100U, 0U));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(0U));
            cut.dispatchWhileWork();
            Mock::VerifyAndClearExpectations(&_freeRtosMock);
        }

        {
            EXPECT_CALL(_freeRtosMock, pvTimerGetTimerID(&_timerHandle))
                .WillRepeatedly(Return(timerId));
            // timer expires at correct time
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .WillOnce(SaveArg<1>(&eventMask));
            callbackFunction(&_timerHandle);

            // Expect expired
            EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(110000U));

            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(eventMask));
            EXPECT_CALL(_runnableMock1, execute());
            EXPECT_CALL(_freeRtosMock, xTimerChangePeriod(&_timerHandle, 100U, 0U));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
                .InSequence(seq)
                .WillOnce(SetArgPointee<2>(0U));
            cut.dispatchWhileWork();
        }
    }
}

/**
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi
 * \desc: To test task schedule at fixedrate functionality with multiple timers
 */
TEST_F(TaskContextTest, testScheduleAtFixedRateMultipleTimers)
{
    TaskContext<TestBindingMock> cut;
    void* timerId = 0L;

    // set timer
    EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

    EXPECT_CALL(_freeRtosMock, pvTimerGetTimerID(&_timerHandle)).WillRepeatedly(Return(timerId));
    EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
        .WillOnce(Return(static_cast<BaseType_t*>(0L)));
    EXPECT_CALL(_freeRtosMock, xTaskNotify(_, _, eSetBits)).Times(1);
    cut.scheduleAtFixedRate(_runnableMock1, _timeout1, 10U, TimeUnit::MILLISECONDS);
    Mock::VerifyAndClearExpectations(&_freeRtosMock);

    // another task that expires later
    cut.scheduleAtFixedRate(_runnableMock2, _timeout2, 20U, TimeUnit::MILLISECONDS);
    Mock::VerifyAndClearExpectations(&_freeRtosMock);
}

/**
 * \refs: SMD_asyncFreeRtos_TaskContextInformational
 * \desc: To test getUnusedStackSize functionality
 */
TEST_F(TaskContextTest, testGetUnusedStackSize)
{
    {
        EXPECT_CALL(_freeRtosMock, uxTaskGetStackHighWaterMark(&_taskHandle))
            .WillOnce(Return(244U));
        EXPECT_EQ(
            244U * sizeof(StackType_t),
            TaskContext<TestBindingMock>::getUnusedStackSize(&_taskHandle));
    }
    {
        TaskContext<TestBindingMock> cut;
        EXPECT_CALL(
            _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
            .WillOnce(Return(&_taskHandle));
        cut.createTask(
            1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());
        EXPECT_CALL(_freeRtosMock, uxTaskGetStackHighWaterMark(NotNull())).WillOnce(Return(124U));
        EXPECT_EQ(124U * sizeof(StackType_t), cut.getUnusedStackSize());
    }
}

} // namespace
