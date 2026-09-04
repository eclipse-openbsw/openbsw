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
#include <cstdint>

namespace ip
{
struct IPAddress;
}

namespace someip
{
struct ServiceDescription;

class IServiceAnnouncer
{
public:
    virtual ~IServiceAnnouncer() = default;

    /**
     * Pure virtual lifecycle function that starts ServiceAnnouncer.
     */
    virtual void start() = 0;

    /**
     * Pure virtual lifecycle function that stops ServiceAnnouncer.
     */
    virtual void stop() = 0;

    /**
     * Pure virtual lifecycle function that responds to a request to
     * find a specific service.
     */
    virtual void respondToFindService(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        minor_version::type minorVersion,
        ttl::type ttl,
        ::ip::IPAddress const& sourceIpAddress,
        bool unicast)
        = 0;

    /**
     * Pure virtual function that tries to add a subscription
     * to a subscribers list.
     */
    virtual void respondToSubscribe(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        uint16_t reserved,
        eventgroup_id::type eventgroup,
        ttl::type ttl,
        ::ip::IPAddress const& sourceIpAddress,
        ::ip::IPAddress const& endpointIpAddress,
        uint16_t endpointPort,
        uint8_t endpointProto)
        = 0;

    /**
     * Pure virtual function that handles sending the subscribe
     * Ack message.
     */
    virtual void sendSubscribeAck(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup,
        major_version::type majorVersion,
        uint16_t reserved,
        ttl::type ttl,
        ::ip::IPAddress const& sourceIpAddress)
        = 0;

    /**
     * Pure virtual function that handles sending the subscribe
     * Nack message.
     */
    virtual void sendSubscribeNack(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup,
        major_version::type majorVersion,
        uint16_t reserved,
        ::ip::IPAddress const& sourceIpAddress)
        = 0;

    /**
     * Pure virtual function that handles sending the subscribe
     * Ack message over multicast IP address.
     */
    virtual void sendSubscribeAckMulticast(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup,
        major_version::type majorVersion,
        uint16_t reserved,
        ttl::type ttl,
        ::ip::IPAddress const& endpointAddress,
        uint16_t endpointPort,
        ::ip::IPAddress const& sourceIpAddress)
        = 0;

    /**
     * Pure virtual function that handles the subscribe.
     */
    virtual void subscribe(ServiceDescription const& service, ::ip::IPAddress const& sourceAddress)
        = 0;

    /**
     * Pure virtual function that handles the unsubscribe.
     */
    virtual void
    unsubscribe(ServiceDescription const& service, ::ip::IPAddress const& sourceAddress)
        = 0;

    /**
     * Pure virtual function that handles finding a service.
     */
    virtual void find(ServiceDescription const& service) = 0;

    /**
     * Pure virtual function that handles offering a service.
     */
    virtual void offer(ServiceDescription const& service) = 0;

    /**
     * Pure virtual function that handles stopping offering a service.
     */
    virtual void stopOffer(ServiceDescription const& service) = 0;

    /**
     * Pure virtual function that triggers sending message to stop offer.
     */
    virtual void sendStopOffers() = 0;
};

} // namespace someip
