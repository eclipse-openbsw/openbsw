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
    TaskContextTest() : _name("test"), _taskHandle(10U) {}

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
};

/**
 * \req: [BSW_ASFR_39], [BSW_ASFR_52], [BSW_ASFR_54], [BSW_ASFR_57]
 * \refs: SMD_asyncFreeRtos_TaskContext, SMD_asyncFreeRtos_TaskContextEventHandling,
 * SMD_asyncFreeRtos_TaskContextInitialization
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
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(0U), Return(false)));
        // second time do nothing again
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
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
            .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
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
 * \req: [BSW_ASFR_51], [BSW_ASFR_52], [BSW_ASFR_53]
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi
 * \desc: To test task execution
 */
TEST_F(TaskContextTest, testExecute)
{
    TaskContext<TestBindingMock> cut;

    EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

    TaskFunction_t* osTaskFunction = 0L;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), Return(&_taskHandle)));
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
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(eventMask), Return(true)));
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
        EXPECT_CALL(_runnableMock1, execute());
        // trigger shutdown on next iteration
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .InSequence(seq)
            .WillOnce(SaveArg<1>(&eventMask));
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
        osTaskFunction(&cut);
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
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(eventMask), Return(true)));
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
        EXPECT_CALL(_runnableMock1, execute());
        // trigger shutdown on next iteration
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .InSequence(seq)
            .WillOnce(SaveArg<1>(&eventMask));
        EXPECT_CALL(
            _freeRtosMock,
            xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
            .InSequence(seq)
            .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
        osTaskFunction(&cut);
        Mock::VerifyAndClearExpectations(&_runnableMock1);
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
    TaskFunction_t* osTaskFunction = 0L;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), Return(&_taskHandle)));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());

    {
        // set timer
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

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
        // Cancel the second timer, nothing should happen.
        cut.cancel(_timeout2);

        {
            // an event with an active timer will lead to a different timeout
            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 1500U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(eventMask), Return(true)));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 1500U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
            // trigger shutdown on next iteration
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .InSequence(seq)
                .WillOnce(SaveArg<1>(&eventMask));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 1500U))
                .InSequence(seq)
                .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
            osTaskFunction(&cut);
            Mock::VerifyAndClearExpectations(&_systemTimerMock);
        }
        {
            // some time has elapsed and OS timer expires a bit too early
            // which leads to a new setup of the timer.
            // The timer then elapses after second iteration
            EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit())
                .WillOnce(Return(249000U))
                .WillRepeatedly(Return(250000U));

            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 10U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0), Return(false)));
            EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
            EXPECT_CALL(
                _freeRtosMock,
                xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
            // trigger shutdown on next iteration
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .InSequence(seq)
                .WillOnce(SaveArg<1>(&eventMask));
            EXPECT_CALL(
                _freeRtosMock,
                xTaskNotifyWait(0U, 7U, NotNull(), TestBindingMock::WAIT_EVENTS_TICK_COUNT))
                .InSequence(seq)
                .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
            osTaskFunction(&cut);
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
    TaskFunction_t* osTaskFunction = 0L;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), Return(&_taskHandle)));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());

    {
        // set timer
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

        // Schedule first timer at fixed rate. Expect task notify
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.scheduleAtFixedRate(_runnableMock1, _timeout1, 20U, TimeUnit::MILLISECONDS);

        // Schedule second timer (to elapse laster). No task notify expected
        cut.scheduleAtFixedRate(_runnableMock2, _timeout2, 151U, TimeUnit::MILLISECONDS);
        // Cancel the second timer, nothing should happen.
        cut.cancel(_timeout2);

        Mock::VerifyAndClearExpectations(&_freeRtosMock);

        {
            // expect nothing if scheduled again
            cut.scheduleAtFixedRate(_runnableMock1, _timeout1, 10U, TimeUnit::MILLISECONDS);
            Mock::VerifyAndClearExpectations(&_freeRtosMock);
        }

        {
            // the event is received and nothing should happen while no time is elapsed
            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 200U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(eventMask), Return(true)));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 200U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
            // trigger shutdown on next iteration
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .InSequence(seq)
                .WillOnce(SaveArg<1>(&eventMask));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 200U))
                .InSequence(seq)
                .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
            osTaskFunction(&cut);
            Mock::VerifyAndClearExpectations(&_freeRtosMock);
        }
        {
            // time has elapsed and the timer should expire and lead to the next iteration
            // with the same timeout
            EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit())
                .WillOnce(Return(105000U))
                .WillOnce(Return(120000U))
                .WillOnce(Return(120000U))
                .WillOnce(Return(120000U))
                .WillRepeatedly(Return(140000U));

            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 150U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0), Return(false)));
            EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 200U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0), Return(false)));
            EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 200U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(false)));
            // trigger shutdown on next iteration
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .InSequence(seq)
                .WillOnce(SaveArg<1>(&eventMask));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 200U))
                .InSequence(seq)
                .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
            osTaskFunction(&cut);
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
    TaskFunction_t* osTaskFunction = 0L;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), Return(&_taskHandle)));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());

    {
        // set timer
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillRepeatedly(Return(100000U));

        // Schedule first timer at fixed rate. Expect task notify
        uint32_t eventMask = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.scheduleAtFixedRate(_runnableMock1, _timeout1, 20U, TimeUnit::MILLISECONDS);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
        // Schedule second timer at fixed rate. Expect task notify again because it changes the next
        // timeout
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask));
        cut.scheduleAtFixedRate(_runnableMock2, _timeout2, 12U, TimeUnit::MILLISECONDS);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);

        {
            // the second timer should lead to the right expected timeout
            Sequence seq;
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 120U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(eventMask), Return(true)));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 120U))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgPointee<2>(0U), StopDispatch(&cut), Return(true)));
            // trigger shutdown on next iteration
            EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
                .WillOnce(Return(static_cast<BaseType_t*>(0L)));
            EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
                .InSequence(seq)
                .WillOnce(SaveArg<1>(&eventMask));
            EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 120U))
                .InSequence(seq)
                .WillOnce(DoAll(CopyArgPointee2(&eventMask), Return(true)));
            osTaskFunction(&cut);
            Mock::VerifyAndClearExpectations(&_freeRtosMock);
        }
    }
}

