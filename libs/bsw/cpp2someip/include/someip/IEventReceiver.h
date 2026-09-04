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

#include "someip/IEventListener.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpParser.h"
#include <ip/IPEndpoint.h>

#include <cstdint>

namespace someip
{
class IEventReceiver
{
public:
    IEventReceiver()                                 = default;
    IEventReceiver(IEventReceiver const&)            = delete;
    IEventReceiver& operator=(IEventReceiver const&) = delete;

    /**
     * Passes payload for an incoming SOME/IP event/notification to every
     * IServiceEventListener added to this receiver.
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

    /**
     * Adds IEventListener from IEventListener List
     *
     * \param listener IEventListener that has to be added
     */
    virtual void addEventListener(IEventListener& listener)    = 0;
    /**
     * Removes IEventListener from IEventListener List
     *
     * \param listener IEventListener that has to be removed
     */
    virtual void removeEventListener(IEventListener& listener) = 0;
};

} // namespace someip
