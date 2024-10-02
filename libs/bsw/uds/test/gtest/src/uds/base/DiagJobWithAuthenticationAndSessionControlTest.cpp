// Copyright 2024 Accenture.

#include "uds/base/DiagJobWithAuthenticationAndSessionControl.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableDiagJobWithAuthenticationAndSessionControl
: public DiagJobWithAuthenticationAndSessionControl
{
public:
    TestableDiagJobWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : DiagJobWithAuthenticationAndSessionControl(
        authenticator, sessionManager, implementedRequest, requestLength, prefixLength)
    {}

    TestableDiagJobWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : DiagJobWithAuthenticationAndSessionControl(
        authenticator,
        sessionManager,
        implementedRequest,
        requestLength,
        prefixLength,
        requestPayloadLength,
        responseLength)
    {}

    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override
    {
        if ((request == nullptr) || (requestLength < 1))
        { // no empty requests!
            return DiagReturnCode::ISO_GENERAL_REJECT;
        }
        // any diag request stops the session timeout
        getDiagSessionManager().stopSessionTimeout();
        return DiagReturnCode::OK;
    }

    IDiagSessionManager& getDiagSessionManager() override
    {
        return DiagJobWithAuthenticationAndSessionControl::getDiagSessionManager();
    }

    IDiagSessionManager const& getDiagSessionManager() const override
    {
        return DiagJobWithAuthenticationAndSessionControl::getDiagSessionManager();
    }

    IDiagAuthenticator const& getDiagAuthenticator() const override
    {
        return DiagJobWithAuthenticationAndSessionControl::getDiagAuthenticator();
    }

    uint8_t const* getImplementedRequest() const
    {
        return DiagJobWithAuthenticationAndSessionControl::getImplementedRequest();
    }

    uint8_t getRequestLength() const
    {
        return DiagJobWithAuthenticationAndSessionControl::getRequestLength();
    }
};

struct DiagJobWithAuthenticationAndSessionControlTest : public ::testing::Test
{
    DefaultDiagAuthenticator fDiagAuthenticator;
    StrictMock<DiagSessionManagerMock> fDiagSessionManager;
};

/**
 * \desc
 * Expect the values which were set in the first constructor.
 */
TEST_F(
    DiagJobWithAuthenticationAndSessionControlTest,
    call_first_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableDiagJobWithAuthenticationAndSessionControl diagJobWithAuthenticationAndSessionControl(
        fDiagAuthenticator, fDiagSessionManager, request, sizeof(request), 1);

    EXPECT_EQ(0x220000U, diagJobWithAuthenticationAndSessionControl.getRequestId());
    ASSERT_EQ(
        &fDiagAuthenticator, &diagJobWithAuthenticationAndSessionControl.getDiagAuthenticator());
    ASSERT_EQ(request, diagJobWithAuthenticationAndSessionControl.getImplementedRequest());
    ASSERT_EQ(3U, diagJobWithAuthenticationAndSessionControl.getRequestLength());
}

/**
 * \desc
 * Expect the values which were set in the second constructor.
 */
TEST_F(
    DiagJobWithAuthenticationAndSessionControlTest,
    call_second_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x31, 0x00, 0x00};
    TestableDiagJobWithAuthenticationAndSessionControl diagJobWithAuthenticationAndSessionControl(
        fDiagAuthenticator, fDiagSessionManager, request, sizeof(request), 1, 2, 3);

    EXPECT_EQ(0x310000U, diagJobWithAuthenticationAndSessionControl.getRequestId());
    ASSERT_EQ(
        &fDiagAuthenticator, &diagJobWithAuthenticationAndSessionControl.getDiagAuthenticator());
    ASSERT_EQ(request, diagJobWithAuthenticationAndSessionControl.getImplementedRequest());
    ASSERT_EQ(3U, diagJobWithAuthenticationAndSessionControl.getRequestLength());
}

/**
 * \desc
 * "getDiagSessionManager" shall return the session manager, which was set in the constructor.
 */
TEST_F(
    DiagJobWithAuthenticationAndSessionControlTest,
    expect_getDiagSessionManager_returns_the_right_sessionManager_which_was_set_in_the_constructor)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableDiagJobWithAuthenticationAndSessionControl diagJobWithAuthenticationAndSessionControl(
        fDiagAuthenticator, fDiagSessionManager, request, sizeof(request), 1);

    EXPECT_EQ(
        &fDiagSessionManager, &diagJobWithAuthenticationAndSessionControl.getDiagSessionManager());
}

/**
 * \desc
 * const "getDiagSessionManager" shall return the session manager, which was set
 * in the constructor.
 */
TEST_F(
    DiagJobWithAuthenticationAndSessionControlTest,
    expect_const_getDiagSessionManager_returns_the_right_sessionManager_which_was_set_in_the_constructor)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableDiagJobWithAuthenticationAndSessionControl diagJobWithAuthenticationAndSessionControl(
        fDiagAuthenticator, fDiagSessionManager, request, sizeof(request), 1);

    TestableDiagJobWithAuthenticationAndSessionControl const& diagJob
        = diagJobWithAuthenticationAndSessionControl;

    EXPECT_EQ(&fDiagSessionManager, &diagJob.getDiagSessionManager());
}

} // anonymous namespace