/**
 * \req: [BSW_ASFR_51], [BSW_ASFR_52], [BSW_ASFR_53]
 * \refs: SMD_asyncFreeRtos_TaskContextAsyncApi
 * \desc: To test dispatchWhileWork function to finish on work done
 */
TEST_F(TaskContextTest, testDispatchWhileWork)
{
    TaskContext<TestBindingMock> cut;
    TaskFunction_t* osTaskFunction = 0L;
    EXPECT_CALL(
        _freeRtosMock, xTaskCreateStatic(NotNull(), _name, 100, NotNull(), 12U, _stack, &_task))
        .WillOnce(DoAll(SaveArg<0>(&osTaskFunction), Return(&_taskHandle)));
    cut.createTask(1U, _task, _name, 12U, _stack, TaskContext<TestBindingMock>::TaskFunctionType());

    {
        // set timer
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit()).WillOnce(Return(100000U));

        // Schedule timer at fixed rate. Expect task notify
        uint32_t eventMask1 = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask1));
        cut.schedule(_runnableMock1, _timeout1, 20U, TimeUnit::MILLISECONDS);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);

        // Execute a task
        uint32_t eventMask2 = 0U;
        EXPECT_CALL(_bindingMock, getHigherPriorityTaskWokenFunc())
            .WillOnce(Return(static_cast<BaseType_t*>(0L)));
        EXPECT_CALL(_freeRtosMock, xTaskNotify(&_taskHandle, _, eSetBits))
            .WillOnce(SaveArg<1>(&eventMask2));
        cut.execute(_runnableMock2);
        Mock::VerifyAndClearExpectations(&_bindingMock);
        Mock::VerifyAndClearExpectations(&_freeRtosMock);

        // time is running during dispatch
        EXPECT_CALL(_systemTimerMock, getSystemTimeUs32Bit())
            .WillOnce(Return(110000U))
            .WillOnce(Return(120000U))
            .WillOnce(Return(120010U))
            .WillOnce(Return(120020U));
        Sequence seq;
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgPointee<2>((eventMask1 | eventMask2)), Return(true)));
        EXPECT_CALL(_runnableMock2, execute()).InSequence(seq);
        EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
        EXPECT_CALL(_freeRtosMock, xTaskNotifyWait(0U, 7U, NotNull(), 0U))
            .InSequence(seq)
            .WillOnce(Return(false));
        cut.dispatchWhileWork();
    }
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
