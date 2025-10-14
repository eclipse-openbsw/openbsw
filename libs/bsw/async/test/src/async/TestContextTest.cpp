// Copyright 2024 Accenture.

#include "async/TestContext.h"

#include "async/AsyncMock.h"
#include "async/RunnableMock.h"
#include "async/TimeoutMock.h"

namespace
{
using namespace ::async;
using namespace ::testing;

class TestContextTest : public Test
{
protected:
    StrictMock<AsyncMock> _asyncMock;
    StrictMock<RunnableMock> _runnableMock1;
    StrictMock<RunnableMock> _runnableMock2;
    TimeoutType _timeout1;
    TimeoutType _timeout2;
    StrictMock<TimeoutMock> _timeoutMock;
};

TEST_F(TestContextTest, testConstructor)
{
    ContextType context = 45U;
    TestContext cut(context);
    EXPECT_EQ(context, cut.getContext());
    EXPECT_EQ(context, cut);
}

TEST_F(TestContextTest, testHandleRunnables)
{
    ContextType context = 25U;
    TestContext cut(context);
    {
        // don't expect runnables to be handled directly
        EXPECT_CALL(_asyncMock, execute(context, Ref(_runnableMock1)));
        execute(context, _runnableMock1);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        cut.execute();
    }
    cut.handleExecute();
    {
        // don't expect any timeout to be handled
        EXPECT_CALL(
            _asyncMock,
            schedule(context, Ref(_runnableMock1), Ref(_timeout1), 30U, TimeUnit::MICROSECONDS));
        schedule(context, _runnableMock1, _timeout1, 30U, TimeUnit::MICROSECONDS);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        cut.expire();
    }
    cut.handleExecute();
    {
        // now expect runnables to be handled by context
        execute(context, _runnableMock1);
        execute(context, _runnableMock2);
        Sequence seq;
        EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
        EXPECT_CALL(_runnableMock2, execute()).InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&_asyncMock);
    }
}

TEST_F(TestContextTest, testHandleTimeouts)
{
    ContextType context = 17U;
    TestContext cut(context);
    {
        // don't expect timeouts to be handled directly
        EXPECT_CALL(
            _asyncMock,
            schedule(context, Ref(_runnableMock1), Ref(_timeout1), 25U, TimeUnit::MICROSECONDS));
        schedule(context, _runnableMock1, _timeout1, 25U, TimeUnit::MICROSECONDS);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        cut.setNow(30U);
        cut.expire();
        // and also not timeouts
        EXPECT_CALL(_timeoutMock, cancel(Ref(_timeout1)));
        _timeout1.cancel();
    }
    cut.handleSchedule();
    {
        // don't expect runnables to be handled directly
        EXPECT_CALL(_asyncMock, execute(context, Ref(_runnableMock1)));
        execute(context, _runnableMock1);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        cut.execute();
    }
    cut.handleSchedule();
    {
        cut.setNow(0U);
        // now expect schedules to be handled by context
        schedule(context, _runnableMock1, _timeout1, 25U, TimeUnit::MICROSECONDS);
        scheduleAtFixedRate(context, _runnableMock2, _timeout2, 25U, TimeUnit::MICROSECONDS);
        // don't expire too early
        cut.setNow(24U);
        cut.expire();
        Mock::VerifyAndClearExpectations(&_asyncMock);
        // expire on exact point in time
        cut.setNow(25U);
        Sequence seq;
        EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
        EXPECT_CALL(_runnableMock2, execute()).InSequence(seq);
        cut.expire();
        Mock::VerifyAndClearExpectations(&_asyncMock);

        // not expired before cyclic call
        cut.setNow(24U);
        cut.expire();
        Mock::VerifyAndClearExpectations(&_asyncMock);

        // expire on cyclic call
        cut.setNow(50U);
        EXPECT_CALL(_runnableMock2, execute());
        cut.expire();
        Mock::VerifyAndClearExpectations(&_asyncMock);

        // cancel
        _timeout2.cancel();
        cut.setNow(75U);
        cut.expire();
        Mock::VerifyAndClearExpectations(&_asyncMock);
    }
}

TEST_F(TestContextTest, testHandleAll)
{
    ContextType context = 17U;
    TestContext cut(context);
    cut.handleAll();
    {
        // now expect runnables to be handled by context
        execute(context, _runnableMock1);
        EXPECT_CALL(_runnableMock1, execute());
        cut.execute();
        Mock::VerifyAndClearExpectations(&_asyncMock);
    }
    cut.handleAll();
    {
        // now expect schedules to be handled by context
        schedule(context, _runnableMock1, _timeout1, 25U, TimeUnit::MICROSECONDS);
        // expire on exact point in time
        cut.setNow(25U);
        EXPECT_CALL(_runnableMock1, execute());
        cut.expire();
        Mock::VerifyAndClearExpectations(&_asyncMock);
    }
    cut.handleAll();
    {
        // leave timeout within context
        schedule(context, _runnableMock1, _timeout1, 50U, TimeUnit::MILLISECONDS);
    }
}

TEST_F(TestContextTest, testScheduleTimestampsOrdering)
{
    ContextType context = 12U;
    TestContext cut(context);
    cut.handleSchedule();
    cut.setNow(0U);
    schedule(context, _runnableMock1, _timeout1, 10U, TimeUnit::MICROSECONDS);
    schedule(context, _runnableMock2, _timeout2, 25U, TimeUnit::MICROSECONDS);
    // expect first runnable to expire at exact time
    cut.setNow(10U);
    EXPECT_CALL(_runnableMock1, execute());
    cut.expire();
}

} // namespace
