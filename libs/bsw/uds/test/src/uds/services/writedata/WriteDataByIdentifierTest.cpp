// Copyright 2024 Accenture.

#include "uds/services/writedata/WriteDataByIdentifier.h"

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <transport/TransportMessageWithBuffer.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class TestableWriteDataByIdentifier : public WriteDataByIdentifier
{
public:
    TestableWriteDataByIdentifier() : fWriteDataByIdentifier() {}

    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return WriteDataByIdentifier::getDefaultDiagReturnCode();
    }

    virtual DiagReturnCode::Type process(
        IncomingDiagConnection& /* connection */,
        uint8_t const /* request */[],
        uint16_t /* requestLength */)
    {
        return DiagReturnCode::OK;
    }

protected:
    WriteDataByIdentifier fWriteDataByIdentifier;
};

class WriteDataByIdentifierTest : public ::testing::Test
{
public:
    WriteDataByIdentifierTest()
    : fTestableWriteDataByIdentifier()
    , fIncomingDiagConnection(::async::CONTEXT_INVALID)
    , fSessionManager()
    {}

    virtual void SetUp()
    {
        fTestableWriteDataByIdentifier.setDefaultDiagSessionManager(fSessionManager);
    }

protected:
    TestableWriteDataByIdentifier fTestableWriteDataByIdentifier;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
};

TEST_F(
    WriteDataByIdentifierTest, constructor_should_set_the_DiagReturnCode_ISO_REQUEST_OUT_OF_RANGE)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fTestableWriteDataByIdentifier.getDefaultDiagReturnCode());
}

TEST_F(
    WriteDataByIdentifierTest,
    verify_which_is_called_by_execute_should_return_the_DiagReturnCode_OK)
{
    uint8_t request[] = {0x2EU, 0x00U, 0x01U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fTestableWriteDataByIdentifier.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    WriteDataByIdentifierTest,
    verify_which_is_called_by_execute_should_return_ISO_INVALID_FORMAT_because_of_wrong_requestLength)
{
    uint8_t request[] = {0x2EU, 0x00U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fTestableWriteDataByIdentifier.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    WriteDataByIdentifierTest,
    verify_which_is_called_by_execute_should_return_NOT_RESPONSIBLE_because_service_request_is_wrong)
{
    uint8_t request[] = {0x22U, 0x00U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        fTestableWriteDataByIdentifier.execute(fIncomingDiagConnection, request, sizeof(request)));
}

} // anonymous namespace
