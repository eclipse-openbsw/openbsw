// Copyright 2024 Accenture.

#include "util/timeout/AsyncTimeoutManager.h"

#include "async/AsyncMock.h"
#include "async/TimeoutMock.h"
#include "util/timeout/AbstractTimeout.h"

namespace
{
using namespace common;
using namespace testing;

class AbstractTimeoutMock : public ::common::AbstractTimeout
{
public:
    MOCK_METHOD1(expired, void(TimeoutExpiredActions));
};

class AsyncTimeoutManagerTest : public Test
{
public:
    StrictMock<AbstractTimeoutMock> _abstractTimeoutMock1;
    StrictMock<AbstractTimeoutMock> _abstractTimeoutMock2;
    StrictMock<::async::AsyncMock> _asyncMock;
    StrictMock<::async::TimeoutMock> _timeoutMock;
};

TEST_F(AsyncTimeoutManagerTest, testAll)
{
    AsyncTimeoutManager cut;
    ::async::ContextType context(3U);
    cut.init(context);
    {
        cut.init();
    }
    {
        // check set single timeout
        EXPECT_CALL(
            _asyncMock,
            schedule(
                context,
                Ref(_abstractTimeoutMock1),
                Ref(_abstractTimeoutMock1._asyncTimeout),
                1239U,
                ::async::TimeUnit::MILLISECONDS));
        EXPECT_EQ(ITimeoutManager2::TIMEOUT_OK, cut.set(_abstractTimeoutMock1, 1239U, false));
        EXPECT_TRUE(_abstractTimeoutMock1.isActive());

        // setting again should return bad result
        EXPECT_EQ(
            ITimeoutManager2::TIMEOUT_ALREADY_SET, cut.set(_abstractTimeoutMock1, 343U, false));

        // expire
        EXPECT_CALL(_abstractTimeoutMock1, expired(_));
        _abstractTimeoutMock1.execute();
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_abstractTimeoutMock1);
        EXPECT_FALSE(_abstractTimeoutMock1.isActive());
    }
    {
        // check set periodic timeout
        EXPECT_CALL(
            _asyncMock,
            scheduleAtFixedRate(
                context,
                Ref(_abstractTimeoutMock1),
                Ref(_abstractTimeoutMock1._asyncTimeout),
                1349U,
                ::async::TimeUnit::MILLISECONDS));
        EXPECT_EQ(ITimeoutManager2::TIMEOUT_OK, cut.set(_abstractTimeoutMock1, 1349U, true));
        EXPECT_TRUE(_abstractTimeoutMock1.isActive());

        // expire
        EXPECT_CALL(_abstractTimeoutMock1, expired(_));
        _abstractTimeoutMock1.execute();
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_abstractTimeoutMock1);
        EXPECT_TRUE(_abstractTimeoutMock1.isActive());

        // expire again
        EXPECT_CALL(_abstractTimeoutMock1, expired(_));
        _abstractTimeoutMock1.execute();
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_abstractTimeoutMock1);
        EXPECT_TRUE(_abstractTimeoutMock1.isActive());
    }
    {
        // check cancel
        EXPECT_CALL(_timeoutMock, cancel(Ref(_abstractTimeoutMock1._asyncTimeout)));
        cut.cancel(_abstractTimeoutMock1);
        Mock::VerifyAndClearExpectations(&_timeoutMock);
        EXPECT_FALSE(_abstractTimeoutMock1.isActive());

        // expire shouldn't work
        _abstractTimeoutMock1.execute();
        Mock::VerifyAndClearExpectations(&_asyncMock);
        Mock::VerifyAndClearExpectations(&_abstractTimeoutMock1);
    }
    {
        cut.shutdown();
    }
}

} // namespace
