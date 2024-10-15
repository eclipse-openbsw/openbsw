// Copyright 2025 Accenture.

#pragma once

#include "ip/IPAddress.h"

namespace ip
{
/**
 * An IPEndpoint combines an IPAddress and a port to represent
 * a complete endpoint.
 */
class IPEndpoint
{
public:
    IPEndpoint() : _address(), _port(PORT_DEFAULT_VALUE), _flags(0U) {}

    IPEndpoint(IPAddress const& addr, uint16_t const port)
    : _address(addr), _port(port), _flags(PORT_IS_SET)
    {}

    IPEndpoint(IPEndpoint const& other) = default;

    bool isSet() const
    {
        bool const isPortSet = (_flags & PORT_IS_SET) != 0;
        return isPortSet && (!isUnspecified(_address));
    }

    void clear()
    {
        _address = IPAddress();
        _port    = PORT_DEFAULT_VALUE;
        _flags   = 0U;
    }

    IPAddress const& getAddress() const { return _address; }

    void setAddress(IPAddress const& addr) { _address = addr; }

    uint16_t getPort() const { return _port; }

    void setPort(uint16_t const port)
    {
        _port = port;
        _flags |= PORT_IS_SET;
    }

    IPEndpoint& operator=(IPEndpoint const& other) = default;

    friend bool operator==(IPEndpoint const& lhs, IPEndpoint const& rhs);
    friend bool operator!=(IPEndpoint const& lhs, IPEndpoint const& rhs);
    friend bool operator<(IPEndpoint const& lhs, IPEndpoint const& rhs);

private:
    static uint16_t const PORT_DEFAULT_VALUE = 0U;
    static uint8_t const PORT_IS_SET         = 0x01U;
    IPAddress _address;
    uint16_t _port;
    uint8_t _flags;
};

inline bool operator==(IPEndpoint const& lhs, IPEndpoint const& rhs)
{
    return (lhs._address == rhs._address) && (lhs._port == rhs._port) && (lhs._flags == rhs._flags);
}

inline bool operator!=(IPEndpoint const& lhs, IPEndpoint const& rhs) { return !(lhs == rhs); }

inline bool operator<(IPEndpoint const& lhs, IPEndpoint const& rhs)
{
    if (lhs._flags != rhs._flags)
    {
        return lhs._flags < rhs._flags;
    }

    if (lhs._port != rhs._port)
    {
        return lhs._port < rhs._port;
    }

    return IPAddressCompareLess()(lhs._address, rhs._address);
}

} // namespace ip
