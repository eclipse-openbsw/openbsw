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

#include <ip/IPEndpoint.h>

#include <etl/flat_set.h>
#include <etl/span.h>
#include <cstdint>

namespace someip
{
namespace internal
{
class EventMessage
{
public:
    static uint16_t const BUFFER_SIZE = 1416U; // UDP (1400 byte payload + 16 byte header)

    EventMessage();

    /**
     * Resets all members.
     */
    void clear();

    /**
     * Checks if destination endpoint is set.
     */
    bool isInitialized() const;

    /**
     * Performs clear() and sets member to given parameters.
     */
    void init(::ip::IPEndpoint const& destination, uint16_t localPort, uint8_t proto);

    /**
     * Checks for equality of given parameters with respective members.
     */
    bool isMatching(::ip::IPEndpoint const& destination, uint16_t localPort, uint8_t proto) const;

    /**
     * Checks if pdu list is full.
     */
    bool isFull() const;

    bool hasPdu(uint32_t pdu) const;

    /**
     * Adds pdu to pdu list if it is not already full.
     */
    void addPdu(uint32_t pdu);

    ::etl::span<uint8_t> getBuffer();
    uint16_t getBufferOffset() const;
    void incBufferOffset(uint16_t length);

    ::ip::IPEndpoint const& getDestination() const;
    uint16_t getLocalPort() const;
    uint8_t getProto() const;

    uint32_t getSendTime() const;

    /**
     * Sets send time member to "time" if send it is equals "0" or greater than
     * "time".
     */
    void adjustSendTime(uint32_t time);

private:
    static uint8_t const MAX_NUMBER_OF_PDUS = 64U;

    ::etl::flat_set<uint32_t, MAX_NUMBER_OF_PDUS> _pduList;

    uint8_t _buffer[BUFFER_SIZE];

    ::ip::IPEndpoint _destination{};
    uint32_t _sendTime     = 0;
    uint16_t _bufferOffset = 0;
    uint16_t _localPort    = 0;
    uint8_t _proto         = 0;
};

} // namespace internal

} // namespace someip
