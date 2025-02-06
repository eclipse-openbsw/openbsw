// Copyright 2025 Accenture.

#include "safeMonitor/Trigger.h"

#include "common.h"

#include <gtest/gtest.h>

struct TriggerTest : ::testing::Test
{
    using MyError = ::safeMonitor::Trigger<HandlerMock, MyEvent, ScopedMutexMock, MyContext>;

    static unsigned int const REPETITIONS = 5;

    HandlerMock _handler;
    MyError _error{_handler, SOMETHING_HAPPENED};
};

/**
 * \desc:
 * Triggers a monitor and check that  handler is called with correct event.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(TriggerTest, TriggersHandler)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(REPETITIONS);
    for (unsigned int i = 0; i < REPETITIONS; ++i)
    {
        _error.trigger();
    }
}

TEST_F(TriggerTest, UsesScopedMutex)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    ScopedMutexMock::reset();
    _error.trigger();
    EXPECT_EQ(1, ScopedMutexMock::numConstructed());
    EXPECT_TRUE(ScopedMutexMock::allDestructed());
}

TEST_F(TriggerTest, UsesDefaultConstructedContext)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    _error.trigger();
    EXPECT_EQ(0xDEADBEEF, _error.getContext().value);
}

TEST_F(TriggerTest, StoresAndReturnsGivenContext)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    MyContext c;
    c.value = 0xDEADC0DE;
    _error.trigger(c);
    EXPECT_EQ(c.value, _error.getContext().value);
}

TEST_F(TriggerTest, ContextIsBeingCopied)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    MyContext const c1;
    _error.trigger(c1);
    MyContext const& c2 = _error.getContext();
    EXPECT_NE(&c1, &c2);
}
