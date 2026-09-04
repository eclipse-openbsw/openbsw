/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/EventMessage.h"

#include <etl/algorithm.h>

namespace someip
{
namespace internal
{
EventMessage::EventMessage() { etl::fill(etl::begin(_buffer), etl::end(_buffer), 0x00U); }

void EventMessage::clear()
{
    _pduList.clear();
    _destination.clear();
    _sendTime     = 0U;
    _bufferOffset = 0U;
    _localPort    = 0U;
    _proto        = 0U;
}

bool EventMessage::isInitialized() const { return _destination.isSet(); }

void EventMessage::init(
    ::ip::IPEndpoint const& destination, uint16_t const localPort, uint8_t const proto)
{
    clear();

    _destination = destination;
    _localPort   = localPort;
    _proto       = proto;
}

bool EventMessage::isMatching(
    ::ip::IPEndpoint const& destination, uint16_t const localPort, uint8_t const proto) const
{
    return (_destination == destination) && (_localPort == localPort) && (_proto == proto);
}

bool EventMessage::isFull() const { return _pduList.full(); }

bool EventMessage::hasPdu(uint32_t const pdu) const { return _pduList.contains(pdu); }

void EventMessage::addPdu(uint32_t const pdu)
{
    if (!_pduList.full())
    {
        (void)_pduList.insert(pdu);
    }
}

::etl::span<uint8_t> EventMessage::getBuffer()
{
    return ::etl::span<uint8_t>(_buffer, BUFFER_SIZE);
}

uint16_t EventMessage::getBufferOffset() const { return _bufferOffset; }

void EventMessage::incBufferOffset(uint16_t const length) { _bufferOffset += length; }

::ip::IPEndpoint const& EventMessage::getDestination() const { return _destination; }

uint16_t EventMessage::getLocalPort() const { return _localPort; }

uint8_t EventMessage::getProto() const { return _proto; }

uint32_t EventMessage::getSendTime() const { return _sendTime; }

void EventMessage::adjustSendTime(uint32_t const time)
{
    if ((_sendTime == 0U) || (_sendTime > time))
    {
        _sendTime = time;
    }
}

} // namespace internal

} // namespace someip
