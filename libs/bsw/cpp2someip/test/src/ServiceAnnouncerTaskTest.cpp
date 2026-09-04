/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceAnnouncerTask.h"

#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"
#include "someip/init.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

/**
 * Make sure isSame() detects correctly whether combination of serviceId, instanceId,
 * minorVersion and MajorVersion of service and task match.
 */
TEST(ServiceAnnouncerTask, test_IsSame)
{
    ServiceAnnouncerTask task;
    task.init(10U, 11U, 12U, 13U, 14U, 15U);
    auto service = ::someip::make<ServiceDescription>();

    service.serviceId = 9U;
    EXPECT_FALSE(task.isSame(service));

    service.serviceId = 10U;
    EXPECT_FALSE(task.isSame(service));

    service.instanceId = 11U;
    EXPECT_FALSE(task.isSame(service));

    service.majorVersion = 14U;
    service.minorVersion = 15U;
    EXPECT_TRUE(task.isSame(service));

    service.instanceId = 10U;
    EXPECT_FALSE(task.isSame(service));
    task.init(10U, ::someip::instance_id::ANY, 12U, 13U, 14U, 15U);
    EXPECT_TRUE(task.isSame(service));

    service.majorVersion = 13U;
    service.minorVersion = 15U;
    service.instanceId   = 11U;
    EXPECT_FALSE(task.isSame(service));
    task.init(10U, 11U, 12U, 13U, major_version::ANY, 15U);
    EXPECT_TRUE(task.isSame(service));

    service.majorVersion = 14U;
    service.minorVersion = 13U;
    service.instanceId   = 11U;
    EXPECT_FALSE(task.isSame(service));
    task.init(10U, 11U, 12U, 13U, 14U, minor_version::ANY);
    EXPECT_TRUE(task.isSame(service));

    service.majorVersion = 13U;
    service.minorVersion = 13U;
    service.instanceId   = 11U;
    EXPECT_FALSE(task.isSame(service));
    task.init(10U, 11U, 12U, 13U, major_version::ANY, minor_version::ANY);
    EXPECT_TRUE(task.isSame(service));
}

/**
 * Make sure timestamp of ServiceAnnouncerTask is set and given back consistently.
 */
TEST(ServiceAnnouncerTask, set_and_get_timestamp)
{
    ServiceAnnouncerTask task;
    task.setTimestamp(13U);
    EXPECT_EQ(13U, task.getTimestamp());
}

/**
 * Make sure ServiceAnnouncerTask is assigned correctly.
 */
TEST(ServiceAnnouncerTask, test_ServiceAnnouncerTask_assignment)
{
    ServiceAnnouncerTask task1;
    ServiceAnnouncerTask task2;

    task1.init(10U, 11U, 12U, 13U, 14U, 15U);
    task2.init(11U, 12U, 13U, 14U, 15U, 16U);

    auto service = ::someip::make<ServiceDescription>();
    task1.copyTo(service);

    ServiceAnnouncerTask* other = &task1;
    task1                       = *other;
    EXPECT_EQ(10U, service.serviceId);
    EXPECT_EQ(11U, service.instanceId);

    task2.copyTo(service);
    task1 = task2;
    EXPECT_EQ(11U, service.serviceId);
    EXPECT_EQ(12U, service.instanceId);
}
} // anonymous namespace
