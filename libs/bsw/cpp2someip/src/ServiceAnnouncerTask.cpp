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

namespace someip
{
ServiceAnnouncerTask::ServiceAnnouncerTask()
: ::etl::forward_link<0>()
, _timestamp()
, _endpoint()
, _destination()
, _ttl()
, _minorVersion()
, _serviceId()
, _instanceId()
, _eventgroup()
, _majorVersion()
, _proto()
, _type()
, _unicast()
{
    clear();
}

void ServiceAnnouncerTask::clear()
{
    _timestamp = 0U;
    _endpoint.clear();
#ifdef PLATFORM_SUPPORT_IPV6
    _destination = ::ip::make_ip6(0U);
#else
    _destination = ::ip::make_ip4(0U);
#endif
    _minorVersion = minor_version::INVALID;
    _ttl          = ttl::INVALID;
    _serviceId    = service_id::INVALID;
    _instanceId   = instance_id::ANY;
    _eventgroup   = eventgroup_id::ALL;
    _majorVersion = major_version::INVALID;
    _proto        = SomeIpConstants::INVALID_PROTO;
    _type         = TaskType::TASK_IDLE;
    _unicast      = false;
}

ServiceAnnouncerTask& ServiceAnnouncerTask::operator=(ServiceAnnouncerTask const& rhs)
{
    if (this != &rhs)
    {
        _timestamp    = rhs._timestamp;
        _endpoint     = rhs._endpoint;
        _destination  = rhs._destination;
        _minorVersion = rhs._minorVersion;
        _ttl          = rhs._ttl;
        _serviceId    = rhs._serviceId;
        _instanceId   = rhs._instanceId;
        _eventgroup   = rhs._eventgroup;
        _majorVersion = rhs._majorVersion;
        _proto        = rhs._proto;
        _type         = rhs._type;
        _unicast      = rhs._unicast;
    }

    return *this;
}

void ServiceAnnouncerTask::init(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroup,
    ttl::type const ttl,
    major_version::type const majorVersion,
    minor_version::type const minorVersion)
{
    _serviceId    = serviceId;
    _instanceId   = instanceId;
    _eventgroup   = eventgroup;
    _ttl          = ttl;
    _majorVersion = majorVersion;
    _minorVersion = minorVersion;
}

void ServiceAnnouncerTask::setEndpoint(::ip::IPAddress const& address, uint16_t const port)
{
    _endpoint.setAddress(address);
    _endpoint.setPort(port);
}

void ServiceAnnouncerTask::initFrom(
    ServiceDescription const& service,
    ::ip::IPAddress const& address,
    bool const unicast,
    uint64_t const timestamp,
    TaskType const type)
{
    _serviceId    = service.serviceId;
    _instanceId   = service.instanceId;
    _eventgroup   = service.eventGroup;
    _ttl          = service.ttl;
    _majorVersion = service.majorVersion;
    _minorVersion = service.minorVersion;
    _proto        = service.proto;
    _destination  = address;
    _endpoint.setAddress(service.ipAddress);
    _endpoint.setPort(service.port);
    _unicast   = unicast;
    _timestamp = timestamp;
    _type      = type;
}

void ServiceAnnouncerTask::copyTo(ServiceDescription& service) const
{
    service.serviceId    = _serviceId;
    service.instanceId   = _instanceId;
    service.eventGroup   = _eventgroup;
    service.ttl          = _ttl;
    service.majorVersion = _majorVersion;
    service.minorVersion = _minorVersion;
    service.ipAddress    = _endpoint.getAddress();
    service.port         = _endpoint.getPort();
    service.proto        = _proto;
}

bool ServiceAnnouncerTask::isSame(ServiceDescription const& service) const
{
    bool const majorVersionOk
        = ((_majorVersion == major_version::ANY) || (_majorVersion == service.majorVersion));
    bool const minorVersionOk
        = ((_minorVersion == minor_version::ANY) || (_minorVersion == service.minorVersion));

    return (
        (_serviceId == service.serviceId)
        && ((_instanceId == service.instanceId) || (_instanceId == instance_id::ANY))
        && majorVersionOk && minorVersionOk);
}

bool ServiceAnnouncerTask::containsEventGroup() const
{
    return (_eventgroup != eventgroup_id::ALL);
}

} // namespace someip
