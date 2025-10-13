// Copyright 2024 Accenture.

#include "uds/services/testerpresent/TesterPresent.h"

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <transport/TransportMessageWithBuffer.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class TesterPresentTest : public ::testing::Test
{
public:
    TesterPresentTest()
    : fTesterPresent(), fIncomingDiagConnection(::async::CONTEXT_INVALID), fSessionManager()
    {}

    virtual void SetUp() { fTesterPresent.setDefaultDiagSessionManager(fSessionManager); }

protected:
    TesterPresent fTesterPresent;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
};

TEST_F(TesterPresentTest, process_which_is_called_by_execute_should_return_DiagReturnCode_OK)
{
    InSequence seq;

    uint8_t request[] = {0x3EU, 0x00U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fTesterPresent.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    TesterPresentTest,
    process_which_is_called_by_execute_should_return_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    InSequence seq;

    uint8_t request[] = {0x3EU, 0x01U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fTesterPresent.execute(fIncomingDiagConnection, request, sizeof(request)));
}

} // anonymous namespace
