// Copyright 2025 Accenture.

#pragma once

#include "ip/IPEndpoint.h"

#include <platform/estdint.h>

namespace udp
{
/**
 * UDP datagram packet.
 */
class DatagramPacket
{
public:
    // [DatagramPacket]
    /**
     * Constructs a UDP datagram packet.
     * \param  data     array of data
     * \param  length   length of data
     * \param  address  target IPAddress
     * \param  port     target port
     */
    DatagramPacket(uint8_t const data[], uint16_t length, ::ip::IPAddress address, uint16_t port);
    DatagramPacket(uint8_t const data[], uint16_t length, ::ip::IPEndpoint const& endpoint);
    DatagramPacket(DatagramPacket const&)            = delete;
    DatagramPacket& operator=(DatagramPacket const&) = delete;

    uint8_t const* getData() const { return _data; }

    uint16_t getLength() const { return _length; }

    ::ip::IPEndpoint getEndpoint() const { return _endpoint; }

    ::ip::IPAddress getAddress() const { return _endpoint.getAddress(); }

    uint16_t getPort() const { return _endpoint.getPort(); }

    bool operator==(DatagramPacket const& other) const;
    // [DatagramPacket]
private:
    ::ip::IPEndpoint _endpoint;
    uint8_t const* _data;
    uint16_t _length;
};

} // namespace udp
