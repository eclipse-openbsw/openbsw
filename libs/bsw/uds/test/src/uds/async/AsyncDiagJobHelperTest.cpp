// Copyright 2024 Accenture.

#include "uds/async/AsyncDiagJobHelper.h"

#include "uds/async/AsyncDiagHelper.h"
#include "uds/async/AsyncDiagHelperMock.h"
#include "uds/base/AbstractDiagJobMock.h"
#include "uds/connection/IncomingDiagConnectionMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#define CONTEXT_EXECUTE fContext.execute()

namespace
{
using namespace ::testing;
using namespace ::uds;

ACTION_P2(StartAsyncRequest, cut, connection) { cut->startAsyncRequest(*connection); }

struct AsyncDiagJobHelperTest : public Test
{
    AsyncDiagJobHelperTest() { fContext.handleAll(); }

    void SetUp() override
    {
        EXPECT_CALL(fAsyncHelperMock, getDiagContext()).WillRepeatedly(Return(fContext));
    }

    async::TestContext fContext{2U};

    StrictMock<AbstractDiagJobMock> fDiagJobMock{
        static_cast<uint8_t const*>(nullptr), 0, 0, DiagSession::DiagSessionMask()};
    StrictMock<IncomingDiagConnectionMock> fConnectionMock1{fContext};
    StrictMock<IncomingDiagConnectionMock> fConnectionMock2{fContext};
    StrictMock<IncomingDiagConnectionMock> fConnectionMock3{fContext};
    StrictMock<IncomingDiagConnectionMock> fConnectionMock4{fContext};
    StrictMock<AsyncDiagHelperMock> fAsyncHelperMock;
    async::AsyncMock fAsyncMock;
};

TEST_F(AsyncDiagJobHelperTest, AllocateAndProcess)
{
    AsyncDiagJobHelper cut(fAsyncHelperMock, fDiagJobMock);

    EXPECT_FALSE(cut.hasPendingAsyncRequest());

    cut.startAsyncRequest(fConnectionMock1);
    EXPECT_TRUE(cut.hasPendingAsyncRequest());
    EXPECT_EQ(&fConnectionMock1, &cut.getPendingConnection());

    cut.endAsyncRequest();
    CONTEXT_EXECUTE;
    EXPECT_FALSE(cut.hasPendingAsyncRequest());
}

TEST_F(AsyncDiagJobHelperTest, EnqueueAndProcessAsynchronously)
{
    AsyncDiagJobHelper cut(fAsyncHelperMock, fDiagJobMock);

    cut.endAsyncRequest();

    cut.startAsyncRequest(fConnectionMock1);

    uint8_t buffer2[15];
    IAsyncDiagHelper::StoredRequest storedRequest2(fConnectionMock2, buffer2, sizeof(buffer2));
    EXPECT_CALL(fAsyncHelperMock, allocateRequest(Ref(fConnectionMock2), buffer2, sizeof(buffer2)))
        .WillOnce(Return(&storedRequest2));
    EXPECT_EQ(DiagReturnCode::OK, cut.enqueueRequest(fConnectionMock2, buffer2, sizeof(buffer2)));

    uint8_t buffer3[20];
    IAsyncDiagHelper::StoredRequest storedRequest3(fConnectionMock3, buffer3, sizeof(buffer3));
    EXPECT_CALL(fAsyncHelperMock, allocateRequest(Ref(fConnectionMock3), buffer3, sizeof(buffer3)))
        .WillOnce(Return(&storedRequest3));
    EXPECT_EQ(DiagReturnCode::OK, cut.enqueueRequest(fConnectionMock3, buffer3, sizeof(buffer3)));

    uint8_t buffer4[25];
    EXPECT_CALL(fAsyncHelperMock, allocateRequest(Ref(fConnectionMock4), buffer4, sizeof(buffer4)))
        .WillOnce(Return(static_cast<IAsyncDiagHelper::StoredRequest*>(nullptr)));
    EXPECT_EQ(
        DiagReturnCode::ISO_BUSY_REPEAT_REQUEST,
        cut.enqueueRequest(fConnectionMock4, buffer4, sizeof(buffer4)));

    EXPECT_CALL(fAsyncHelperMock, processAndReleaseRequest(Ref(fDiagJobMock), Ref(storedRequest2)));
    EXPECT_CALL(fAsyncHelperMock, processAndReleaseRequest(Ref(fDiagJobMock), Ref(storedRequest3)))
        .WillOnce(StartAsyncRequest(&cut, &fConnectionMock3));
    cut.endAsyncRequest();
    CONTEXT_EXECUTE;

    EXPECT_TRUE(cut.hasPendingAsyncRequest());
    cut.endAsyncRequest();
}

} // namespace
