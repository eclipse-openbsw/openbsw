// Copyright 2024 Accenture.

#include "async/EventDispatcher.h"

#include <etl/delegate.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class EventDispatcherTest : public Test
{
public:
    using HandlerFunctionType = ::etl::delegate<void()>;

    MOCK_METHOD0(handleEvent1, void());
    MOCK_METHOD0(handleEvent2, void());
    MOCK_METHOD0(handleEvent3, void());
};

struct TestLock
{
    TestLock() {}

    ~TestLock() {}
};

TEST_F(EventDispatcherTest, testAll)
{
    EventDispatcher<3, TestLock> cut;
    {
        // set two event handlers
        cut.setEventHandler(
            0U,
            HandlerFunctionType::create<EventDispatcherTest, &EventDispatcherTest::handleEvent1>(
                *this));
        cut.setEventHandler(
            2U,
            HandlerFunctionType::create<EventDispatcherTest, &EventDispatcherTest::handleEvent3>(
                *this));
        // handle all events (in correct order)
        Sequence seq;
        EXPECT_CALL(*this, handleEvent1()).InSequence(seq);
        EXPECT_CALL(*this, handleEvent3()).InSequence(seq);
        cut.handleEvents(7U);
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // add third event handler and handle all events
        cut.setEventHandler(
            1U,
            HandlerFunctionType::create<EventDispatcherTest, &EventDispatcherTest::handleEvent2>(
                *this));
        Sequence seq;
        EXPECT_CALL(*this, handleEvent1()).InSequence(seq);
        EXPECT_CALL(*this, handleEvent2()).InSequence(seq);
        EXPECT_CALL(*this, handleEvent3()).InSequence(seq);
        cut.handleEvents(7U);
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // set single event and expect only single handler to be called
        EXPECT_CALL(*this, handleEvent3());
        cut.handleEvents(4U);
        Mock::VerifyAndClearExpectations(this);
    }
    {
        // remove handler and expect it to no more be called
        // set two event handlers
        cut.removeEventHandler(2U);
        // handle all events (in correct order)
        Sequence seq;
        EXPECT_CALL(*this, handleEvent1()).InSequence(seq);
        EXPECT_CALL(*this, handleEvent2()).InSequence(seq);
        cut.handleEvents(7U);
        Mock::VerifyAndClearExpectations(this);
    }
}

} // namespace
