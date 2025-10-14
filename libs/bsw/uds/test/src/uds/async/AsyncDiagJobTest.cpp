// Copyright 2024 Accenture.

#include "uds/async/AsyncDiagJob.h"

#include "uds/async/AsyncDiagHelperMock.h"
#include "uds/base/AbstractDiagJobMock.h"
#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#define CONTEXT_EXECUTE fContext.execute()

#include <etl/memory.h>
#include <etl/span.h>

namespace
{
using namespace ::testing;
using namespace ::uds;

class DiagJobMock : public AbstractDiagJob
{
public:
    explicit DiagJobMock(
        uint32_t p1 = 0, uint32_t p2 = 0, uint32_t p3 = 0, uint32_t p4 = 0, uint32_t p5 = 0)
    : AbstractDiagJob(nullptr, 0U, 0U, 255U)
    {
        fParams[0] = static_cast<uint8_t>(p1);
        fParams[1] = static_cast<uint8_t>(p2);
        fParams[2] = static_cast<uint8_t>(p3);
        fParams[3] = static_cast<uint8_t>(p4);
        fParams[4] = static_cast<uint8_t>(p5);
    }

    ::etl::span<uint8_t const> getParams() { return ::etl::span<uint8_t const>(fParams, 5); }

    MOCK_METHOD2(verify, DiagReturnCode::Type(uint8_t const request[], uint16_t requestLength));

    MOCK_METHOD3(
        doProcess,
        DiagReturnCode::Type(
            IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength));

    DiagReturnCode::Type
    callProcess(IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
    {
        return process(connection, request, requestLength);
    }

protected:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override
    {
        return doProcess(connection, request, requestLength);
    }

private:
    uint8_t fParams[5];
};

struct AsyncDiagJobTest : public Test
{
    AsyncDiagJobTest()
    {
        AbstractDiagJob::setDefaultDiagSessionManager(fDiagSessionManagerMock);
        EXPECT_CALL(fDiagSessionManagerMock, getActiveSession())
            .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
        fContext.handleAll();
    }

    void SetUp() override
    {
        EXPECT_CALL(fAsyncDiagHelperMock, getDiagContext()).WillRepeatedly(Return(fContext));
    }

    async::TestContext fContext{2U};
    StrictMock<IncomingDiagConnectionMock> fConnection1Mock{fContext};
    StrictMock<IncomingDiagConnectionMock> fConnection2Mock{fContext};
    StrictMock<AsyncDiagHelperMock> fAsyncDiagHelperMock;
    StrictMock<DiagSessionManagerMock> fDiagSessionManagerMock;
    async::AsyncMock fAsyncMock{};
};

TEST_F(AsyncDiagJobTest, Constructors)
{
    ::async::ContextType ctype = 1U;
    {
        // Default constructor
        AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, ctype);
        uint8_t const expectedParams[] = {0U, 0U, 0U, 0U, 0U};
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expectedParams), cut.getParams()));
    }
    {
        // constructor with 1 parameter
        AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, ctype, 1U);
        uint8_t const expectedParams[] = {1U, 0U, 0U, 0U, 0U};
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expectedParams), cut.getParams()));
    }
    {
        // constructor with 2 parameters
        AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, ctype, 1U, 2U);
        uint8_t const expectedParams[] = {1U, 2U, 0U, 0U, 0U};
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expectedParams), cut.getParams()));
    }
    {
        // constructor with 3 parameters
        AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, ctype, 1U, 2U, 3U);
        uint8_t const expectedParams[] = {1U, 2U, 3U, 0U, 0U};
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expectedParams), cut.getParams()));
    }
    {
        // constructor with 4 parameters
        AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, ctype, 1U, 2U, 3U, 4U);
        uint8_t const expectedParams[] = {1U, 2U, 3U, 4U, 0U};
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expectedParams), cut.getParams()));
    }
    {
        // constructor with 5 parameters
        AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, ctype, 1U, 2U, 3U, 4U, 5U);
        uint8_t const expectedParams[] = {1U, 2U, 3U, 4U, 5U};
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expectedParams), cut.getParams()));
    }
}

TEST_F(AsyncDiagJobTest, ProcessAsynchronouslyAndStoreRequestIfNeeded)
{
    AsyncDiagJob<DiagJobMock> cut(fAsyncDiagHelperMock, fContext);

    uint8_t const request1[] = {0x21, 0x23, 0x34};
    // Accept first request
    EXPECT_CALL(cut, verify(request1, sizeof(request1))).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(cut, doProcess(Ref(fConnection1Mock), request1, sizeof(request1)))
        .WillOnce(Return(DiagReturnCode::OK));
    EXPECT_EQ(DiagReturnCode::OK, cut.execute(fConnection1Mock, request1, sizeof(request1)));
    CONTEXT_EXECUTE;

    // Expect second request to be stored
    uint8_t const request2[] = {0x54, 0x21, 0x34};
    IAsyncDiagHelper::StoredRequest storedRequest1(fConnection2Mock, request2, sizeof(request2));
    EXPECT_CALL(
        fAsyncDiagHelperMock, allocateRequest(Ref(fConnection2Mock), request2, sizeof(request2)))
        .WillOnce(Return(&storedRequest1));
    EXPECT_CALL(cut, verify(request2, sizeof(request2))).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_EQ(DiagReturnCode::OK, cut.execute(fConnection2Mock, request2, sizeof(request2)));
    CONTEXT_EXECUTE;

    // send response of first request
    EXPECT_CALL(fConnection1Mock, terminate());
    EXPECT_CALL(fAsyncDiagHelperMock, processAndReleaseRequest(Ref(cut), Ref(storedRequest1)));
    cut.responseSent(fConnection1Mock, AbstractDiagJob::RESPONSE_SENT);
    CONTEXT_EXECUTE;

    EXPECT_CALL(cut, doProcess(Ref(fConnection2Mock), request2, sizeof(request2)))
        .WillOnce(Return(DiagReturnCode::ISO_INVALID_FORMAT));
    EXPECT_CALL(fConnection2Mock, terminate());
    EXPECT_EQ(DiagReturnCode::OK, cut.callProcess(fConnection2Mock, request2, sizeof(request2)));
    CONTEXT_EXECUTE;
}

} // namespace
