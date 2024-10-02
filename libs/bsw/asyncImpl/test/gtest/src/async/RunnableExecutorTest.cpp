// Copyright 2024 Accenture.

#include "async/RunnableExecutor.h"

#include "async/EventPolicy.h"
#include "async/QueueNode.h"
#include "async/RunnableMock.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class RunnableExecutorTest : public Test
{
public:
    using HandlerFunctionType = ::estd::function<void()>;

    MOCK_METHOD2(setEventHandler, void(size_t event, HandlerFunctionType handlerFunction));
    MOCK_METHOD1(removeEventHandler, void(size_t event));
    MOCK_METHOD1(setEvents, void(EventMaskType events));

protected:
    StrictMock<RunnableMock> _runnableMock1;
    StrictMock<RunnableMock> _runnableMock2;
    StrictMock<RunnableMock> _runnableMock3;
};

struct TestLock
{
    TestLock() {}

    ~TestLock() {}
};

TEST_F(RunnableExecutorTest, testAll)
{
    RunnableExecutor<IRunnable, EventPolicy<RunnableExecutorTest, 2>, TestLock> cut(*this);
    HandlerFunctionType eventHandler;
    {
        // expect event handler to be set on init
        EXPECT_CALL(*this, setEventHandler(2U, _)).WillOnce(SaveArg<1>(&eventHandler));
        cut.init();
        EXPECT_TRUE(eventHandler.has_value());
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // expect event to be set on each added runnable
        EXPECT_CALL(*this, setEvents(1U << 2U));
        cut.enqueue(_runnableMock1);
        Mock::VerifyAndClearExpectations(this);
        EXPECT_CALL(*this, setEvents(1U << 2U));
        cut.enqueue(_runnableMock2);
        Mock::VerifyAndClearExpectations(this);
        EXPECT_CALL(*this, setEvents(1U << 2U));
        cut.enqueue(_runnableMock3);
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // expect event to be set if runnable is enqueued again
        EXPECT_CALL(*this, setEvents(1U << 2U));
        cut.enqueue(_runnableMock1);
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // expect all runnables to be executed on handle event
        Sequence seq;
        EXPECT_CALL(_runnableMock1, execute()).InSequence(seq);
        EXPECT_CALL(_runnableMock2, execute()).InSequence(seq);
        EXPECT_CALL(_runnableMock3, execute()).InSequence(seq);
        eventHandler();
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // expect nothing to be executed on handle event
        eventHandler();
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // expect event handler to be removed on shutdown
        EXPECT_CALL(*this, removeEventHandler(2U));
        cut.shutdown();
        Mock::VerifyAndClearExpectations(this);
    }
}

} // namespace
