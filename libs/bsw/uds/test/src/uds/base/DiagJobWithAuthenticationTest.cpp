// Copyright 2024 Accenture.

#include "uds/base/DiagJobWithAuthentication.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/session/IDiagSessionManager.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableDiagJobWithAuthentication : public DiagJobWithAuthentication
{
public:
    TestableDiagJobWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : DiagJobWithAuthentication(
        authenticator, implementedRequest, requestLength, prefixLength, sessionMask)
    {}

    TestableDiagJobWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : DiagJobWithAuthentication(
        authenticator,
        implementedRequest,
        requestLength,
        prefixLength,
        requestPayloadLength,
        responseLength,
        sessionMask)
    {}

    virtual DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength)
    {
        if ((request == nullptr) || (requestLength < 1))
        { // no empty requests!
            return DiagReturnCode::ISO_GENERAL_REJECT;
        }
        // any diag request stops the session timeout
        getDiagSessionManager().stopSessionTimeout();
        return DiagReturnCode::OK;
    }

    virtual IDiagAuthenticator const& getDiagAuthenticator() const
    {
        return DiagJobWithAuthentication::getDiagAuthenticator();
    }

    using AbstractDiagJob::getImplementedRequest;
    using AbstractDiagJob::getRequestLength;
};

struct DiagJobWithAuthenticationTest : ::testing::Test
{
    DefaultDiagAuthenticator fDiagAuthenticator;
};

/**
 * \desc
 * Expect the values which were set in the first constructor.
 */
TEST_F(DiagJobWithAuthenticationTest, call_first_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableDiagJobWithAuthentication diagJobWithAuthentication(
        fDiagAuthenticator, request, sizeof(request), 1);

    EXPECT_EQ(0x220000U, diagJobWithAuthentication.getRequestId());
    ASSERT_EQ(request, diagJobWithAuthentication.getImplementedRequest());
    ASSERT_EQ(3U, diagJobWithAuthentication.getRequestLength());
}

/**
 * \desc
 * Expect the values which were set in the second constructor.
 */
TEST_F(DiagJobWithAuthenticationTest, call_second_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x31, 0x00, 0x00};
    TestableDiagJobWithAuthentication diagJobWithAuthentication(
        fDiagAuthenticator, request, sizeof(request), 1, 2, 3);

    EXPECT_EQ(0x310000U, diagJobWithAuthentication.getRequestId());
    ASSERT_EQ(request, diagJobWithAuthentication.getImplementedRequest());
    ASSERT_EQ(3U, diagJobWithAuthentication.getRequestLength());
}

/**
 * \desc
 * "getDiagAuthenticator" shall return the authenticator, which was set in the constructor.
 */
TEST_F(
    DiagJobWithAuthenticationTest,
    expect_that_getDiagAuthenticator_returns_the_right_authenticator_which_was_set_in_the_constructor)
{
    uint8_t const request[] = {0x22, 0x00, 0x00};
    TestableDiagJobWithAuthentication diagJobWithAuthentication(
        fDiagAuthenticator, request, sizeof(request), 1);

    EXPECT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &diagJobWithAuthentication.getDiagAuthenticator());
}

} // anonymous namespace
