/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdEndpoint.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceQuery.h"
#include "someip/SubscribedEventGroup.h"
#include "someip/SubscriptionEndpoint.h"
#include "someip/SubscriptionManager.h"

#include <gtest/gtest.h>

namespace
{
/**
 * Make sure sizeof SdEndpoint == 20U, ServiceQuery == 120U, ServiceDescription == 36U,
 * SubscribedEventGroup == 16U, SubscriptionEndpoint == 40U and SubscriptionManager == 32U.
 */
TEST(SdSizeof, Sizeof)
{
#ifdef PLATFORM_SUPPORT_IPV6
    EXPECT_EQ(20U, sizeof(::someip::SdEndpoint));
    EXPECT_EQ(120U, sizeof(::someip::ServiceQuery));
    EXPECT_EQ(36U, sizeof(::someip::ServiceDescription));
    EXPECT_EQ(24U, sizeof(::someip::SubscribedEventGroup));
    EXPECT_EQ(40U, sizeof(::someip::SubscriptionEndpoint));
    EXPECT_EQ(32U, sizeof(::someip::SubscriptionManager));
#else
    EXPECT_EQ(8U, sizeof(::someip::SdEndpoint));
    EXPECT_EQ(80U, sizeof(::someip::ServiceQuery));
    EXPECT_EQ(24U, sizeof(::someip::ServiceDescription));
    EXPECT_EQ(24U, sizeof(::someip::SubscribedEventGroup));
    EXPECT_EQ(24U, sizeof(::someip::SubscriptionEndpoint));
    EXPECT_EQ(32U, sizeof(::someip::SubscriptionManager));
#endif
}

} // anonymous namespace
