/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SessionManager.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

struct SessionManagerTest : ::testing::Test
{
    declare::SessionManager<16U> manager;

    uint16_t sessionId = 0U;
    bool rebootFlag    = false;
};

/**
 * Make sure sessionId and rebootFlag are affected correctly in case of multicast and rollover.
 */
TEST_F(SessionManagerTest, getSessionInfoForNextMulticastMessage_sessionId_when_rollover)
{
    for (uint32_t i = 1U; i <= 0xFFFFU; ++i)
    {
        manager.getSessionInfoForNextMulticastMessage(sessionId, rebootFlag);
        EXPECT_EQ(i, sessionId);
        EXPECT_TRUE(rebootFlag);
    }

    // next one will be a rollover!
    manager.getSessionInfoForNextMulticastMessage(sessionId, rebootFlag);
    EXPECT_EQ(1U, sessionId);
    EXPECT_FALSE(rebootFlag);
}

/**
 * Make sure sessionId and rebootFlag are affected correctly in case of unicast and rollover.
 */
TEST_F(SessionManagerTest, getSessionInfoForNextUnicastMessage_sessionId_when_rollover)
{
    ::ip::IPAddress addr = ::ip::make_ip4(20U);

    for (uint32_t i = 1U; i <= 0xFFFFU; ++i)
    {
        manager.getSessionInfoForNextUnicastMessage(addr, sessionId, rebootFlag);
        EXPECT_EQ(i, sessionId);
        EXPECT_TRUE(rebootFlag);
    }

    // next one will be a rollover!
    manager.getSessionInfoForNextUnicastMessage(addr, sessionId, rebootFlag);
    EXPECT_EQ(1U, sessionId);
    EXPECT_FALSE(rebootFlag);
}

/**
 * Make sure sessionId and rebootFlag are affected correctly in case of unicast and too many
 * endpoints.
 */
TEST_F(SessionManagerTest, getSessionInfoForNextUnicastMessage_when_too_many_endpoints)
{
    for (uint32_t i = 1U; i <= 16U; ++i)
    {
        ::ip::IPAddress addr = ::ip::make_ip4(i);
        manager.getSessionInfoForNextUnicastMessage(addr, sessionId, rebootFlag);
        EXPECT_EQ(1U, sessionId);
        EXPECT_TRUE(rebootFlag);
    }

    // no more space for endpoint!
    manager.getSessionInfoForNextUnicastMessage(::ip::make_ip4(17U), sessionId, rebootFlag);
    EXPECT_EQ(0U, sessionId);
    EXPECT_FALSE(rebootFlag);
}

} // anonymous namespace
