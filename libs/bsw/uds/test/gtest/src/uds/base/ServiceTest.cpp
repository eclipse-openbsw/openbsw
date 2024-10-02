// Copyright 2024 Accenture.

#include "uds/services/communicationcontrol/CommunicationControl.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestService : public Service
{
public:
    TestService(uint8_t const service, DiagSession::DiagSessionMask sessionMask)
    : Service(service, sessionMask)
    {}

    TestService(
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask)
    : Service(service, requestPayloadLength, responseLength, sessionMask)
    {}

    using Service::enableSuppressPositiveResponse;

    using Service::verify;
};

struct ServiceTest : Test
{
    ServiceTest() { AbstractDiagJob::setDefaultDiagSessionManager(_sessionManager); }

    static uint8_t const MY_SERVICE_ID = 0x01;

    DiagSessionManagerMock _sessionManager;
    TestService _service{
        MY_SERVICE_ID,
        DiagSession::DiagSessionMask::getInstance() << DiagSession::APPLICATION_DEFAULT_SESSION()};
};

TEST_F(ServiceTest, VariableLengthConstructor) { EXPECT_EQ(0x01U, _service.getRequestId()); }

TEST_F(ServiceTest, FixedLengthConstructor)
{
    TestService s(
        MY_SERVICE_ID,
        3,
        3,
        DiagSession::DiagSessionMask::getInstance() << DiagSession::APPLICATION_DEFAULT_SESSION());
    EXPECT_EQ(0x01U, s.getRequestId());
}

TEST_F(ServiceTest, VerifyOK)
{
    uint8_t request[] = {MY_SERVICE_ID, 0x02, 0x03};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.verify(&request[0], 3));
}

TEST_F(ServiceTest, VerifyNotResponsible)
{
    uint8_t request[] = {0x02, 0x02, 0x03};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::NOT_RESPONSIBLE, _service.verify(&request[0], 3));
}

TEST_F(ServiceTest, VerifyServiceNotSupportedInActiveSession)
{
    uint8_t request[] = {0x01, 0x02, 0x03};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(
        DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION,
        _service.verify(&request[0], 3));
}

} // namespace
