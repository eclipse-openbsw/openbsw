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

#include "someip/ISomeIpSerializable.h"
#include "someip/SomeIpConstants.h"
#include <ip/IPEndpoint.h>

#include <etl/span.h>
#include <cstdint>

namespace someip
{
/**
 * Interface for an event-sender.
 *
 * Note: For an event without payload the corresponding ISomeIpSerializable pointer should be 0L.
 */
class IEventSender
{
public:
    IEventSender()                               = default;
    IEventSender(IEventSender const&)            = delete;
    IEventSender& operator=(IEventSender const&) = delete;

    virtual ~IEventSender() = default;

    enum class ErrorCode : uint8_t
    {
        /** event was sent */
        EVENT_SEND_OK,
        /** error happened during sending */
        EVENT_SEND_ERROR,
        /** not enough memory to send the event */
        EVENT_SEND_OUT_OF_MEMORY,

        /** internal codes **/
        EVENT_SEND_PENDING
    };

    /** send unicast to specific address */
    virtual ErrorCode sendEvent(
        service_id::type serviceId,
        major_version::type majorVersion,
        uint16_t eventId,
        uint16_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        uint16_t sourcePort,
        uint8_t proto,
        ::ip::IPEndpoint const& destinationEndpoint,
        uint16_t sessionId = 0U)
        = 0;

    /** send unicast to subscribers */
    virtual ErrorCode sendEvent(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        uint16_t eventId,
        ::etl::span<uint16_t const> eventGroupIds,
        uint16_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        uint16_t sourcePort,
        uint8_t proto,
        uint16_t sessionId = 0U)
        = 0;

    /** send multicast to subscribers */
    virtual ErrorCode sendMulticastEvent(
        service_id::type serviceId,
        major_version::type majorVersion,
        instance_id::type instanceId,
        uint16_t eventId,
        ::etl::span<uint16_t const> eventGroupIds,
        uint16_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        uint16_t sourcePort,
        uint8_t proto,
        ::ip::IPEndpoint const& destinationEndpoint,
        uint16_t sessionId = 0U)
        = 0;
};

} // namespace someip
