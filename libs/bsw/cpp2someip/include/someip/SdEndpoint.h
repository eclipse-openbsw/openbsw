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

#include "someip/SomeIpConstants.h"

#include <ip/IPEndpoint.h>

#include <cstdint>

namespace someip
{
class SdEndpoint : public ::ip::IPEndpoint
{
public:
    SdEndpoint();
    SdEndpoint(::ip::IPAddress const& ipAddr, uint16_t port, uint8_t proto);
    SdEndpoint(SdEndpoint const& other);

    SdEndpoint& operator=(SdEndpoint const& other);

    bool isValid() const;
    void clear();

    void setProto(uint8_t proto);
    uint8_t getProto() const;

private:
    uint8_t _proto;
};

inline SdEndpoint::SdEndpoint() : ::ip::IPEndpoint(), _proto(SomeIpConstants::INVALID_PROTO) {}

inline SdEndpoint::SdEndpoint(
    ::ip::IPAddress const& ipAddr, uint16_t const port, uint8_t const proto)
: ::ip::IPEndpoint(ipAddr, port), _proto(proto)
{}

inline SdEndpoint::SdEndpoint(SdEndpoint const& other) = default;

inline bool SdEndpoint::isValid() const
{
    return isSet() && (_proto != SomeIpConstants::INVALID_PROTO) && (getPort() != 0);
}

inline void SdEndpoint::clear()
{
    ::ip::IPEndpoint::clear();
    _proto = SomeIpConstants::INVALID_PROTO;
}

inline void SdEndpoint::setProto(uint8_t const proto) { _proto = proto; }

inline uint8_t SdEndpoint::getProto() const { return _proto; }

inline SdEndpoint& SdEndpoint::operator=(SdEndpoint const& other)
{
    if (this != &other)
    {
        setAddress(other.getAddress());
        setPort(other.getPort());
        _proto = other._proto;
    }

    return *this;
}

} // namespace someip
