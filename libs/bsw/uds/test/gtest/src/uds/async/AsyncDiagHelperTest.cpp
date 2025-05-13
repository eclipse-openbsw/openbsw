// Copyright 2024 Accenture.

#include "uds/async/AsyncDiagHelper.h"

#include "uds/base/AbstractDiagJobMock.h"
#include "uds/connection/IncomingDiagConnectionMock.h"

namespace
{
using namespace ::testing;
using namespace ::uds;

class AsyncDiagHelperMock : public ::uds::declare::AsyncDiagHelper<1>
{
public:
    AsyncDiagHelperMock(::async::ContextType diagContext)
    : ::uds::declare::AsyncDiagHelper<1>(diagContext)
    {}

    using AsyncDiagHelper::verify;
};

struct AsyncDiagHelperTest : public Test
{
    StrictMock<AbstractDiagJobMock> fDiagJobMock{
        static_cast<uint8_t const*>(nullptr), 0, 0, DiagSession::DiagSessionMask()};
    StrictMock<IncomingDiagConnectionMock> fConnectionMock{::async::CONTEXT_INVALID};
};

TEST_F(AsyncDiagHelperTest, AllocateAndProcess)
{
    ::uds::declare::AsyncDiagHelper<2> cut(2U);

    uint8_t buffer1[20];
    AsyncDiagHelper::StoredRequest* request1
        = cut.allocateRequest(fConnectionMock, buffer1, sizeof(buffer1));
    ASSERT_TRUE(request1 != nullptr);
    EXPECT_EQ(&fConnectionMock, &request1->getConnection());
    EXPECT_EQ(buffer1, request1->getRequest());
    EXPECT_EQ(sizeof(buffer1), request1->getRequestLength());

    uint8_t buffer2[15];
    AsyncDiagHelper::StoredRequest* request2
        = cut.allocateRequest(fConnectionMock, buffer2, sizeof(buffer2));
    ASSERT_TRUE(request2 != nullptr);
    EXPECT_EQ(&fConnectionMock, &request2->getConnection());
    EXPECT_EQ(buffer2, request2->getRequest());
    EXPECT_EQ(sizeof(buffer2), request2->getRequestLength());

    uint8_t buffer3[25];
    EXPECT_EQ(nullptr, cut.allocateRequest(fConnectionMock, buffer3, sizeof(buffer3)));

    EXPECT_CALL(fDiagJobMock, process(Ref(fConnectionMock), buffer2, sizeof(buffer2)))
        .WillOnce(Return(DiagReturnCode::OK));
    cut.processAndReleaseRequest(fDiagJobMock, *request2);

    AsyncDiagHelper::StoredRequest* request3
        = cut.allocateRequest(fConnectionMock, buffer3, sizeof(buffer3));
    ASSERT_TRUE(request3 != nullptr);
    EXPECT_EQ(&fConnectionMock, &request3->getConnection());
    EXPECT_EQ(buffer3, request3->getRequest());
    EXPECT_EQ(sizeof(buffer3), request3->getRequestLength());

    EXPECT_CALL(fDiagJobMock, process(Ref(fConnectionMock), buffer1, sizeof(buffer1)))
        .WillOnce(Return(DiagReturnCode::ISO_BUSY_REPEAT_REQUEST));
    EXPECT_CALL(fConnectionMock, terminate());
    cut.processAndReleaseRequest(fDiagJobMock, *request1);
}

TEST_F(AsyncDiagHelperTest, CoverVerify)
{
    AsyncDiagHelperMock cut(2U);
    uint8_t buffer[10];
    EXPECT_EQ(DiagReturnCode::OK, cut.verify(buffer, sizeof(buffer)));
}

} // namespace
