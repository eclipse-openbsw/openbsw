// Copyright 2025 Accenture.

#include "safeMonitor/Sequence.h"

#include "common.h"

#include <gtest/gtest.h>

struct SequenceTest : ::testing::Test
{
    enum MyCheckpoints
    {
        ALPHA,
        BETA,
        GAMMA,
        DELTA
    };

    using MySequence
        = ::safeMonitor::Sequence<HandlerMock, MyEvent, MyCheckpoints, ScopedMutexMock, MyContext>;

    HandlerMock _handler;
    MySequence _sequence{_handler, SOMETHING_HAPPENED, ALPHA, DELTA};
};

/**
 * \desc:
 * Checks that the handler is NOT called if all checkpoints are reached in the correct order.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(SequenceTest, RunsThroughInCorrectOrder)
{
    EXPECT_CALL(_handler, handle(::testing::_)).Times(0U);
    _sequence.hit(ALPHA);
    _sequence.hit(BETA);
    _sequence.hit(GAMMA);
    _sequence.hit(DELTA);
}

/**
 * \desc:
 * Checks that the handler is NOT called if all checkpoints are reached in the correct order
 * and correctly started from the beginning again.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(SequenceTest, WrapsBackToFirstCheckpoint)
{
    EXPECT_CALL(_handler, handle(::testing::_)).Times(0U);
    _sequence.hit(ALPHA);
    _sequence.hit(BETA);
    _sequence.hit(GAMMA);
    _sequence.hit(DELTA);
    _sequence.hit(ALPHA); // wrap
    _sequence.hit(BETA);
}

/**
 * \desc:
 * Checks that the handler is called if the first checkpoint is wrong.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(SequenceTest, DetectsWrongCheckpointAtTheBeginning)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    _sequence.hit(DELTA); // fire
}

/**
 * \desc:
 * Checks that the handler is called if a checkpoint in the middle of the sequence is wrong.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(SequenceTest, DetectsWrongCheckpointInTheMiddle)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    _sequence.hit(ALPHA);
    _sequence.hit(BETA);
    _sequence.hit(DELTA); // fire
}

/**
 * \desc:
 * Checks that the handler is called if the checkpoint at the end of the sequence is wrong.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(SequenceTest, DetectsWrongCheckpointAtTheEnd)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1U);
    _sequence.hit(ALPHA);
    _sequence.hit(BETA);
    _sequence.hit(GAMMA);
    _sequence.hit(BETA); // fire
}

TEST_F(SequenceTest, FiresMultipleTimes)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(3U);
    _sequence.hit(DELTA); // fire
    _sequence.hit(GAMMA); // fire
    _sequence.hit(BETA);  // fire
    _sequence.hit(ALPHA); // correct -> don't fire
}

TEST_F(SequenceTest, UsesScopedMutex)
{
    ScopedMutexMock::reset();
    _sequence.hit(ALPHA);
    _sequence.hit(BETA);
    _sequence.hit(GAMMA);
    _sequence.hit(DELTA);
    EXPECT_EQ(4, ScopedMutexMock::numConstructed());
    EXPECT_TRUE(ScopedMutexMock::allDestructed());
}

TEST_F(SequenceTest, UsesDefaultConstructedContext)
{
    _sequence.hit(ALPHA);
    EXPECT_EQ(0xDEADBEEF, _sequence.getContext().value);
}

TEST_F(SequenceTest, StoresAndReturnsGivenContext)
{
    MyContext c;
    c.value = 0xDEADC0DE;
    _sequence.hit(ALPHA, c);
    EXPECT_EQ(c.value, _sequence.getContext().value);
}

TEST_F(SequenceTest, ContextIsBeingCopied)
{
    MyContext const c1;
    _sequence.hit(ALPHA, c1);
    MyContext const& c2 = _sequence.getContext();
    EXPECT_NE(&c1, &c2);
}
