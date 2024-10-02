// Copyright 2024 Accenture.

#include "util/timeout/AbstractTimeout.h"

#include "async/AsyncMock.h"
#include "async/TimeoutMock.h"

namespace
{
using namespace common;
using namespace testing;

ACTION(Reset) { AbstractTimeout::TimeoutExpiredActions(arg0).reset(); }

ACTION_P(Reset, newTimeout) { AbstractTimeout::TimeoutExpiredActions(arg0).reset(newTimeout); }

ACTION(Cancel) { AbstractTimeout::TimeoutExpiredActions(arg0).cancel(); }

class AbstractTimeoutMock : public ::common::AbstractTimeout
{
public:
    MOCK_METHOD1(expired, void(TimeoutExpiredActions));
};

class AbstractTimeoutTest : public Test
{
public:
    StrictMock<::async::AsyncMock> _asyncMock;
    StrictMock<::async::TimeoutMock> _timeoutMock;
};

TEST_F(AbstractTimeoutTest, testAll)
{
    StrictMock<AbstractTimeoutMock> cut;
    {
        EXPECT_EQ(cut._context, ::async::CONTEXT_INVALID);
        EXPECT_EQ(cut._time, 0U);
        EXPECT_FALSE(cut._isCyclic);
        EXPECT_FALSE(cut._isActive);
        EXPECT_FALSE(cut.isActive());
    }
    ::async::ContextType context(3U);
    {
        // nothing should happen if executed accidentally
        cut.execute();
    }
    cut._context  = context;
    cut._time     = 2398U;
    cut._isActive = true;
    {
        // execute single-shot timeout
        EXPECT_CALL(cut, expired(_));
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_FALSE(cut._isActive);
    }
    {
        cut._isActive = true;
        // reset single-shot timeout
        Sequence seq;
        EXPECT_CALL(cut, expired(_)).WillOnce(Reset());
        EXPECT_CALL(_timeoutMock, cancel(Ref(cut._asyncTimeout))).InSequence(seq);
        EXPECT_CALL(
            _asyncMock,
            schedule(
                context, Ref(cut), Ref(cut._asyncTimeout), 2398U, ::async::TimeUnit::MILLISECONDS))
            .InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_TRUE(cut._isActive);
    }
    {
        // reset single-shot timeout with new value
        Sequence seq;
        EXPECT_CALL(cut, expired(_)).WillOnce(Reset(3384U));
        EXPECT_CALL(_timeoutMock, cancel(Ref(cut._asyncTimeout))).InSequence(seq);
        EXPECT_CALL(
            _asyncMock,
            schedule(
                context, Ref(cut), Ref(cut._asyncTimeout), 3384U, ::async::TimeUnit::MILLISECONDS))
            .InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_TRUE(cut._isActive);
    }
    {
        // cancel single-shot timeout
        Sequence seq;
        EXPECT_CALL(cut, expired(_)).WillOnce(Cancel());
        EXPECT_CALL(_timeoutMock, cancel(Ref(cut._asyncTimeout))).InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_FALSE(cut._isActive);
    }

    cut._isCyclic = true;
    {
        // execute cyclic timeout
        cut._isActive = true;
        EXPECT_CALL(cut, expired(_));
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_TRUE(cut._isActive);
    }
    {
        // reset cyclic timeout
        cut._time = 2314U;
        Sequence seq;
        EXPECT_CALL(cut, expired(_)).WillOnce(Reset());
        EXPECT_CALL(_timeoutMock, cancel(Ref(cut._asyncTimeout))).InSequence(seq);
        EXPECT_CALL(
            _asyncMock,
            scheduleAtFixedRate(
                context, Ref(cut), Ref(cut._asyncTimeout), 2314U, ::async::TimeUnit::MILLISECONDS))
            .InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_TRUE(cut._isActive);
    }
    {
        // reset cyclic timeout with new value
        Sequence seq;
        EXPECT_CALL(cut, expired(_)).WillOnce(Reset(2343U));
        EXPECT_CALL(_timeoutMock, cancel(Ref(cut._asyncTimeout))).InSequence(seq);
        EXPECT_CALL(
            _asyncMock,
            scheduleAtFixedRate(
                context, Ref(cut), Ref(cut._asyncTimeout), 2343U, ::async::TimeUnit::MILLISECONDS))
            .InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_TRUE(cut._isActive);
    }
    {
        // cancel cyclic timeout
        Sequence seq;
        EXPECT_CALL(cut, expired(_)).WillOnce(Cancel());
        EXPECT_CALL(_timeoutMock, cancel(Ref(cut._asyncTimeout))).InSequence(seq);
        cut.execute();
        Mock::VerifyAndClearExpectations(&cut);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_FALSE(cut._isActive);
    }
}

} // namespace
