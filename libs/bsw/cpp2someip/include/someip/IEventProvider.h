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
/**
 * Interface for generated event provider.
 */
class IEventProvider
{
protected:
    IEventProvider() = default;

public:
    IEventProvider(IEventProvider const&)            = delete;
    IEventProvider& operator=(IEventProvider const&) = delete;

    virtual ~IEventProvider() = default;

    /**
     * Notify the initial events for given subscriber to an event-group
     * \note The default initial events are the field-notifier within this event-group.
     */
    virtual bool notifyInitialEvents(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        eventgroup_id::type eventGroup,
        ::ip::IPAddress const& ipAddress,
        port::type port,
        proto::type proto)
        = 0;
};

} // namespace someip
