/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
