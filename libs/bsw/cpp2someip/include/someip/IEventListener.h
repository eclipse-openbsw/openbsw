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
#include <etl/intrusive_links.h>
#include <cstdint>

namespace someip
{
class SomeIpParser;

class IEventListener : public ::etl::forward_link<0>
{
protected:
    IEventListener() = default;

public:
    IEventListener(IEventListener const&)            = delete;
    IEventListener& operator=(IEventListener const&) = delete;

    /**
     * Passes payload for an incoming SOME/IP event/notification.
     *
     * \param serviceId ServiceID of the event/notification
     * \param eventId EventID of the event/notification
     * \param instanceId InstanceID of the event/notification
     * \param majorVersion majorVersion of the event/notification
     * \param parser Payload of the event/notification
     */
    virtual void eventReceived(
        service_id::type serviceId,
        uint16_t eventId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        SomeIpParser& parser)
        = 0;
};
} // namespace someip
