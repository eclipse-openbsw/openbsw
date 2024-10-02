// Copyright 2024 Accenture.

#include "uds/base/SubfunctionWithAuthentication.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableSubfunctionWithAuthentication : public SubfunctionWithAuthentication
{
public:
    TestableSubfunctionWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        DiagSession::DiagSessionMask sessionMask)
    : SubfunctionWithAuthentication(authenticator, implementedRequest, sessionMask)
    {}

    TestableSubfunctionWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask)
    : SubfunctionWithAuthentication(
        authenticator, implementedRequest, requestPayloadLength, responseLength, sessionMask)
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
};

struct SubfunctionWithAuthenticationTest : public ::testing::Test
{
    DefaultDiagAuthenticator fDiagAuthenticator;
};

/**
 * \desc
 * Expect the values which were set in the first constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationTest, call_first_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x22U, 0x00U};
    TestableSubfunctionWithAuthentication subfunctionWithAuthentication(
        fDiagAuthenticator, request, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x2200U, subfunctionWithAuthentication.getRequestId());
    ASSERT_EQ(request, subfunctionWithAuthentication.getImplementedRequest());
    ASSERT_EQ(2U, subfunctionWithAuthentication.getRequestLength());
}

/**
 * \desc
 * Expect the values which were set in the second constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationTest, call_second_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x31U, 0x00U};
    TestableSubfunctionWithAuthentication subfunctionWithAuthentication(
        fDiagAuthenticator, request, 1U, 2U, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x3100U, subfunctionWithAuthentication.getRequestId());
    ASSERT_EQ(request, subfunctionWithAuthentication.getImplementedRequest());
    ASSERT_EQ(2U, subfunctionWithAuthentication.getRequestLength());
}

/**
 * \desc
 * "getDiagAuthenticator" shall return the authenticator, which was set in the constructor.
 */
TEST_F(
    SubfunctionWithAuthenticationTest,
    expect_that_getDiagAuthenticator_returns_the_right_authenticator_which_was_set_in_the_constructor)
{
    uint8_t const request[] = {0x22U, 0x00U, 0x00U};
    TestableSubfunctionWithAuthentication subfunctionWithAuthentication(
        fDiagAuthenticator, request, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &subfunctionWithAuthentication.getDiagAuthenticator());
}

} // anonymous namespace
