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

#include "someip/BufferedEventSender.h"
#include "someip/IEventListener.h"
#include "someip/IEventReceiver.h"
#include "someip/IEventSender.h"
#include "someip/SubscriptionEndpoint.h"

#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <cstdint>

namespace someip
{
class ISubscriptionManager;

class EventTransceiver
: public IEventSender
, public IEventReceiver
{
public:
    EventTransceiver(
        BufferedEventSender& eventSender,
        ISubscriptionManager& subscriptionManager,
        ::etl::ivector<SubscriptionEndpoint>& subscriptionEndpoints);

    /* Clears event listeners list. */
    void shutdown();

    /* Sends event to the given unicast destinationEndpoint. */
    IEventSender::ErrorCode sendEvent(
        service_id::type,
        major_version::type,
        uint16_t eventId,
        uint16_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        port::type, // source port
        proto::type,
        ::ip::IPEndpoint const& destinationEndpoint,
        uint16_t sessionId = 0U) override;

    /**
     * Sends event to all remote endpoints that are subscribed to an eventgroup
     * containing the event
     */
    IEventSender::ErrorCode sendEvent(
        service_id::type,
        major_version::type,
        instance_id::type,
        uint16_t eventId,
        ::etl::span<eventgroup_id::type const>,
        uint16_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        port::type, // source port
        proto::type,
        uint16_t sessionId = 0U) override;

    /**
     * Sends event to given multicast destinationEndpoint if a subscription to
     * an eventgroup is present that contains the event.
     */
    IEventSender::ErrorCode sendMulticastEvent(
        service_id::type,
        major_version::type,
        instance_id::type,
        uint16_t eventId,
        ::etl::span<eventgroup_id::type const>,
        uint16_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        port::type, // source port
        proto::type,
        ::ip::IPEndpoint const& destinationEndpoint,
        uint16_t sessionId = 0U) override;

    /**
     *  Forwards information of received event to all entries of event listener
     *  list.
     */
    void eventReceived(
        service_id::type,
        uint16_t eventId,
        instance_id::type,
        major_version::type,
        SomeIpParser& parser) override;

    /* Adds entry to the event listener list. */
    void addEventListener(IEventListener& listener) override;
    /* Removes entry from the event listener list. */
    void removeEventListener(IEventListener& listener) override;

private:
    BufferedEventSender& _eventSender;
    ISubscriptionManager& _subscriptionManager;

    using EventListenerList = ::etl::intrusive_forward_list<IEventListener, ::etl::forward_link<0>>;
    EventListenerList _eventListeners;

    ::etl::ivector<SubscriptionEndpoint>& _subscriptionEndpoints;
};

namespace declare
{

template<uint8_t NUM_SUBSCRIPTION_ENDPOINTS>
class EventTransceiver : public ::someip::EventTransceiver
{
public:
    EventTransceiver(
        ::someip::BufferedEventSender& eventSender, ISubscriptionManager& subscriptionManager);

private:
    ::etl::vector<SubscriptionEndpoint, NUM_SUBSCRIPTION_ENDPOINTS> _subscriptionEndpoints;
};

template<uint8_t NUM_SUBSCRIPTION_ENDPOINTS>
inline EventTransceiver<NUM_SUBSCRIPTION_ENDPOINTS>::EventTransceiver(
    ::someip::BufferedEventSender& eventSender, ISubscriptionManager& subscriptionManager)
: ::someip::EventTransceiver(eventSender, subscriptionManager, _subscriptionEndpoints)
, _subscriptionEndpoints()
{}

} // namespace declare

} // namespace someip
