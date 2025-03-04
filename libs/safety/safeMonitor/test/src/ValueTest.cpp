// Copyright 2025 Accenture.

#include "safeMonitor/Value.h"

#include "common.h"

#include <gtest/gtest.h>

#include <cstdint>

using MyValue = ::safeMonitor::Value<HandlerMock, MyEvent, uint8_t, ScopedMutexMock, MyContext>;

namespace
{
static constexpr MyValue::Type EXPECTED_VALUE = 127U;
static constexpr unsigned int REPETITIONS     = 5;
} // namespace

struct ValueTest : ::testing::Test
{
    HandlerMock _handler;
    MyValue _value{_handler, SOMETHING_HAPPENED, EXPECTED_VALUE};
};

/**
 * \desc:
 * Checks that the handler is called if the current value is higher than the expected value.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(ValueTest, DetectsTooHighAValue)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(REPETITIONS);
    for (unsigned int i = 0; i < REPETITIONS; ++i)
    {
        _value.check(EXPECTED_VALUE + 1U);
    }
}

/**
 * \desc:
 * Checks that the handler is called if the current value is lower than the expected value.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(ValueTest, DetectsTooLowAValue)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(REPETITIONS);
    for (unsigned int i = 0; i < REPETITIONS; ++i)
    {
        _value.check(EXPECTED_VALUE - 1U);
    }
}

/**
 * \desc:
 * Checks that the handler is NOT called if the current value is equal to the expected value.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(ValueTest, PassesWithExpectedValue)
{
    EXPECT_CALL(_handler, handle(::testing::_)).Times(0U);
    for (unsigned int i = 0; i < REPETITIONS; ++i)
    {
        _value.check(EXPECTED_VALUE);
    }
}

TEST_F(ValueTest, UsesScopedMutex)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    ScopedMutexMock::reset();
    _value.check(EXPECTED_VALUE);
    _value.check(EXPECTED_VALUE - 1U);
    EXPECT_EQ(2, ScopedMutexMock::numConstructed());
    EXPECT_TRUE(ScopedMutexMock::allDestructed());
}

TEST_F(ValueTest, UsesDefaultConstructedContext)
{
    _value.check(EXPECTED_VALUE);
    EXPECT_EQ(0xDEADBEEF, _value.getContext().value);
}

TEST_F(ValueTest, StoresAndReturnsGivenContext)
{
    MyContext c;
    c.value = 0xDEADC0DE;
    _value.check(EXPECTED_VALUE, c);
    EXPECT_EQ(c.value, _value.getContext().value);
}

TEST_F(ValueTest, ContextIsBeingCopied)
{
    MyContext const c1;
    _value.check(EXPECTED_VALUE, c1);
    MyContext const& c2 = _value.getContext();
    EXPECT_NE(&c1, &c2);
}
