/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/securityaccess/SecurityAccess.h"

#include "transport/TransportConfiguration.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableSecurityAccess : public SecurityAccess
{
public:
    TestableSecurityAccess(DiagSession::DiagSessionMask mask) : SecurityAccess(mask) {}

    uint16_t getCurrentEcuId() { return SecurityAccess::fCurrentEcuId; }

    uint16_t getAuthenticatedEcu() { return SecurityAccess::fAuthenticatedEcu; }

    void setCurrentEcuId(uint16_t ecuId) { SecurityAccess::fCurrentEcuId = ecuId; }

    void setAuthenticatedEcu(uint16_t ecuId) { SecurityAccess::fAuthenticatedEcu = ecuId; }
};

class SecurityAccessTest : public ::testing::Test
{
public:
    SecurityAccessTest() : fDiagSessionMask(), fSecurityAccess(fDiagSessionMask) {}

protected:
    DiagSession::DiagSessionMask fDiagSessionMask;
    TestableSecurityAccess fSecurityAccess;
};

/**
 * \desc
 * Expect the values which were set in the constructor.
 */
TEST_F(SecurityAccessTest, constructor)
{
    EXPECT_EQ(0x27U, fSecurityAccess.getRequestId());
    EXPECT_EQ(
        transport::TransportConfiguration::INVALID_DIAG_ADDRESS,
        fSecurityAccess.getAuthenticatedEcu());
    EXPECT_EQ(
        transport::TransportConfiguration::INVALID_DIAG_ADDRESS, fSecurityAccess.getCurrentEcuId());
    EXPECT_EQ(0, fSecurityAccess.getSecurityAccessType());
}

/**
 * \desc
 * isAuthenticated should returns true, if the ecuId is authenticated, otherwise it should
 * return false.
 */
TEST_F(SecurityAccessTest, isAuthenticated_returns_true_if_ecuId_is_authenticated_otherwise_false)
{
    EXPECT_TRUE(
        fSecurityAccess.isAuthenticated(transport::TransportConfiguration::INVALID_DIAG_ADDRESS));
    EXPECT_FALSE(fSecurityAccess.isAuthenticated(0x10U));
}

/**
 * \desc
 * resetAuthentication should reset the authenticated ecu and the current ecu.
 */
TEST_F(SecurityAccessTest, resetAuthentication)
{
    fSecurityAccess.setCurrentEcuId(0x10U);
    fSecurityAccess.setAuthenticatedEcu(0x10U);

    fSecurityAccess.resetAuthentication();
    EXPECT_EQ(
        transport::TransportConfiguration::INVALID_DIAG_ADDRESS, fSecurityAccess.getCurrentEcuId());
    EXPECT_EQ(
        transport::TransportConfiguration::INVALID_DIAG_ADDRESS,
        fSecurityAccess.getAuthenticatedEcu());
}

} // anonymous namespace
