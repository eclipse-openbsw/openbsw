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

namespace someip
{
template<>
ServiceDescription make<ServiceDescription>()
{
    return {
        minor_version::INVALID,
        ttl::INVALID,
        service_id::INVALID,
        instance_id::ANY,
        eventgroup_id::ALL,
#ifdef PLATFORM_SUPPORT_IPV6
        ::ip::make_ip6(0U),
#else
        ::ip::make_ip4(0U),
#endif
        port::INVALID,
        SomeIpConstants::INVALID_PROTO,
        major_version::INVALID};
}

bool matches(ServiceDescription const& lhs, ServiceDescription const& rhs)
{
    return (lhs.serviceId == rhs.serviceId) && (lhs.instanceId == rhs.instanceId)
           && (lhs.eventGroup == rhs.eventGroup) && (lhs.majorVersion == rhs.majorVersion);
}

bool isInstanceOf(ServiceDescription const& lhs, ServiceDescription const& rhs)
{
    return (lhs.serviceId == rhs.serviceId)
           && ((lhs.instanceId == rhs.instanceId) || (lhs.instanceId == instance_id::ANY)
               || (rhs.instanceId == instance_id::ANY))
           && (lhs.eventGroup == rhs.eventGroup)
           && ((lhs.majorVersion == rhs.majorVersion) || (lhs.majorVersion == major_version::ANY)
               || (rhs.majorVersion == major_version::ANY));
}

bool isEventgroupOfService(ServiceDescription const& lhs, ServiceDescription const& rhs)
{
    return ((containsEventGroup(lhs)) && (!containsEventGroup(rhs)))
           && (lhs.serviceId == rhs.serviceId)
           && ((lhs.instanceId == instance_id::ANY) || (rhs.instanceId == instance_id::ANY)
               || (lhs.instanceId == rhs.instanceId))
           && (lhs.majorVersion == rhs.majorVersion);
}

bool containsEventGroup(ServiceDescription const& desc)
{
    return (desc.eventGroup != eventgroup_id::ALL);
}

} // namespace someip
