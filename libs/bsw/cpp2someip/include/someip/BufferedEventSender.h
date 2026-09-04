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

#include "someip/EventMessage.h"
#include "someip/IEventSender.h"
#include "someip/INetwork.h"
#include "someip/SomeIpConstants.h"

#include <async/Types.h>
#include <async/util/Call.h>

#include <ip/IPEndpoint.h>

#include <etl/flat_set.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <cstdint>

namespace common
{
class ITimeoutManager2;
}

namespace someip
{
class ISomeIpSerializable;
class ITpTransceiver;

/**
 * Collects events before sending to minimize network traffic.
 */
class BufferedEventSender
{
public:
    ~BufferedEventSender() = default;

    void init();
    void shutdown();

    /**
     * Enqueues an event for sending to a destination.
     *
     * \Note: Events is being send immediately if
     *  maximumDelayTime == 0 or
     *  payload > EventMessage::MAX_PAYLOAD_SIZE or
     *  no available buffer for a new destination
     */
    IEventSender::ErrorCode sendEvent(
        service_id::type serviceId,
        major_version::type majorVersion,
        uint16_t eventId,
        uint32_t maximumDelayTime,
        ISomeIpSerializable const* payload,
        uint16_t localPort,
        uint8_t proto,
        ::ip::IPEndpoint const& destination,
        uint16_t sessionId = 0U);

    void sendNextMessage();

    // Only used in tests !
    size_t countMessages() const;

protected:
    BufferedEventSender(
        INetwork& network,
        ::async::ContextType const ethernetContext,
        ITpTransceiver& tpTransceiver,
        ::etl::span<internal::EventMessage*> messages);

    void setMessages(::etl::span<internal::EventMessage*> messages) { _eventMessages = messages; }

private:
    static uint32_t const MAX_PACKET_DELAY = 100U;

    IEventSender::ErrorCode sendSingleEvent(
        NetworkChannel& channel,
        uint16_t localPort,
        uint8_t proto,
        ::ip::IPEndpoint const& destination,
        uint16_t sessionId,
        size_t length) const;

    void sendBufferedEvents(internal::EventMessage& message);

    static bool bufferEvent(
        internal::EventMessage& message, size_t length, ::etl::span<uint8_t> const& eventBuffer);

    static bool serializeEvent(
        service_id::type,
        major_version::type,
        uint16_t eventId,
        ISomeIpSerializable const* payload,
        ::etl::span<uint8_t> buffer,
        size_t* length /* out */,
        uint16_t sessionId = 0);

    internal::EventMessage*
    findMessage(::ip::IPEndpoint const& destination, port::type localPort, proto::type proto) const;

    internal::EventMessage* getEmptyMessage() const;
    internal::EventMessage* getNextScheduledMessage() const;

    void updateSchedule(uint32_t currentTime);

    ::async::ContextType const _ethernetContext;
    ::async::Function _sendNextMessageFunction;
    ::async::TimeoutType _sendNextMessageTimeout;
    INetwork& _network;
    ITpTransceiver& _tpTransceiver;
    ::etl::span<internal::EventMessage*> _eventMessages;
    uint32_t _nextSendTime; // avoid TimeoutManager::getRemainingTime()
};

namespace declare
{
template<uint8_t NUM_BUFFERS>
class BufferedEventSender : public ::someip::BufferedEventSender
{
public:
    BufferedEventSender(
        INetwork& network,
        ::async::ContextType const ethernetContext,
        ITpTransceiver& tpTransceiver);

private:
    internal::EventMessage _messageArray[NUM_BUFFERS];
    ::etl::vector<internal::EventMessage*, NUM_BUFFERS> _messageList;
};

template<uint8_t NUM_BUFFERS>
inline BufferedEventSender<NUM_BUFFERS>::BufferedEventSender(
    INetwork& network, ::async::ContextType const ethernetContext, ITpTransceiver& tpTransceiver)
: ::someip::BufferedEventSender(
    network, ethernetContext, tpTransceiver, ::etl::span<internal::EventMessage*>())
, _messageArray()
, _messageList()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    for (uint8_t i = 0U; i < NUM_BUFFERS; ++i)
#pragma GCC diagnostic pop
    {
        internal::EventMessage& buffer = _messageArray[i];
        _messageList.push_back(&buffer);
    }
    setMessages(_messageList);
}

// Specialization for zero buffers
template<>
class BufferedEventSender<0> : public ::someip::BufferedEventSender
{
public:
    BufferedEventSender(
        INetwork& network,
        ::async::ContextType const ethernetContext,
        ITpTransceiver& tpTransceiver)
    : ::someip::BufferedEventSender(
        network, ethernetContext, tpTransceiver, ::etl::span<internal::EventMessage*>())
    {}
};

} // namespace declare
} // namespace someip
