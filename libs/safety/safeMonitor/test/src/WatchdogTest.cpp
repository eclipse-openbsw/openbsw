// Copyright 2025 Accenture.

#include "safeMonitor/Watchdog.h"

#include "common.h"

#include <gtest/gtest.h>

struct WatchdogTest : ::testing::Test
{
    WatchdogTest() : _handler(), _snoopy(_handler, SOMETHING_HAPPENED, TIMEOUT) {}

    using MyWatchdog = ::safeMonitor::Watchdog<HandlerMock, MyEvent, ScopedMutexMock, MyContext>;

    static MyWatchdog::Timeout const TIMEOUT = 10;
    static unsigned int const REPETITIONS    = 5;

    HandlerMock _handler;
    MyWatchdog _snoopy;
};

/**
 * \desc:
 * Decrements the timeout counter until it reaches zero and checks that the handler is called.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(WatchdogTest, DetectsTimeout)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    for (MyWatchdog::Timeout i = 0U; i < WatchdogTest::TIMEOUT; ++i)
    {
        _snoopy.service(); // last iteration should fire
    }
}

TEST_F(WatchdogTest, Triggers)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(5U);
    for (MyWatchdog::Timeout i = 0U; i < WatchdogTest::TIMEOUT + 4U; ++i)
    {
        _snoopy.service(); // last five iterations produce timeout
    }
}

/**
 * \desc:
 * Decrements the timeout counter but before reaching zero the test resets
 * the counter again. Checks thats the handler is not called.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(WatchdogTest, KickResetsTimeout)
{
    EXPECT_CALL(_handler, handle(::testing::_)).Times(0U);

    // call service until internal counter is right before timeout
    for (MyWatchdog::Timeout i = 0U; i < WatchdogTest::TIMEOUT - 1U; ++i)
    {
        _snoopy.service();
    }

    _snoopy.kick();    // resets timeout
    _snoopy.service(); // should not trigger the handler
}

/**
 * \desc:
 * Resets the timeout counter multiple times. Afterward decrements the timeout counter
 * until is reaches zero and checks that the handler is called.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(WatchdogTest, KickResetsTimeoutOnlyOnce)
{
    EXPECT_CALL(_handler, handle(::testing::_)).Times(1U);

    _snoopy.kick();
    _snoopy.kick();

    for (MyWatchdog::Timeout i = 0U; i < WatchdogTest::TIMEOUT; ++i)
    {
        _snoopy.service(); // last iteration should trigger the handler
    }
}

TEST_F(WatchdogTest, ServiceUsesScopedMutex)
{
    ScopedMutexMock::reset();
    for (unsigned int i = 0; i < REPETITIONS; ++i)
    {
        _snoopy.service();
        EXPECT_EQ(static_cast<int>(i + 1), ScopedMutexMock::numConstructed());
        EXPECT_TRUE(ScopedMutexMock::allDestructed());
    }
}

TEST_F(WatchdogTest, KickUsesScopedMutex)
{
    ScopedMutexMock::reset();
    for (unsigned int i = 0; i < REPETITIONS; ++i)
    {
        _snoopy.kick();
        EXPECT_EQ(static_cast<int>(i + 1), ScopedMutexMock::numConstructed());
        EXPECT_TRUE(ScopedMutexMock::allDestructed());
    }
}

TEST_F(WatchdogTest, UsesDefaultConstructedContext)
{
    _snoopy.kick();
    EXPECT_EQ(0xDEADBEEF, _snoopy.getContext().value);
}

TEST_F(WatchdogTest, StoresAndReturnsGivenContext)
{
    MyContext c;
    c.value = 0xDEADC0DE;
    _snoopy.kick(c);
    EXPECT_EQ(c.value, _snoopy.getContext().value);
}

TEST_F(WatchdogTest, ContextIsBeingCopied)
{
    MyContext const c1;
    _snoopy.kick(c1);
    MyContext const& c2 = _snoopy.getContext();
    EXPECT_NE(&c1, &c2);
}
