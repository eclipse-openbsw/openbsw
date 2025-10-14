// Copyright 2024 Accenture.

#include "uds/base/ServiceWithAuthenticationAndSessionControl.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableServiceWithAuthenticationAndSessionControl
: public ServiceWithAuthenticationAndSessionControl
{
public:
    TestableServiceWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const service,
        DiagSession::DiagSessionMask sessionMask)
    : ServiceWithAuthenticationAndSessionControl(
        authenticator, sessionManager, service, sessionMask)
    {}

    TestableServiceWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask)
    : ServiceWithAuthenticationAndSessionControl(
        authenticator, sessionManager, service, requestPayloadLength, responseLength, sessionMask)
    {}

    IDiagAuthenticator const& getDiagAuthenticator() const override
    {
        return ServiceWithAuthentication::getDiagAuthenticator();
    }

    IDiagSessionManager& getDiagSessionManager() override
    {
        return ServiceWithAuthenticationAndSessionControl::getDiagSessionManager();
    }

    IDiagSessionManager const& getDiagSessionManager() const override
    {
        return ServiceWithAuthenticationAndSessionControl::getDiagSessionManager();
    }
};

struct ServiceWithAuthenticationAndSessionControlTest : public ::testing::Test
{
    DefaultDiagAuthenticator fDiagAuthenticator;
    StrictMock<DiagSessionManagerMock> fDiagSessionManager;
};

/**
 * \desc
 * Expect the values, which were set in the first constructor.
 */
TEST_F(
    ServiceWithAuthenticationAndSessionControlTest,
    call_first_constructor_and_expect_that_the_values_are_set_right)
{
    TestableServiceWithAuthenticationAndSessionControl serviceWithAuthenticationAndSessionCtrl(
        fDiagAuthenticator,
        fDiagSessionManager,
        0x11U, // ECUReset service
        DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x11U, serviceWithAuthenticationAndSessionCtrl.getRequestId());
    ASSERT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &serviceWithAuthenticationAndSessionCtrl.getDiagAuthenticator());
}

/**
 * \desc
 * Expect the values, which were set in the second constructor.
 */
TEST_F(
    ServiceWithAuthenticationAndSessionControlTest,
    call_second_constructor_and_expect_that_the_values_are_set_right)
{
    TestableServiceWithAuthenticationAndSessionControl serviceWithAuthenticationAndSessionCtrl(
        fDiagAuthenticator,
        fDiagSessionManager,
        0x10U, // DiagnosticSessionControl service
        1U,
        2U,
        DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x10U, serviceWithAuthenticationAndSessionCtrl.getRequestId());
    ASSERT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &serviceWithAuthenticationAndSessionCtrl.getDiagAuthenticator());
}

/**
 * \desc
 * "getDiagSessionManager" shall return the session manager, which was set in the constructor.
 */
TEST_F(
    ServiceWithAuthenticationAndSessionControlTest,
    expect_getDiagSessionManager_returns_the_right_sessionManager_which_was_set_in_the_constructor)
{
    TestableServiceWithAuthenticationAndSessionControl serviceWithAuthenticationAndSessionCtrl(
        fDiagAuthenticator,
        fDiagSessionManager,
        0x11U, // ECUReset service
        DiagSession::ALL_SESSIONS());

    EXPECT_EQ(
        static_cast<IDiagSessionManager const*>(&fDiagSessionManager),
        &serviceWithAuthenticationAndSessionCtrl.getDiagSessionManager());
}

/**
 * \desc
 * const "getDiagSessionManager" shall return the session manager, which was set
 * in the constructor.
 */
TEST_F(
    ServiceWithAuthenticationAndSessionControlTest,
    expect_const_getDiagSessionManager_returns_the_right_sessionManager_which_was_set_in_the_constructor)
{
    TestableServiceWithAuthenticationAndSessionControl serviceWithAuthenticationAndSessionCtrl(
        fDiagAuthenticator,
        fDiagSessionManager,
        0x11U, // ECUReset service
        DiagSession::ALL_SESSIONS());

    TestableServiceWithAuthenticationAndSessionControl const& service
        = serviceWithAuthenticationAndSessionCtrl;

    EXPECT_EQ(
        static_cast<IDiagSessionManager const*>(&fDiagSessionManager),
        &service.getDiagSessionManager());
}

} // anonymous namespace
