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
#include <ip/IPAddress.h>

#include <cstdint>

namespace someip
{
/*
 * Note: This structure is not properly aligned and thus
 * "Composite type has padding after field 'majorVersion'" warning pops up on
 * verification. The plan is to get rid of the necessity to have this entire struct.
 * Otherwise creating an artificial padding could lead to more confusion and "unused"
 * member fields.
 */
struct ServiceDescription
{
    minor_version::type minorVersion;
    ::someip::ttl::type ttl;
    service_id::type serviceId;
    instance_id::type instanceId;
    eventgroup_id::type eventGroup;
    ::ip::IPAddress ipAddress;
    uint16_t port;
    uint8_t proto;
    major_version::type majorVersion;
};

bool matches(ServiceDescription const&, ServiceDescription const&);
bool isInstanceOf(ServiceDescription const&, ServiceDescription const&);
bool isEventgroupOfService(ServiceDescription const&, ServiceDescription const&);
bool containsEventGroup(ServiceDescription const&);

/*
 * inline implementation
 */

bool operator==(ServiceDescription const& lhs, ServiceDescription const& rhs) = delete;

/**
 * This method will initialize this ServiceDescription instance by
 * copying the SERVICE_ID, MAJOR_VERSION, and MINOR_VERSION
 * enum values from the 'Service' template parameter.
 *
 * Usage:
 *   ServiceDescription service;
 *   someip::initFrom<ServiceClass>(service);
 */
template<class Service>
inline void initFrom(ServiceDescription& desc)
{
    desc.serviceId    = Service::SERVICE_ID;
    desc.majorVersion = Service::MAJOR_VERSION;
    desc.minorVersion = Service::MINOR_VERSION;
}

} // namespace someip
