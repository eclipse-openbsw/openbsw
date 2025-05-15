// Copyright 2024 Accenture.

#include "async/EventPolicy.h"

#include <etl/delegate.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class EventPolicyTest : public Test
{
public:
    using HandlerFunctionType = ::etl::delegate<void()>;

    MOCK_METHOD(void, setEventHandler, (size_t event, HandlerFunctionType handlerFunction));
    MOCK_METHOD(void, removeEventHandler, (size_t event));
    MOCK_METHOD(void, setEvents, (EventMaskType events));

    MOCK_METHOD(void, handleEvent, ());

protected:
};

TEST_F(EventPolicyTest, testAll)
{
    EventPolicy<EventPolicyTest, 1> cut(*this);
    {
        // expect set event handler to be propagated
        EXPECT_CALL(
            *this,
            setEventHandler(
                1U,
                HandlerFunctionType::create<EventPolicyTest, &EventPolicyTest::handleEvent>(
                    *this)));
        cut.setEventHandler(
            HandlerFunctionType::create<EventPolicyTest, &EventPolicyTest::handleEvent>(*this));
    }
    {
        // expect remove event handler to be propagated
        EXPECT_CALL(*this, removeEventHandler(1U));
        cut.removeEventHandler();
    }
    {
        // expect set event to call setEvents on dispatcher
        EXPECT_CALL(*this, setEvents(1 << 1U));
        cut.setEvent();
    }
}

} // namespace
