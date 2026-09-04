/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/BufferedEventSender.h"

#include "bsp/timer/SystemTimer.h"
#include "someip/ISomeIpSerializable.h"
#include "someip/NetworkChannel.h"
#include "someip/SomeIpMessage.h"
#include "someip/SomeIpSerializer.h"
#include "someip/Statistics.h"
#include "someip/TpTransceiver.h"
#include "someip/logger.h"

#include <etl/algorithm.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

BufferedEventSender::BufferedEventSender(
    INetwork& network,
    ::async::ContextType const ethernetContext,
    ITpTransceiver& tpTransceiver,
    ::etl::span<internal::EventMessage*> messages)
: _ethernetContext(ethernetContext)
, _sendNextMessageFunction(
      ::async::Function::CallType::
          create<BufferedEventSender, &BufferedEventSender::sendNextMessage>(*this))
, _sendNextMessageTimeout()
, _network(network)
, _tpTransceiver(tpTransceiver)
, _eventMessages(messages)
, _nextSendTime(0U)
{}

void BufferedEventSender::init() { _nextSendTime = 0U; }

void BufferedEventSender::shutdown()
{
    _sendNextMessageTimeout.cancel();

    for (internal::EventMessage* const message : _eventMessages)
    {
        if (message != nullptr)
        {
            message->clear();
        }
    }
}

IEventSender::ErrorCode BufferedEventSender::sendEvent(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    uint16_t const eventId,
    uint32_t const maximumDelayTime,
    ISomeIpSerializable const* const payload,
    uint16_t const localPort,
    uint8_t const proto,
    ::ip::IPEndpoint const& destination,
    uint16_t const sessionId)
{
    Statistics::incCounter(Statistics::Counter::PDU_TX);

    auto channel = _network.getRpcChannel(localPort, destination, proto);

    if (!channel.has_value())
    {
        WARN_LOG(SOMEIP, "BufferedEventSender::sendSingleEvent() no channel");
        return IEventSender::ErrorCode::EVENT_SEND_ERROR;
    }

    size_t eventSize = 0U;
    if (!serializeEvent(
            serviceId,
            majorVersion,
            eventId,
            payload,
            channel->getOutputBuffer(),
            &eventSize,
            sessionId))
    {
        return IEventSender::ErrorCode::EVENT_SEND_ERROR;
    }

    // check if we should send this event immediately
    if ((_eventMessages.size() == 0U) || (maximumDelayTime == 0U)
        || (eventSize > internal::EventMessage::BUFFER_SIZE))
    {
        return sendSingleEvent(
            channel.value(), localPort, proto, destination, sessionId, eventSize);
    }

    uint32_t const pdu
        = ((static_cast<uint32_t>(serviceId) << 16U) | static_cast<uint32_t>(eventId));
    uint32_t const currentTime = static_cast<uint32_t>(getSystemTimeMs32Bit());
    uint32_t const sendTime
        = currentTime
          + ((maximumDelayTime > MAX_PACKET_DELAY) ? MAX_PACKET_DELAY : maximumDelayTime);

    // check if we already got a message for this endpoint
    internal::EventMessage* message = findMessage(destination, localPort, proto);

    if (message != nullptr)
    {
        // send message if not enough space left or already full
        if (((message->getBufferOffset() + eventSize) > internal::EventMessage::BUFFER_SIZE)
            || message->isFull())
        {
            Statistics::incCounter(Statistics::Counter::TRAIN_FULL);
            sendBufferedEvents(*message);
        }
        else if (message->hasPdu(pdu)) // send message if duplicate pdu
        {
            Statistics::incCounter(Statistics::Counter::TRAIN_DUPLICATE_PDU);
            sendBufferedEvents(*message);
        }
        else
        {
            // misra
        }
    }
    else
    {
        // check if we have a free message
        message = getEmptyMessage();
        if (message == nullptr)
        {
            // send next scheduled message
            message = getNextScheduledMessage();
            if (message != nullptr)
            {
                sendBufferedEvents(*message);
            }
        }
    }

    if (message == nullptr) // no message available !
    {
        ERROR_LOG(SOMEIP, "BufferedEventSender::sendEvent: no free buffer");
        Statistics::incCounter(Statistics::Counter::TRAIN_NOT_AVAILABLE);
        return IEventSender::ErrorCode::EVENT_SEND_ERROR;
    }

    bool isNew = false;
    if (!message->isInitialized())
    {
        message->init(destination, localPort, proto);
        isNew = true;
    }

    if (bufferEvent(*message, eventSize, channel->getOutputBuffer()))
    {
        message->addPdu(pdu);
        message->adjustSendTime(sendTime);

        updateSchedule(currentTime);
        return IEventSender::ErrorCode::EVENT_SEND_OK;
    }

    // cleanup
    if (isNew)
    {
        message->clear();
    }

    return IEventSender::ErrorCode::EVENT_SEND_ERROR;
}

