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

#include "someip/IEventSender.h"
#include "someip/ISomeIpSerializable.h"

#include <gmock/gmock.h>

namespace someip
{
class EventSenderMock : public IEventSender
{
public:
    MOCK_METHOD(
        ErrorCode,
        sendEvent,
        (service_id::type serviceId,
         major_version::type majorVersion,
         uint16_t eventId,
         uint16_t maximumDelayTime,
         ISomeIpSerializable const* payload,
         uint16_t sourcePort,
         uint8_t proto,
         ::ip::IPEndpoint const& destinationEndpoint,
         uint16_t sessionId),
        (override));

    MOCK_METHOD(
        ErrorCode,
        sendEvent,
        (service_id::type serviceId,
         major_version::type majorVersion,
         instance_id::type instanceId,
         uint16_t eventId,
         ::etl::span<uint16_t const> eventGroupIds,
         uint16_t maximumDelayTime,
         ISomeIpSerializable const* payload,
         uint16_t sourcePort,
         uint8_t proto,
         uint16_t sessionId),
        (override));

    ErrorCode sendMulticastEvent(
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
        uint16_t sessionId) override
    {
        sendMulticastEvent_0_10(
            serviceId,
            majorVersion,
            instanceId,
            eventId,
            eventGroupIds,
            maximumDelayTime,
            payload,
            sourcePort,
            proto,
            destinationEndpoint);
        sendMulticastEvent_11(sessionId);
        return ErrorCode::EVENT_SEND_OK;
    }

    MOCK_METHOD(
        void,
        sendMulticastEvent_0_10,
        (service_id::type serviceId,
         major_version::type majorVersion,
         instance_id::type instanceId,
         uint16_t eventId,
         ::etl::span<uint16_t const> eventGroupIds,
         uint16_t maximumDelayTime,
         ISomeIpSerializable const* payload,
         uint16_t sourcePort,
         uint8_t proto,
         ::ip::IPEndpoint const& destinationEndpoint));

    MOCK_METHOD(void, sendMulticastEvent_11, (uint16_t));
};

} // namespace someip
