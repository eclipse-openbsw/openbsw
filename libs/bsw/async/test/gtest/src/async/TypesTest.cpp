// Copyright 2024 Accenture.

#include "async/Types.h"

#include "async/LockMock.h"
#include "async/RunnableMock.h"
#include "async/TimeoutMock.h"

namespace
{
using namespace ::async;
using namespace ::testing;

TEST(TypesTest, testLockType)
{
    {
        // without mock
        LockType cut;
    }
    {
        StrictMock<LockMock> lockMock;
        {
            EXPECT_CALL(lockMock, lock());
            LockType cut;
            Mock::VerifyAndClearExpectations(&lockMock);
            EXPECT_CALL(lockMock, unlock());
        }
        Mock::VerifyAndClearExpectations(&lockMock);
    }
}

TEST(TypesTest, testModifiableLockType)
{
    {
        // without mock
        ModifiableLockType cut;
        cut.unlock();
        cut.lock();
    }
    {
        // default behaviour with mock
        StrictMock<LockMock> lockMock;
        {
            EXPECT_CALL(lockMock, lock());
            ModifiableLockType cut;
            Mock::VerifyAndClearExpectations(&lockMock);
            EXPECT_CALL(lockMock, unlock());
        }
        Mock::VerifyAndClearExpectations(&lockMock);
    }
    {
        StrictMock<LockMock> lockMock;
        {
            EXPECT_CALL(lockMock, lock());
            ModifiableLockType cut;
            Mock::VerifyAndClearExpectations(&lockMock);

            // don't expect lock if already locked
            cut.lock();
            Mock::VerifyAndClearExpectations(&lockMock);

            // expect unlock on first call
            EXPECT_CALL(lockMock, unlock());
            cut.unlock();
            Mock::VerifyAndClearExpectations(&lockMock);

            // don't expect unlock if already unlocked
            cut.unlock();
            Mock::VerifyAndClearExpectations(&lockMock);

            // expect unlock if locked
            EXPECT_CALL(lockMock, lock());
            cut.lock();
            EXPECT_CALL(lockMock, unlock());
        }
        Mock::VerifyAndClearExpectations(&lockMock);
    }
}

TEST(TypesTest, testTimeoutType)
{
    {
        // without mock
        TimeoutType timeout;
        timeout.cancel();
    }
    {
        // with mock
        StrictMock<TimeoutMock> timeoutMock;
        {
            TimeoutType timeout;
            EXPECT_CALL(timeoutMock, cancel(Ref(timeout)));
            timeout.cancel();
            Mock::VerifyAndClearExpectations(&timeoutMock);
        }
    }
}

} // namespace
