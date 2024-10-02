// Copyright 2024 Accenture.

#include "uds/base/ServiceWithAuthentication.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableServiceWithAuthentication : public ServiceWithAuthentication
{
public:
    TestableServiceWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const service,
        DiagSession::DiagSessionMask sessionMask)
    : ServiceWithAuthentication(authenticator, service, sessionMask)
    {}

    TestableServiceWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask)
    : ServiceWithAuthentication(
        authenticator, service, requestPayloadLength, responseLength, sessionMask)
    {}

    IDiagAuthenticator const& getDiagAuthenticator() const override
    {
        return ServiceWithAuthentication::getDiagAuthenticator();
    }
};

struct ServiceWithAuthenticationTest : ::testing::Test
{
    DefaultDiagAuthenticator fDiagAuthenticator;
};

/**
 * \desc
 * Expect the id, which was set in the first constructor.
 */
TEST_F(ServiceWithAuthenticationTest, call_first_constructor_and_expect_that_the_id_is_set_right)
{
    TestableServiceWithAuthentication serviceWithAuthentication(
        fDiagAuthenticator,
        0x11, // ECUReset service
        DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x11U, serviceWithAuthentication.getRequestId());
}

/**
 * \desc
 * Expect the id, which was set in the second constructor.
 */
TEST_F(ServiceWithAuthenticationTest, call_second_constructor_and_expect_that_the_id_is_set_right)
{
    TestableServiceWithAuthentication serviceWithAuthentication(
        fDiagAuthenticator,
        0x10, // DiagnosticSessionControl service
        1,
        2,
        DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x10U, serviceWithAuthentication.getRequestId());
}

/**
 * \desc
 * "getDiagAuthenticator" shall return the authenticator, which was set in the constructor.
 */
TEST_F(
    ServiceWithAuthenticationTest,
    expect_that_getDiagAuthenticator_returns_the_right_authenticator_which_was_set_in_the_constructor)
{
    TestableServiceWithAuthentication serviceWithAuthentication(
        fDiagAuthenticator,
        0x11, // ECUReset service
        DiagSession::ALL_SESSIONS());

    EXPECT_EQ(
        static_cast<IDiagAuthenticator const*>(&fDiagAuthenticator),
        &serviceWithAuthentication.getDiagAuthenticator());
}

} // anonymous namespace
