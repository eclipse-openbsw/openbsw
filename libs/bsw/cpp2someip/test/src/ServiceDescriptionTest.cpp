/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceDescription.h"

#include "someip/SomeIpConstants.h"
#include "someip/init.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
using namespace ::testing;
using ::someip::ServiceDescription;

struct ServiceDescriptionTest
: ::testing::Test
, ServiceDescription
{
    ServiceDescription a{};
    ServiceDescription b{};

    ServiceDescriptionTest() : ServiceDescription()
    {
        a              = ::someip::make<ServiceDescription>();
        a.serviceId    = 10U;
        a.instanceId   = 11U;
        a.eventGroup   = 12U;
        a.majorVersion = 13U;

        b = a;
    }
};

/**
 * Test whether matches() recognizes matching and not matching combinations of instanceID,
 * serviceId, eventGroup and majorVersion correctly.
 */
TEST_F(ServiceDescriptionTest, test_matches)
{
    using ::someip::matches;

    EXPECT_TRUE(matches(a, b));

    b.majorVersion = a.majorVersion - 1U;
    EXPECT_FALSE(matches(a, b));

    b.majorVersion = a.majorVersion;
    b.eventGroup   = a.eventGroup - 1U;
    EXPECT_FALSE(matches(a, b));

    b.eventGroup = a.eventGroup;
    b.instanceId = a.instanceId - 1U;
    EXPECT_FALSE(matches(a, b));

    b.instanceId = a.instanceId;
    b.serviceId  = a.serviceId - 1U;
    EXPECT_FALSE(matches(a, b));
}

/**
 * Test whether isInstanceOf() recognizes desired combinations of instanceID,
 * serviceId, eventGroup and majorVersion correctly. Different minorVersions should not affect the
 * result.
 */
TEST_F(ServiceDescriptionTest, test_isInstanceOf)
{
    using ::someip::isInstanceOf;

    EXPECT_TRUE(isInstanceOf(a, b));

    a.minorVersion = 14U;
    b.minorVersion = 15U;
    EXPECT_TRUE(isInstanceOf(a, b));

    b.majorVersion = a.majorVersion + 1U;
    EXPECT_FALSE(isInstanceOf(a, b));

    b.majorVersion = a.majorVersion;
    b.eventGroup   = a.eventGroup + 1U;
    EXPECT_FALSE(isInstanceOf(a, b));

    b.eventGroup = a.eventGroup;
    b.instanceId = a.instanceId + 1U;
    EXPECT_FALSE(isInstanceOf(a, b));

    b.instanceId = a.instanceId;
    b.serviceId  = a.serviceId + 1U;
    EXPECT_FALSE(isInstanceOf(a, b));
}

/**
 * Make sure instanceOf() handles INSTANCE_ID_ANY correctly.
 */
TEST_F(ServiceDescriptionTest, test_isInstanceOf_INSTANCE_ID_ANY)
{
    using ::someip::isInstanceOf;

    a.minorVersion = 14U;
    b.minorVersion = 14U;

    a.instanceId = static_cast<uint16_t>(::someip::instance_id::ANY);
    EXPECT_TRUE(isInstanceOf(a, b));

    b.instanceId = static_cast<uint16_t>(::someip::instance_id::ANY);
    EXPECT_TRUE(isInstanceOf(a, b));
}

/**
 * Test whether isEventgroupOfService() recognizes desired combinations of instanceID,
 * serviceId, eventGroup and majorVersion correctly.
 */
TEST_F(ServiceDescriptionTest, test_IsEventgroupOfService)
{
    using ::someip::isEventgroupOfService;

    b.eventGroup = ::someip::eventgroup_id::ALL;
    EXPECT_TRUE(isEventgroupOfService(a, b));

    b.majorVersion = a.majorVersion - 1U;
    EXPECT_FALSE(isEventgroupOfService(a, b));

    b.majorVersion = a.majorVersion;
    b.instanceId   = a.instanceId - 1U;
    EXPECT_FALSE(isEventgroupOfService(a, b));

    b.instanceId = a.instanceId;
    b.serviceId  = a.serviceId - 1U;
    EXPECT_FALSE(isEventgroupOfService(a, b));

    b.serviceId  = a.serviceId;
    b.eventGroup = a.eventGroup - 1U;
    EXPECT_FALSE(isEventgroupOfService(a, b));
}

/**
 * Make sure isEventgroupOfService() is only successful if the second ServiceDescription (and only
 * the second one) has ::someip::eventgroup_id::ALL.
 */
TEST_F(ServiceDescriptionTest, test_isEventgroupOfService_ALL_EVENTGROUPS)
{
    using ::someip::isEventgroupOfService;

    a.eventGroup = ::someip::eventgroup_id::ALL;
    b.eventGroup = ::someip::eventgroup_id::ALL;

    EXPECT_FALSE(isEventgroupOfService(a, b));

    a.eventGroup = 10U;
    EXPECT_TRUE(isEventgroupOfService(a, b));

    b.eventGroup = 10U;
    EXPECT_FALSE(isEventgroupOfService(a, b));
}

/**
 * Make sure isEventgroupOfService() is successful if one or both ServiceDescriptions contain
 * INSTANCE_ID_ANY.
 */
TEST_F(ServiceDescriptionTest, test_isEventgroupOfService_INSTANCE_ID_ANY)
{
    using ::someip::isEventgroupOfService;

    b.eventGroup = ::someip::eventgroup_id::ALL;

    b.instanceId = a.instanceId - 6U;
    EXPECT_FALSE(isEventgroupOfService(a, b));

    b.instanceId = a.instanceId;
    b.instanceId = static_cast<uint16_t>(::someip::instance_id::ANY);
    EXPECT_TRUE(isEventgroupOfService(a, b));

    a.instanceId = static_cast<uint16_t>(::someip::instance_id::ANY);
    EXPECT_TRUE(isEventgroupOfService(a, b));
}

/**
 * Make sure ServiceDescription is assigned correctly.
 */
TEST(ServiceDescription, test_ServiceDescription_assignment)
{
    auto a         = ::someip::make<ServiceDescription>();
    a.serviceId    = 10U;
    a.instanceId   = 11U;
    a.eventGroup   = 12U;
    a.majorVersion = 13U;
    a.minorVersion = 17U;
    a.ttl          = 14U;
    a.ipAddress    = ::ip::make_ip4(192U, 0U, 2U, 10U);
    a.port         = 15U;
    a.proto        = 16U;

    ServiceDescription b = a;
    EXPECT_EQ(10U, b.serviceId);
    EXPECT_EQ(11U, b.instanceId);
    EXPECT_EQ(12U, b.eventGroup);
    EXPECT_EQ(13U, b.majorVersion);
    EXPECT_EQ(17U, b.minorVersion);
    EXPECT_EQ(14U, b.ttl);
    EXPECT_EQ(0xC000020A, ::ip::ip4_to_u32(b.ipAddress));
    EXPECT_EQ(15U, b.port);
    EXPECT_EQ(16U, b.proto);

    ServiceDescription& other = b;
    b                         = other;
    EXPECT_EQ(10U, b.serviceId);
    EXPECT_EQ(11U, b.instanceId);
    EXPECT_EQ(12U, b.eventGroup);
    EXPECT_EQ(13U, b.majorVersion);
    EXPECT_EQ(17U, b.minorVersion);
    EXPECT_EQ(14U, b.ttl);
    EXPECT_EQ(0xC000020A, ::ip::ip4_to_u32(b.ipAddress));
    EXPECT_EQ(15U, b.port);
    EXPECT_EQ(16U, b.proto);
}
} // anonymous namespace
