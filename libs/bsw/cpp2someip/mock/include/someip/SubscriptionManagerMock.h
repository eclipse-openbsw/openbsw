/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/ISubscriptionManager.h"

#include <gmock/gmock.h>

namespace someip
{
class SubscriptionManagerMock : public ISubscriptionManager
{
public:
    MOCK_METHOD(
        InternalSubscribeResult,
        addSubscription,
        (service_id::type serviceId,
         major_version::type majorVersion,
         instance_id::type instanceId,
         eventgroup_id::type eventgroup,
         ttl::type ttl,
         ::ip::IPAddress const& ipAddress,
         uint16_t port));

    MOCK_METHOD(
        void,
        removeSubscription,
        (service_id::type serviceId,
         major_version::type majorVersion,
         instance_id::type instanceId,
         eventgroup_id::type eventgroup,
         ::ip::IPAddress const& ipAddress,
         uint16_t port));

    MOCK_METHOD(
        void,
        removeSubscriptions,
        (service_id::type serviceId,
         major_version::type majorVersion,
         instance_id::type instanceId,
         eventgroup_id::type eventgroup));

    MOCK_METHOD(void, removeSubscriptions, (::ip::IPAddress const& ipAddress));

    MOCK_METHOD(void, updateTTLs, (uint32_t expiredSeconds));

    MOCK_METHOD(
        SubscriptionEndpointList*,
        getSubscriptions,
        (SubscribedEventGroup const& eventgroup),
        (const));

    MOCK_METHOD(uint16_t, getCurrentNumberOfSubscriptions, (), (const));
    MOCK_METHOD(uint16_t, getMaximumNumberOfSubscriptions, (), (const));
};

} // namespace someip
