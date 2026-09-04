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

#include "someip/SomeIpConstants.h"
#include "someip/SubscriptionEndpoint.h"

namespace someip
{
class SubscribedEventGroup
{
public:
    SubscribedEventGroup();
    SubscribedEventGroup(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup);

    SubscribedEventGroup(SubscribedEventGroup const& rhs)            = delete;
    SubscribedEventGroup(SubscribedEventGroup const&& rhs)           = delete;
    SubscribedEventGroup& operator=(SubscribedEventGroup&& rhs)      = delete;
    SubscribedEventGroup& operator=(SubscribedEventGroup const& rhs) = delete;

    bool operator==(SubscribedEventGroup const& rhs) const;

    void clear();

    SubscriptionEndpointList& getEndpoints();

    service_id::type getServiceId() const;
    instance_id::type getInstanceId() const;
    eventgroup_id::type getEventgroupId() const;
    major_version::type getMajorVersion() const;

private:
    SubscriptionEndpointList _endpoints;

    service_id::type _serviceId;
    instance_id::type _instanceId;
    eventgroup_id::type _eventgroup;
    major_version::type _majorVersion;
};

/*
 * inline implementation
 */
inline SubscriptionEndpointList& SubscribedEventGroup::getEndpoints() { return _endpoints; }

inline service_id::type SubscribedEventGroup::getServiceId() const { return _serviceId; }

inline instance_id::type SubscribedEventGroup::getInstanceId() const { return _instanceId; }

inline eventgroup_id::type SubscribedEventGroup::getEventgroupId() const { return _eventgroup; }

inline major_version::type SubscribedEventGroup::getMajorVersion() const { return _majorVersion; }

} // namespace someip