// virtual
void BufferedEventSender::sendNextMessage()
{
    uint32_t const currentTime = static_cast<uint32_t>(getSystemTimeMs32Bit());
    _nextSendTime              = 0U;

    // send expired messages
    for (internal::EventMessage* const message : _eventMessages)
    {
        if (message != nullptr)
        {
            if (!message->isInitialized())
            {
                continue;
            }

            uint32_t const sendTime = message->getSendTime();

            if (currentTime >= sendTime)
            {
                Statistics::incCounter(Statistics::Counter::TRAIN_TIMEOUT);
                sendBufferedEvents(*message);
            }
            else if ((_nextSendTime == 0U) || (_nextSendTime > sendTime))
            {
                _nextSendTime = sendTime;
            }
            else
            {
                // misra
            }
        }
        else
        {
            ERROR_LOG(SOMEIP, "BufferedEventSender::expired() nullptr msg");
        }
    }

    // schedule next message
    if (_nextSendTime != 0U)
    {
        uint32_t const timeout = _nextSendTime - currentTime;
        async::schedule(
            _ethernetContext,
            _sendNextMessageFunction,
            _sendNextMessageTimeout,
            timeout,
            ::async::TimeUnit::MILLISECONDS);
    }
}

IEventSender::ErrorCode BufferedEventSender::sendSingleEvent(
    NetworkChannel& channel,
    uint16_t const /* localPort */,
    uint8_t const proto,
    ::ip::IPEndpoint const& /* destination */,
    uint16_t const sessionId,
    size_t const length) const
{
    IEventSender::ErrorCode errorCode = IEventSender::ErrorCode::EVENT_SEND_OK;

    if (ITpTransceiver::isOutgoingTpMessage(proto, static_cast<uint32_t>(length)))
    {
        SomeIpMessage message(channel.getOutputBuffer().first(length));
        message.setSessionId(sessionId);

        if (!_tpTransceiver.sendTpMessage(channel, message))
        {
            errorCode = IEventSender::ErrorCode::EVENT_SEND_ERROR;
        }
    }
    else
    {
        if (!channel.send(static_cast<uint32_t>(length)))
        {
            errorCode = IEventSender::ErrorCode::EVENT_SEND_ERROR;
        }
    }

    if (errorCode != IEventSender::ErrorCode::EVENT_SEND_OK)
    {
        WARN_LOG(SOMEIP, "BufferedEventSender::sendSingleEvent() send failed");
    }

    return errorCode;
}

void BufferedEventSender::sendBufferedEvents(internal::EventMessage& message)
{
    auto channel = _network.getRpcChannel(
        message.getLocalPort(), message.getDestination(), message.getProto());

    if (!channel.has_value())
    {
        WARN_LOG(SOMEIP, "BufferedEventSender::sendBufferedEvents() no channel");
    }
    else
    {
        size_t const length = message.getBufferOffset();

        if (!channel->send(static_cast<uint32_t>(length), message.getBuffer().first(length)))
        {
            WARN_LOG(SOMEIP, "BufferedEventSender::sendBufferedEvents() send failed");
        }
    }

    message.clear();
}

