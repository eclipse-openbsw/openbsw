/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "ip/IPAddress.h"

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/signal.h>
#include <etl/span.h>

namespace ip
{
struct Ip4Config
{
    bool useDhcp = false;
};

/**
 * Represents a IP configuration for a network interface. It can represent configurations
 * for both IPv4 and IPv6 addressing. In case of IPv4 it holds also the corresponding
 * values for network mask, default gateway address and can return the valid subnet broadcast
 * address.
 */

class NetworkInterfaceConfig
{
public:
    using Ip6AddressType = uint32_t[4];

    /**
     * Constructor. Represents an invalid interface configuration (not configured).
     */
    NetworkInterfaceConfig();
    /**
     * Constructor for a IPv4 interface configuration. The corresponding values are expected as raw
     * 32 bit values (compare the IPAddress IPv4 constructor with a single uint32_t value).
     * \param ip4Address raw value for the IPv4 address
     * \param networkMask raw value for the corresponding network mask
     * \param defaultGateway raw value for the IPv4 address of the default gateway
     */
    NetworkInterfaceConfig(uint32_t ip4Address, uint32_t networkMask, uint32_t defaultGateway);

    /**
     * Constructor for a IPv6 interface configuration. The corresponding address is expected as
     * a raw array of four 32 bit values (compare to the IPAddress IPv6 constructor with an array).
     * \param ip6Address raw array holding the IPv6 address
     */
    explicit NetworkInterfaceConfig(Ip6AddressType const& ip6Address);

    /**
     * Checks whether this object represents a physically linked, valid IPv4 or IPv6 configuration.
     * \return true if physically linked and address valid
     */
    bool isValid() const;

    /**
     * Get the family of the represented IP configuration
     * \return
     *   - IPV4 in case of a valid IPv4 configuration
     *   - IPV6 in case of a valid IPv6 configuration
     *   - FAMILY_UNKNOWN in case of an invalid configurations
     */
    ::ip::IPAddress::Family ipFamily() const;

    /**
     * Get the IP address for the configuration.
     * \return a valid IPv4 or IPv6 address in case of a valid configuration, an undefined IP
     * address otherwise
     */
    ::ip::IPAddress ipAddress() const;

    /**
     * Get the corresponding IPv4 network mask for the configuration.
     * \return a valid IPv4 address holding the network mask for a IPv4 configuration, an undefined
     * IP address otherwise
     */
    ::ip::IPAddress networkMask() const;

    /**
     * Get the corresponding IPv4 default gateway address for the configuration.
     * \return a valid IPv4 default gateway address for a IPv4 configuration, an undefined IP
     * address otherwise
     */
    ::ip::IPAddress defaultGateway() const;

    /**
     * Calculates the subnet broadcast address for a IPv4 configuration.
     * \return a valid IPv4 subnet broadcat address for a IPv4 configuration, an undefined IP
     * address otherwise
     */
    ::ip::IPAddress broadcastAddress() const;

    /**
     * Compare two network addresses for equality.
     * \param lhs config on the left-hand side of the operator
     * \param rhs config on the right-hand side of the operator
     *  - true if config family and all address fields are equal.
     *  - false otherwise
     */
    friend bool operator==(NetworkInterfaceConfig const& lhs, NetworkInterfaceConfig const& rhs);

    /**
     * Compare two network addresses for inequality.
     * \param lhs config on the left-hand side of the operator
     * \param rhs config on the right-hand side of the operator
     *  - true if config family or any address field is not equal.
     *  - false otherwise
     */
    friend bool operator!=(NetworkInterfaceConfig const& lhs, NetworkInterfaceConfig const& rhs);

private:
    ::etl::array<uint32_t, 4> _config{};
    ::ip::IPAddress::Family _family;
};

/**
 * Inline implementation.
 */
inline bool NetworkInterfaceConfig::isValid() const
{
    return _family != ::ip::IPAddress::FAMILY_UNKNOWN;
}

inline ::ip::IPAddress::Family NetworkInterfaceConfig::ipFamily() const { return _family; }

inline bool operator!=(NetworkInterfaceConfig const& lhs, NetworkInterfaceConfig const& rhs)
{
    return !operator==(lhs, rhs);
}

using NetworkInterfaceConfigKey = uint8_t;

using ConfigChangedSlotType = ::etl::delegate<void(uint8_t, NetworkInterfaceConfig const&)>;

inline bool updateConfig(NetworkInterfaceConfig& config, NetworkInterfaceConfig const& newConfig)
{
    auto const change = config != newConfig;
    config            = newConfig;
    return change;
}

/**
 * Interface for updating and retrieving IP address configurations of network interfaces.
 *
 * IP addresses are typically assigned dynamically to network interfaces. Therefore components
 * need to get notified about changes of assigned network addresses. This can be done by
 * registering as a listener to config changes.
 */
class NetworkInterfaceConfigRegistry
{
public:
    virtual NetworkInterfaceConfig getConfig(uint8_t busId) const = 0;
    virtual bool connect(ConfigChangedSlotType const& slot)       = 0;
    virtual void disconnect(ConfigChangedSlotType const& slot)    = 0;

protected:
    NetworkInterfaceConfigRegistry()                                                 = default;
    ~NetworkInterfaceConfigRegistry()                                                = default;
    NetworkInterfaceConfigRegistry(NetworkInterfaceConfigRegistry const&)            = delete;
    NetworkInterfaceConfigRegistry& operator=(NetworkInterfaceConfigRegistry const&) = delete;
};

namespace declare
{
/**
 * Concrete NetworkInterfaceConfigRegistry that owns an etl::signal sized for a given
 * number of listener slots.
 */
template<size_t SlotCapacity>
class NetworkInterfaceConfigRegistry : public ::ip::NetworkInterfaceConfigRegistry
{
public:
    using ConfigChangedSignal
        = ::etl::signal<void(uint8_t, NetworkInterfaceConfig const&), SlotCapacity>;

    NetworkInterfaceConfigRegistry(
        ::etl::span<uint8_t const> busIds, ::etl::span<NetworkInterfaceConfig const> configs)
    : busIds(busIds), configs(configs)
    {}

    ::etl::span<uint8_t const> busIds;
    ::etl::span<NetworkInterfaceConfig const> configs;

    ConfigChangedSignal configChangedSignal;

    NetworkInterfaceConfig getConfig(uint8_t const busId) const override
    {
        for (size_t i = 0; i < busIds.size(); ++i)
        {
            if (busIds[i] == busId)
            {
                return configs[i];
            }
        }
        return {};
    }

    bool connect(ConfigChangedSlotType const& slot) override
    {
        return configChangedSignal.connect(slot);
    }

    void disconnect(ConfigChangedSlotType const& slot) override
    {
        configChangedSignal.disconnect(slot);
    }
};
} // namespace declare
} // namespace ip
