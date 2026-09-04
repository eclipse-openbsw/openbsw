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

#include "someip/IServiceAnnouncer.h"
#include "someip/ServiceDescription.h"

#include <ip/IPAddress.h>
#include <ip/IPEndpoint.h>

#include <gmock/gmock.h>

namespace someip
{
class ServiceAnnouncerMock : public IServiceAnnouncer
{
public:
    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());

    MOCK_METHOD(
        void,
        respondToFindService,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         minor_version::type minorVersion,
         ttl::type ttl,
         ::ip::IPAddress const& sourceIpAddress,
         bool unicast));

    MOCK_METHOD(
        void,
        respondToSubscribe,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         uint16_t reserved,
         eventgroup_id::type eventgroup,
         ttl::type ttl,
         ::ip::IPAddress const& sourceIpAddress,
         ::ip::IPAddress const& endpointIpAddress,
         uint16_t endpointPort,
         uint8_t endpointProto));

    MOCK_METHOD(
        void,
        sendSubscribeAck,
        (service_id::type serviceId,
         instance_id::type instanceId,
         eventgroup_id::type eventgroup,
         major_version::type majorVersion,
         uint16_t reserved,
         ttl::type ttl,
         ::ip::IPAddress const& sourceIpAddress));

    MOCK_METHOD(
        void,
        sendSubscribeNack,
        (service_id::type serviceId,
         instance_id::type instanceId,
         eventgroup_id::type eventgroup,
         major_version::type majorVersion,
         uint16_t reserved,
         ::ip::IPAddress const& sourceIpAddress));

    MOCK_METHOD(
        void,
        sendSubscribeAckMulticast,
        (service_id::type serviceId,
         instance_id::type instanceId,
         eventgroup_id::type eventgroup,
         major_version::type majorVersion,
         uint16_t reserved,
         ttl::type ttl,
         ::ip::IPAddress const& endpointAddress,
         uint16_t endpointPort,
         ::ip::IPAddress const& sourceIpAddress));

    MOCK_METHOD(
        void, subscribe, (ServiceDescription const& service, ::ip::IPAddress const& sourceAddress));
    MOCK_METHOD(
        void,
        unsubscribe,
        (ServiceDescription const& service, ::ip::IPAddress const& sourceAddress));
    MOCK_METHOD(void, find, (ServiceDescription const& service));
    MOCK_METHOD(void, offer, (ServiceDescription const& service));
    MOCK_METHOD(void, stopOffer, (ServiceDescription const& service));

    MOCK_METHOD(void, sendStopOffers, ());
};

} // namespace someip
