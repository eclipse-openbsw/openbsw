/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RebootTracker.h"

#include <ip/IPAddress.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;
using namespace ::ip;

struct RebootTrackerTest : ::testing::Test
{
    RebootTrackerTest() { _rebootTracker.init(); }

    static uint8_t const MAX_ENDPOINTS = 5U;

    declare::RebootTracker<MAX_ENDPOINTS> _rebootTracker;

    ::ip::IPAddress source1 = make_ip4(192U, 0U, 2U, 0U);
    ::ip::IPAddress source2 = make_ip4(192U, 0U, 2U, 1U);

    ::ip::IPAddress dest1 = make_ip4(192U, 0U, 2U, 3U);
    ::ip::IPAddress dest2 = make_ip4(224U, 1U, 255U, 255U);
    ::ip::IPAddress dest3 = make_ip4(192U, 0U, 2U, 4U);
};

/**
 * Make sure no reboot occurs on creating new session.
 */
TEST_F(RebootTrackerTest, no_reboot_on_creating_session)
{
    SessionInfo session(source1, ::ip::isMulticastAddress(dest1), 1U, true);

    EXPECT_FALSE(_rebootTracker.evaluate(session));
}

/**
 * Make sure reboot occurs on sessionID reset for unicast.
 */
TEST_F(RebootTrackerTest, reboot_on_unicast_sessionId_reset)
{
    SessionInfo session(source1, ::ip::isMulticastAddress(dest1), 50U, true);

    EXPECT_FALSE(_rebootTracker.evaluate(session));
    _rebootTracker.apply(session);

    session.sessionId -= 1U;
    EXPECT_TRUE(_rebootTracker.evaluate(session));
}

/**
 * Make sure reboot occurs on sessionID reset for multicast.
 */
TEST_F(RebootTrackerTest, reboot_on_multicast_sessionId_reset)
{
    EXPECT_TRUE(::ip::isMulticastAddress(dest2));

    SessionInfo session(source1, ::ip::isMulticastAddress(dest2), 50U, true);

    EXPECT_FALSE(_rebootTracker.evaluate(session));
    _rebootTracker.apply(session);

    session.sessionId -= 1U;
    EXPECT_TRUE(_rebootTracker.evaluate(session));
}

/**
 * Make sure reboot occurs on rebootFlag reset for unicast.
 */
TEST_F(RebootTrackerTest, reboot_on_unicast_rebootFlag_reset)
{
    SessionInfo session(source1, ::ip::isMulticastAddress(dest1), 50U, true);

    EXPECT_FALSE(_rebootTracker.evaluate(session));
    _rebootTracker.apply(session);

    session.sessionId++;
    session.rebootFlag = false;
    EXPECT_FALSE(_rebootTracker.evaluate(session));
    _rebootTracker.apply(session);

    session.sessionId++;
    session.rebootFlag = true;
    EXPECT_TRUE(_rebootTracker.evaluate(session));
}

/**
 * Make sure reboot occurs on rebootFlag reset for multicast.
 */
TEST_F(RebootTrackerTest, reboot_on_multicast_rebootFlag_reset)
{
    EXPECT_TRUE(::ip::isMulticastAddress(dest2));

    SessionInfo session(source1, ::ip::isMulticastAddress(dest2), 50U, true);

    session.sessionId++;
    session.rebootFlag = false;
    EXPECT_FALSE(_rebootTracker.evaluate(session));
    _rebootTracker.apply(session);

    session.sessionId++;
    session.rebootFlag = true;
    EXPECT_TRUE(_rebootTracker.evaluate(session));
}

TEST_F(RebootTrackerTest, testTwoAlternatingUnicastClients)
{
    SessionInfo session1(source1, ::ip::isMulticastAddress(dest3), 1U, true);
    SessionInfo session2(source2, ::ip::isMulticastAddress(dest3), 1U, true);

    EXPECT_FALSE(_rebootTracker.evaluate(session1));
    _rebootTracker.apply(session1);

    EXPECT_FALSE(_rebootTracker.evaluate(session2));
    _rebootTracker.apply(session2);

    session1.sessionId++;
    EXPECT_FALSE(_rebootTracker.evaluate(session1));
    _rebootTracker.apply(session1);
    session2.sessionId++;
    EXPECT_FALSE(_rebootTracker.evaluate(session2));
    _rebootTracker.apply(session2);

    session1.sessionId = 1U;
    EXPECT_TRUE(_rebootTracker.evaluate(session1));
    session2.sessionId = 1U;
    EXPECT_TRUE(_rebootTracker.evaluate(session2));
}

TEST_F(RebootTrackerTest, testMaxNumberOfClients)
{
    uint16_t sessionId = 1U;

    for (size_t i = 0U; i < MAX_ENDPOINTS; ++i)
    {
        auto const source = make_ip4(192U, 0U, 2U, i + 1U);
        SessionInfo session(source, ::ip::isMulticastAddress(dest3), sessionId, true);
        EXPECT_FALSE(_rebootTracker.evaluate(session));
        _rebootTracker.apply(session);
    }

    sessionId++;
    for (size_t i = 0U; i < MAX_ENDPOINTS; ++i)
    {
        auto const source = make_ip4(192U, 0U, 2U, i + 1U);
        SessionInfo session(source, ::ip::isMulticastAddress(dest3), sessionId, true);
        EXPECT_FALSE(_rebootTracker.evaluate(session));
        _rebootTracker.apply(session);
    }

    sessionId = 1U;
    for (size_t i = 0U; i < MAX_ENDPOINTS; ++i)
    {
        auto const source = make_ip4(192U, 0U, 2U, i + 1U);
        SessionInfo session(source, ::ip::isMulticastAddress(dest3), sessionId, true);
        EXPECT_TRUE(_rebootTracker.evaluate(session));
        _rebootTracker.apply(session);
    }
}

TEST_F(RebootTrackerTest, testMoreThanMaxNumberOfClients)
{
    uint16_t sessionId = 1U;

    size_t const MAX_NUM_CLIENTS = MAX_ENDPOINTS + 1U;
    for (size_t i = 0U; i < MAX_NUM_CLIENTS; ++i)
    {
        auto const source = make_ip4(192U, 0U, 2U, i + 1U);
        SessionInfo session(source, ::ip::isMulticastAddress(dest3), sessionId, true);
        EXPECT_FALSE(_rebootTracker.evaluate(session));
        _rebootTracker.apply(session);
    }

    sessionId++;
    for (size_t i = 0U; i < MAX_NUM_CLIENTS; ++i)
    {
        auto const source = make_ip4(192U, 0U, 2U, i + 1U);
        SessionInfo session(source, ::ip::isMulticastAddress(dest3), sessionId, true);
        EXPECT_FALSE(_rebootTracker.evaluate(session));
        _rebootTracker.apply(session);
    }

    sessionId = 1U;
    for (size_t i = 0U; i < MAX_NUM_CLIENTS; ++i)
    {
        auto const source = make_ip4(192U, 0U, 2U, i + 1U);
        SessionInfo session(source, ::ip::isMulticastAddress(dest3), sessionId, true);
        bool reboot = _rebootTracker.evaluate(session);
        _rebootTracker.apply(session);
        if (MAX_ENDPOINTS == i)
        {
            // for the previously untracked client a reboot should not be reported
            EXPECT_FALSE(reboot);
        }
        else
        {
            // but for the tracked clients we have a reboot here
            EXPECT_TRUE(reboot);
        }
    }
}
} // anonymous namespace
