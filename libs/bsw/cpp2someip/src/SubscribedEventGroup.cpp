/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SubscribedEventGroup.h"

#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"

namespace someip
{
SubscribedEventGroup::SubscribedEventGroup()
: _endpoints()
, _serviceId(service_id::INVALID)
, _instanceId(instance_id::ANY)
, _eventgroup(eventgroup_id::ALL)
, _majorVersion(major_version::INVALID)
{}

SubscribedEventGroup::SubscribedEventGroup(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroup)
: _endpoints()
, _serviceId(serviceId)
, _instanceId(instanceId)
, _eventgroup(eventgroup)
, _majorVersion(majorVersion)
{}

bool SubscribedEventGroup::operator==(SubscribedEventGroup const& rhs) const
{
    return (
        (rhs._serviceId == _serviceId) && (rhs._majorVersion == _majorVersion)
        && (rhs._instanceId == _instanceId) && (rhs._eventgroup == _eventgroup));
}

void SubscribedEventGroup::clear()
{
    _endpoints.clear();
    _serviceId    = service_id::INVALID;
    _majorVersion = major_version::INVALID;
    _instanceId   = instance_id::ANY;
    _eventgroup   = eventgroup_id::ALL;
}

} // namespace someip
