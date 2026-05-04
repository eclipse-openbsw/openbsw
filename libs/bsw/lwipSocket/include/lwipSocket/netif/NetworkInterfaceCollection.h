#pragma once

#include "lwipSocket/netif/NetworkInterface.h"

#include <etl/vector.h>

namespace openbsw::lwip
{

/**
 * Collection of multiple network interfaces with VLAN handling.
 *
 * In case you want to manage multiple network interfaces with different VLAN IDs you need to
 * implement proper routing to decide to which interface to route a frame. LwIP does not provide any
 * read-to-use implementation for this.
 */
template<typename D, size_t C>
class NetworkInterfaceCollection
{
public:
    using DriverType    = D;
    using InterfaceType = NetworkInterface<D>;

    static constexpr size_t InterfaceCount = C;

public:
    /**
     * Constructor to create an empty collection for the given driver.
     */
    NetworkInterfaceCollection(D& driver) : _driver(driver) {}

    /**
     * Adds a single network interface with the given IP address and netmask.
     */
    void add(uint16_t vlan, ::ip::IPAddress address, ::ip::IPAddress mask)
    {
        add(vlan, address, mask, ::ip::make_ip4(0U, 0U, 0U, 0U));
    }

    /**
     * Adds a single network interface with the given IP address, netmask and gateway.
     */
    void add(uint16_t vlan, ::ip::IPAddress address, ::ip::IPAddress mask, ::ip::IPAddress gateway)
    {
        _interfaces.emplace_back(_driver, vlan, address, mask, gateway);
    }

    /**
     * Sets all interfaces up and adds them to the LwIP stack.
     */
    void up()
    {
        for (auto& interface : _interfaces)
        {
            interface.up();
        }
    }

    /**
     * Sets all interfaces down and removes them from the LwIP stack.
     */
    void down()
    {
        for (auto& interface : _interfaces)
        {
            interface.down();
        }
    }

    /**
     * Notifies about the link state of the underlying physical interface.
     */
    void linkUp()
    {
        for (auto& interface : _interfaces)
        {
            interface.linkUp();
        }
    }

    /**
     * Notifies about the link state of the underlying physical interface.
     */
    void linkDown()
    {
        for (auto& interface : _interfaces)
        {
            interface.linkDown();
        }
    }

    /**
     * Processes a frame received from the low-lever device driver.
     *
     * When having multiple network interfaces with different VLAN IDs we need to decide to which
     * network interface to route a frame. Therefore, we parse the incoming frame, extract it's VLAN
     * ID and handle it for the matching interface.
     */
    void input(struct pbuf* buffer)
    {
        auto payload = etl::span<uint8_t>(static_cast<uint8_t*>(buffer->payload), buffer->len);
        if (payload.size() < 16U)
        {
            return;
        }
        payload.advance(12U);
        auto const type = payload.take<etl::be_uint16_t>();
        auto const vlanId
            = (type == 0x8100U) ? (payload.take<etl::be_uint16_t>() & 0x0FFFU) : 0xFFFFU;

        for (auto& interface : _interfaces)
        {
            if (interface.getVlanId() == vlanId)
            {
                interface.input(buffer);
                return;
            }
        }
    }

private:
    DriverType& _driver;
    ::etl::vector<InterfaceType, InterfaceCount> _interfaces;
};

} // namespace openbsw::lwip
