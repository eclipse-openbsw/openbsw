/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/EventTransceiver.h"

#include "someip/BufferedEventSender.h"
#include "someip/ISubscriptionManager.h"
#include "someip/SomeIpParser.h"
#include "someip/SubscribedEventGroup.h"

#include <etl/algorithm.h>

namespace someip
{
EventTransceiver::EventTransceiver(
    BufferedEventSender& eventSender,
    ISubscriptionManager& subscriptionManager,
    ::etl::ivector<SubscriptionEndpoint>& subscriptionEndpoints)
: _eventSender(eventSender)
, _subscriptionManager(subscriptionManager)
, _eventListeners()
, _subscriptionEndpoints(subscriptionEndpoints)
{}

void EventTransceiver::shutdown() { _eventListeners.clear(); }

// virtual
IEventSender::ErrorCode EventTransceiver::sendEvent(
    service_id::type serviceId,
    major_version::type majorVersion,
    uint16_t eventId,
    uint16_t maximumDelayTime,
    ISomeIpSerializable const* payload,
    port::type sourcePort,
    proto::type proto,
    ::ip::IPEndpoint const& destinationEndpoint,
    uint16_t sessionId)
{
    return _eventSender.sendEvent(
        serviceId,
        majorVersion,
        eventId,
        static_cast<uint32_t>(maximumDelayTime),
        payload,
        sourcePort,
        proto,
        destinationEndpoint,
        sessionId);
}

// virtual
IEventSender::ErrorCode EventTransceiver::sendEvent(
    service_id::type serviceId,
    major_version::type majorVersion,
    instance_id::type instanceId,
    uint16_t eventId,
    ::etl::span<eventgroup_id::type const> eventGroupIds,
    uint16_t maximumDelayTime,
    ISomeIpSerializable const* payload,
    port::type sourcePort,
    proto::type proto,
    uint16_t sessionId)
{
    _subscriptionEndpoints.clear();

    for (auto const eventGroupId : eventGroupIds)
    {
        SubscribedEventGroup const eventgroup(serviceId, majorVersion, instanceId, eventGroupId);
        SubscriptionEndpointList* const endpoints
            = _subscriptionManager.getSubscriptions(eventgroup);
        if (endpoints == nullptr)
        {
            continue;
        }

        for (auto const& endpoint : *endpoints)
        {
            if (etl::find(_subscriptionEndpoints.begin(), _subscriptionEndpoints.end(), endpoint)
                == _subscriptionEndpoints.end())
            {
                _subscriptionEndpoints.push_back(endpoint);
            }
        }
    }

    for (auto const& unique_endpoint : _subscriptionEndpoints)
    {
        (void)_eventSender.sendEvent(
            serviceId,
            majorVersion,
            eventId,
            static_cast<uint32_t>(maximumDelayTime),
            payload,
            sourcePort,
            proto,
            unique_endpoint,
            sessionId);
    }

    return IEventSender::ErrorCode::EVENT_SEND_OK;
}

// virtual
IEventSender::ErrorCode EventTransceiver::sendMulticastEvent(
    service_id::type serviceId,
    major_version::type majorVersion,
    instance_id::type instanceId,
    uint16_t eventId,
    ::etl::span<eventgroup_id::type const> eventGroupIds,
    uint16_t maximumDelayTime,
    ISomeIpSerializable const* payload,
    port::type sourcePort,
    proto::type proto,
    ::ip::IPEndpoint const& destinationEndpoint,
    uint16_t sessionId)
{
    bool hasSubscriber = false;

    for (auto const eventGroupId : eventGroupIds)
    {
        SubscribedEventGroup const eventgroup(serviceId, majorVersion, instanceId, eventGroupId);
        SubscriptionEndpointList* const endpoints
            = _subscriptionManager.getSubscriptions(eventgroup);
        if (endpoints == nullptr)
        {
            continue;
        }

        if (!endpoints->empty())
        {
            hasSubscriber = true;
            break;
        }
    }

    IEventSender::ErrorCode result = IEventSender::ErrorCode::EVENT_SEND_OK;

    if (hasSubscriber)
    {
        result = _eventSender.sendEvent(
            serviceId,
            majorVersion,
            eventId,
            static_cast<uint32_t>(maximumDelayTime),
            payload,
            sourcePort,
            proto,
            destinationEndpoint,
            sessionId);
    }

    return result;
}

// virtual
void EventTransceiver::eventReceived(
    service_id::type serviceId,
    uint16_t eventId,
    instance_id::type instanceId,
    major_version::type majorVersion,
    SomeIpParser& parser)
{
    auto iter          = _eventListeners.begin();
    auto const endIter = _eventListeners.end();

    for (; iter != endIter; ++iter)
    {
        parser.resetCurrentPosition();
        iter->eventReceived(serviceId, eventId, instanceId, majorVersion, parser);
    }
}

// virtual
void EventTransceiver::addEventListener(IEventListener& listener)
{
    _eventListeners.push_front(listener);
}

// virtual
void EventTransceiver::removeEventListener(IEventListener& listener)
{
    _eventListeners.remove_if([&listener](IEventListener const& l) { return &l == &listener; });
}

} // namespace someip
