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

#include "ServiceInfo.h"

#include <etl/intrusive_links.h>

namespace someip
{
struct ServiceDescription;

class IServiceListener : public ::etl::forward_link<0>
{
public:
    enum class ServiceStatus : uint8_t
    {
        SERVICE_AVAILABLE,
        SERVICE_UNAVAILABLE,
        SERVICE_RELIABLE,
        SERVICE_SUBSCRIPTION_NACK
    };

    virtual ~IServiceListener() = default;

    /**
     * Pure virtual function that enables updating the service status.
     */
    virtual void serviceStatusChanged(ServiceDescription const& service, ServiceStatus status) = 0;

    /**
     * Pure virtual function to give an IServiceListener the possibility to update the
     * ServiceDescription of an event group query. This is important in case of TCP client
     * connections where the local port needs to be updated. This update function will be called
     * after the serviceStatusChanged() function so that the IServiceListener has the chance to
     * compute a new local port and open the TCP connection. The updated event group
     * ServiceDescription will be reflected in the following subscribe packets.
     *
     * \param eventGroup ServiceDescription of the event group.
     * \param status Indicates if the parent service became available or unavailable.
     */
    virtual void updateEventgroupDescription(ServiceDescription&, ServiceStatus) = 0;

    /**
     * Pure virtual function that returns method detail.
     */
    virtual MethodDetail const* getMethodDetail(uint16_t methodId) const = 0;

    /**
     * Pure virtual function that returns event detail.
     */
    virtual EventDetail const* getEventDetail(uint16_t eventId) const = 0;
};

} // namespace someip
