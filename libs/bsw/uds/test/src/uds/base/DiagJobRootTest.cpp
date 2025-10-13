// Copyright 2024 Accenture.

/**
 * Contains unit tests for uds::DiagJobRoot.
 */
#include "uds/base/DiagJobRoot.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableDiagJobRoot : public DiagJobRoot
{
public:
    using AbstractDiagJob::getImplementedRequest;
    using AbstractDiagJob::getRequestLength;
};

struct DiagJobRootTest : Test
{
    DiagJobRootTest() { AbstractDiagJob::setDefaultDiagSessionManager(fSessionManager); }

    DiagSessionManagerMock fSessionManager;
    TestableDiagJobRoot fRoot;
    IncomingDiagConnection fIncomingConnection{::async::CONTEXT_INVALID};
    ApplicationDefaultSession fApplicationSession;
};

TEST_F(DiagJobRootTest, constructor)
{
    EXPECT_EQ(0x00U, fRoot.getRequestId());
    ASSERT_EQ(nullptr, fRoot.getImplementedRequest());
    ASSERT_EQ(0U, fRoot.getRequestLength());
}

/**
 * \desc
 * A request, which is too small shall return ISO_GENERAL_REJECT.
 */
TEST_F(DiagJobRootTest, verify_general_reject)
{
    ASSERT_EQ(DiagReturnCode::ISO_GENERAL_REJECT, fRoot.verify(nullptr, 0));
    ASSERT_EQ(DiagReturnCode::ISO_GENERAL_REJECT, fRoot.verify(nullptr, 1));
    uint8_t const request[] = {0x22, 0x00, 0x00};
    ASSERT_EQ(DiagReturnCode::ISO_GENERAL_REJECT, fRoot.verify(request, 0));
}

/**
 * \desc
 * A non empty request shall return OK.
 */
TEST_F(DiagJobRootTest, verify_ok)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    ASSERT_EQ(DiagReturnCode::OK, fRoot.verify(request, sizeof(request)));
}

/**
 * \desc
 * If no session timeout is active, the request length is > 1 and the positive
 * response is suppressed, then the TesterPresent functional request should be ignore
 * and execute shall return OK.
 */
TEST_F(
    DiagJobRootTest,
    session_timeout_is_not_active_so_TesterPresent_request_should_be_ignore_and_execute_returns_OK)
{
    uint8_t const request[]           = {0x3E, 0x80};
    uint16_t const serviceId          = request[0];
    uint16_t const functionalAddress  = 0xDF;
    fIncomingConnection.targetAddress = functionalAddress;
    fIncomingConnection.serviceId     = serviceId;

    EXPECT_CALL(fSessionManager, isSessionTimeoutActive()).WillOnce(Return(false));

    ASSERT_EQ(DiagReturnCode::OK, fRoot.execute(fIncomingConnection, request, sizeof(request)));
}

/**
 * \desc
 * If session timeout is active the TesterPresent functional request should be execute without
 * further checks. Processes the given request shall result in a negative response
 * (ISO_SERVICE_NOT_SUPPORTED), because no special settings were made.
 */
TEST_F(
    DiagJobRootTest, session_timeout_is_active_AbstractDiagJob_execute_should_be_call_immediately)
{
    uint8_t const request[]           = {0x3E, 0x80};
    uint16_t const serviceId          = request[0];
    uint16_t const functionalAddress  = 0xDF;
    fIncomingConnection.targetAddress = functionalAddress;
    fIncomingConnection.serviceId     = serviceId;

    EXPECT_CALL(fSessionManager, isSessionTimeoutActive()).WillOnce(Return(true));

    EXPECT_CALL(fSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    ASSERT_EQ(
        DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED,
        fRoot.execute(fIncomingConnection, request, sizeof(request)));
}

/**
 * \desc
 * Since the service id is not TesterPresent the diag job can be executed immediately without
 * further checks. execute shall return ISO_SERVICE_NOT_SUPPORTED, because no special settings
 * were made.
 */
TEST_F(
    DiagJobRootTest,
    service_id_is_not_TesterPresent_AbstractDiagJob_execute_should_be_call_immediately)
{
    uint8_t const request[]           = {0x21, 0x80};
    uint16_t const serviceId          = request[0];
    uint16_t const functionalAddress  = 0xDF;
    fIncomingConnection.targetAddress = functionalAddress;
    fIncomingConnection.serviceId     = serviceId;

    EXPECT_CALL(fSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    ASSERT_EQ(
        DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED,
        fRoot.execute(fIncomingConnection, request, sizeof(request)));
}

/**
 * \desc
 * Session timeout is active, but the length of the TesterPresent request is too short.
 * execute shall be called without further checks. Processes the given request shall result
 * in a negative response (ISO_SERVICE_NOT_SUPPORTED), because no special settings were made.
 */
TEST_F(
    DiagJobRootTest,
    TesterPresent_request_is_too_short_AbstractDiagJob_execute_should_be_call_immediately)
{
    uint8_t const request[]           = {0x3E};
    uint16_t const serviceId          = request[0];
    uint16_t const functionalAddress  = 0xDF;
    fIncomingConnection.targetAddress = functionalAddress;
    fIncomingConnection.serviceId     = serviceId;

    EXPECT_CALL(fSessionManager, isSessionTimeoutActive()).WillOnce(Return(false));

    EXPECT_CALL(fSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    ASSERT_EQ(
        DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED,
        fRoot.execute(fIncomingConnection, request, sizeof(request)));
}

/**
 * \desc
 * Session timeout is active and the length of the TesterPresent request is ok, but the positive
 * response is not suppress. execute shall be called without further checks.
 * Processes the given request shall result in a negative response (ISO_SERVICE_NOT_SUPPORTED),
 * because no special settings were made.
 */
TEST_F(
    DiagJobRootTest,
    positive_response_is_not_suppress_AbstractDiagJob_execute_should_be_call_immediately)
{
    uint8_t const request[]           = {0x3E, 0x00};
    uint16_t const serviceId          = request[0];
    uint16_t const functionalAddress  = 0xDF;
    fIncomingConnection.targetAddress = functionalAddress;
    fIncomingConnection.serviceId     = serviceId;

    EXPECT_CALL(fSessionManager, isSessionTimeoutActive()).WillOnce(Return(false));

    EXPECT_CALL(fSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    ASSERT_EQ(
        DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED,
        fRoot.execute(fIncomingConnection, request, sizeof(request)));
}

} // namespace
