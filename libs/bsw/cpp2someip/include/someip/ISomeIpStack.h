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

#include <cstdint>

namespace someip
{
struct ServiceQuery;
class ProvidedService;
class IEventListener;
} // namespace someip

namespace someip
{
/*
 * Interface for the SOME/IP stack.
 *
 * Contains methods to initialize the SD and RPC ports,
 * to register consumed or provided services and eventgroups
 * and to register event-listener at the stack.
 */
class ISomeIpStack
{
public:
    virtual ~ISomeIpStack() = default;

    /* CONFIG */

    virtual bool initSdPort(uint16_t port)  = 0;
    virtual bool initUdpPort(uint16_t port) = 0;
    virtual bool initTcpPort(uint16_t port) = 0;

    /* CLIENT */

    /**
     * Register a service query with:
     *
     * - serviceId,
     * - majorVersion, minorVersion
     * - instanceId (or INSTANCE_ID_ANY)
     * - service-listener
     *
     * Register an eventgroup query with:
     *
     * - serviceId,
     * - majorVersion, minorVersion
     * - instanceId,
     * - eventgroupId
     * - TTL
     * - localPort
     * - protocol
     *
     * \return true on success.
     */
    virtual bool registerServiceQuery(ServiceQuery& query) = 0;

    /**
     * Unregister a service or eventgroup query.
     */
    virtual void unregisterServiceQuery(ServiceQuery& query) = 0;

    /**
     * Register an event-listener.
     */
    virtual void addEventListener(IEventListener& listener) = 0;

    /**
     * Unregister an event-listener.
     */
    virtual void removeEventListener(IEventListener& listener) = 0;

    /* SERVER */

    /**
     * Register a provided service with:
     *
     * - serviceId,
     * - majorVersion, minorVersion
     * - instanceId
     * - TTL
     * - localPort
     * - protocol
     * - service-handler
     *
     * Register a provided eventgroup with:
     *
     * - serviceId,
     * - majorVersion, minorVersion
     * - instanceId,
     * - eventgroupId
     * - TTL
     * - [multicastIp] if multicast
     * - localPort
     * - service-handler
     *
     * \return true on success.
     */
    virtual bool registerProvidedService(ProvidedService& service) = 0;

    /**
     * Unregister a provided service or eventgroup.
     */
    virtual void unregisterProvidedService(ProvidedService& service) = 0;
};

} // namespace someip