bool BufferedEventSender::bufferEvent(
    internal::EventMessage& message, size_t const length, ::etl::span<uint8_t> const& eventBuffer)
{
    ::etl::span<uint8_t> const buffer = message.getBuffer().subspan(message.getBufferOffset());

    etl::copy_n(eventBuffer.begin(), length, buffer.begin());

    message.incBufferOffset(static_cast<uint16_t>(length));
    return true;
}

bool BufferedEventSender::serializeEvent(
    service_id::type const serviceId,
    major_version::type const majorVersion,
    uint16_t const eventId,
    ISomeIpSerializable const* const payload,
    ::etl::span<uint8_t> const buffer,
    size_t* const length,
    uint16_t const sessionId)
{
    if (buffer.size() < SomeIpMessage::OFFSET_PAYLOAD)
    {
        WARN_LOG(SOMEIP, "BufferedEventSender::serializeEvent() buffer is too small");
        return false;
    }

    SomeIpMessage message(buffer);

    message.setServiceId(serviceId);
    message.setMethodId(eventId);
    message.setPayloadLength(0);
    message.setRequestId(0U);
    message.setMessageType(SomeIpMessage::MessageType::NOTIFICATION);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_OK);
    message.setProtocolVersion(1U);
    message.setInterfaceVersion(majorVersion);
    message.setSessionId(sessionId);

    if (payload != nullptr)
    {
        SomeIpSerializer serializer(
            ::etl::span<uint8_t>(message.getPayload(), message.getMaximumPayloadLength()));

        serializer << *payload;

        if (!serializer.isGood())
        {
            WARN_LOG(SOMEIP, "BufferedEventSender::serializeEvent() serialization failed");
            return false;
        }
        message.setPayloadLength(static_cast<uint32_t>(serializer.getCurrentPosition()));
    }

    if (length != nullptr)
    {
        *length = message.getTotalLength();
    }
    else
    {
        ERROR_LOG(SOMEIP, "BufferedEventSender::serializeEvent() invalid length");
    }
    return true;
}

size_t BufferedEventSender::countMessages() const
{
    size_t result = 0U;

    for (internal::EventMessage* const message : _eventMessages)
    {
        if ((message != nullptr) && (message->isInitialized()))
        {
            result++;
        }
    }

    return result;
}

internal::EventMessage* BufferedEventSender::findMessage(
    ::ip::IPEndpoint const& destination, port::type const localPort, proto::type const proto) const
{
    for (internal::EventMessage* const message : _eventMessages)
    {
        if ((message != nullptr) && (message->isMatching(destination, localPort, proto)))
        {
            return message;
        }
    }

    return nullptr;
}

internal::EventMessage* BufferedEventSender::getEmptyMessage() const
{
    for (internal::EventMessage* const message : _eventMessages)
    {
        if ((message != nullptr) && (!message->isInitialized()))
        {
            return message;
        }
    }

    return nullptr;
}

internal::EventMessage* BufferedEventSender::getNextScheduledMessage() const
{
    internal::EventMessage* result = nullptr;

    for (internal::EventMessage* const message : _eventMessages)
    {
        if ((message != nullptr) && (!message->isInitialized()))
        {
            continue;
        }
        if ((result == nullptr)
            || ((message != nullptr) && (result->getSendTime() > message->getSendTime())))
        {
            result = message;
        }
    }

    return result;
}

void BufferedEventSender::updateSchedule(uint32_t const currentTime)
{
    internal::EventMessage* const message = getNextScheduledMessage();

    if (message == nullptr)
    {
        return;
    }

    uint32_t const sendTime = message->getSendTime();

    if ((_nextSendTime == 0U) || (_nextSendTime > sendTime))
    {
        _nextSendTime          = sendTime;
        uint32_t const timeout = sendTime - currentTime;

        _sendNextMessageTimeout.cancel();
        async::schedule(
            _ethernetContext,
            _sendNextMessageFunction,
            _sendNextMessageTimeout,
            timeout,
            ::async::TimeUnit::MILLISECONDS);
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
