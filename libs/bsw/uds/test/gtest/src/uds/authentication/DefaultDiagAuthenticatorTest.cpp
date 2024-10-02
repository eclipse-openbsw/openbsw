// Copyright 2024 Accenture.

#include "uds/authentication/DefaultDiagAuthenticator.h"

#include <gtest/gtest.h>

TEST(DefaultDiagAuthenticatorTest, isAuthenticated_expected_true_when_giving_ecuId)
{
    ::uds::DefaultDiagAuthenticator defaultDiagAuthenticator;
    ASSERT_TRUE(defaultDiagAuthenticator.isAuthenticated(42U));
}

TEST(
    DefaultDiagAuthenticatorTest, getNotAuthenticatedReturnCode_returns_ISO_AUTHENTICATION_REQUIRED)
{
    ::uds::DefaultDiagAuthenticator defaultDiagAuthenticator;
    EXPECT_EQ(
        ::uds::DiagReturnCode::ISO_AUTHENTICATION_REQUIRED,
        defaultDiagAuthenticator.getNotAuthenticatedReturnCode());
}
