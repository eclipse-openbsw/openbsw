// Copyright 2024 Accenture.

#include "uds/base/SubfunctionWithAuthenticationAndSessionControl.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableSubfunctionWithAuthenticationAndSessionControl
: public SubfunctionWithAuthenticationAndSessionControl
{
public:
    TestableSubfunctionWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        DiagSession::DiagSessionMask sessionMask)
    : SubfunctionWithAuthenticationAndSessionControl(
        authenticator, sessionManager, implementedRequest, sessionMask)
    {}

    TestableSubfunctionWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask)
    : SubfunctionWithAuthenticationAndSessionControl(
        authenticator,
        sessionManager,
        implementedRequest,
        requestPayloadLength,
        responseLength,
        sessionMask)
    {}

    IDiagAuthenticator const& getDiagAuthenticator() const override
    {
        return SubfunctionWithAuthentication::getDiagAuthenticator();
    }

    uint8_t const* getImplementedRequest() const
    {
        return SubfunctionWithAuthentication::getImplementedRequest();
    }

    uint8_t getRequestLength() const { return SubfunctionWithAuthentication::getRequestLength(); }

    IDiagSessionManager& getDiagSessionManager() override
    {
        return SubfunctionWithAuthenticationAndSessionControl::getDiagSessionManager();
    }

    IDiagSessionManager const& getDiagSessionManager() const override
    {
        return SubfunctionWithAuthenticationAndSessionControl::getDiagSessionManager();
    }
};

struct SubfunctionWithAuthenticationAndSessionControlTest : ::testing::Test
{
    DefaultDiagAuthenticator fDiagAuthenticator;
    StrictMock<DiagSessionManagerMock> fDiagSessionManager;
};

/**
 * \desc
 * Expect the values which were set in the first constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationAndSessionControlTest,
    call_first_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x22, 0x00};
    TestableSubfunctionWithAuthenticationAndSessionControl subfunctionWithAuthAndSessionCtrl(
        fDiagAuthenticator, fDiagSessionManager, request, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x2200U, subfunctionWithAuthAndSessionCtrl.getRequestId());
    ASSERT_EQ(request, subfunctionWithAuthAndSessionCtrl.getImplementedRequest());
    ASSERT_EQ(2U, subfunctionWithAuthAndSessionCtrl.getRequestLength());
    ASSERT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &subfunctionWithAuthAndSessionCtrl.getDiagAuthenticator());
}

/**
 * \desc
 * Expect the values which were set in the second constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationAndSessionControlTest,
    call_second_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x31, 0x00};
    TestableSubfunctionWithAuthenticationAndSessionControl subfunctionWithAuthAndSessionCtrl(
        fDiagAuthenticator, fDiagSessionManager, request, 1, 2, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x3100U, subfunctionWithAuthAndSessionCtrl.getRequestId());
    ASSERT_EQ(request, subfunctionWithAuthAndSessionCtrl.getImplementedRequest());
    ASSERT_EQ(2U, subfunctionWithAuthAndSessionCtrl.getRequestLength());
    ASSERT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &subfunctionWithAuthAndSessionCtrl.getDiagAuthenticator());
}

/**
 * \desc
 * "getDiagSessionManager" shall return the session manager, which was set in the constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationAndSessionControlTest,
    expect_getDiagSessionManager_returns_the_right_sessionManager_which_was_set_in_the_constructor)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableSubfunctionWithAuthenticationAndSessionControl subfunctionWithAuthAndSessionCtrl(
        fDiagAuthenticator, fDiagSessionManager, request, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(
        static_cast<IDiagSessionManager const*>(&fDiagSessionManager),
        &subfunctionWithAuthAndSessionCtrl.getDiagSessionManager());
}

/**
 * \desc
 * const "getDiagSessionManager" shall return the session manager, which was set
 * in the constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationAndSessionControlTest,
    expect_const_getDiagSessionManager_returns_the_right_sessionManager_which_was_set_in_the_constructor)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableSubfunctionWithAuthenticationAndSessionControl subfunctionWithAuthAndSessionCtrl(
        fDiagAuthenticator, fDiagSessionManager, request, DiagSession::ALL_SESSIONS());

    TestableSubfunctionWithAuthenticationAndSessionControl const& subfunction
        = subfunctionWithAuthAndSessionCtrl;

    EXPECT_EQ(
        static_cast<IDiagSessionManager const*>(&fDiagSessionManager),
        &subfunction.getDiagSessionManager());
}

} // anonymous namespace
