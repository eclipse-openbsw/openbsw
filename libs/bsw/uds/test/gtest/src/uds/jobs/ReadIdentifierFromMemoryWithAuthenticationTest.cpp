// Copyright 2024 Accenture.

#include "uds/jobs/ReadIdentifierFromMemoryWithAuthentication.h"

#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/session/ApplicationDefaultSession.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableReadIdentifierFromMemoryWithAuthentication
: public ReadIdentifierFromMemoryWithAuthentication
{
public:
    TestableReadIdentifierFromMemoryWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint16_t const id,
        uint8_t const responseData[],
        uint8_t const responseLength,
        DiagSessionMask const sessionMask)
    : ReadIdentifierFromMemoryWithAuthentication(
        authenticator, id, responseData, responseLength, sessionMask)
    {}

    TestableReadIdentifierFromMemoryWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint16_t const id,
        ::estd::slice<uint8_t const> const& responseData,
        DiagSessionMask const sessionMask)
    : ReadIdentifierFromMemoryWithAuthentication(authenticator, id, responseData, sessionMask)
    {}

    IDiagAuthenticator const& getDiagAuthenticator() const
    {
        return ReadIdentifierFromMemoryWithAuthentication::getDiagAuthenticator();
    }
};

TEST(
    ReadIdentifierFromMemoryWithAuthenticationTest, call_constructor_and_check_getDiagAuthenticator)
{
    uint16_t const IDENTIFIER = 0x4242U;
    uint8_t const DATA[4U]    = {0x00U, 0x01U, 0x02U, 0x03U};
    DefaultDiagAuthenticator const authenticator;

    TestableReadIdentifierFromMemoryWithAuthentication readIdFromMemoryWithAuth(
        authenticator,
        IDENTIFIER,
        DATA,
        sizeof(DATA),
        AbstractDiagJob::DiagSessionMask::getInstance()
            << DiagSession::APPLICATION_DEFAULT_SESSION());

    // DefaultDiagAuthenticator always returns true for isAuthenticated()
    EXPECT_TRUE(readIdFromMemoryWithAuth.getDiagAuthenticator().isAuthenticated(72U));
}

} // namespace
