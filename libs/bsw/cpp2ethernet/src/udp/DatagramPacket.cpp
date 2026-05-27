/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "udp/DatagramPacket.h"

#include <cstring>

namespace udp
{
DatagramPacket::DatagramPacket(
    uint8_t const data[], uint16_t const length, ::ip::IPAddress const address, uint16_t const port)
: _endpoint(::ip::IPEndpoint(address, port)), _data(data), _length(length)
{}

DatagramPacket::DatagramPacket(
    uint8_t const data[], uint16_t const length, ::ip::IPEndpoint const& endpoint)
: _endpoint(endpoint), _data(data), _length(length)
{}

bool DatagramPacket::operator==(DatagramPacket const& other) const
{
    if (_endpoint == other._endpoint)
    {
        if (_length == other._length)
        {
            return ::memcmp(_data, other._data, static_cast<size_t>(_length)) == 0;
        }
    }
    return false;
}

} // namespace udp
