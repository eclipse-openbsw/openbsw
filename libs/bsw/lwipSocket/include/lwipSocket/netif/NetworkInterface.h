#pragma once

#include "ip/IPAddress.h"

#include "lwip/err.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "netif/ethernet.h"

namespace openbsw::lwip
{

/**
 * Representation of a single LwIP network interface.
 *
 * This class encapsulates a single LwIP network interface with an associated VLAN ID and IPv4
 * address. Furthermore, it provides an integration with a low-level network driver, which needs to
 * implement two methods:
 *
 *   - getMac() returning the MAC address of the physical device
 *   - writeFrame(struct netif*, struct pbuf*)
 *
 * To instantiate a network interface you need to instantiate an object and call the up() and down()
 * methods at the appropriate points in your lifecycle. Furthermore, you need to communicate link
 * up/down events by calling linkUp() and linkDown() respectively.
 *
 * Outgoing frames are handled via the LwIP linkoutput callback and passed to you low-level network
 * driver using the writeFrame() method. For incoming frames you have to call input(struct pbuf*)
 * for each frame received.
 *
 * Note, that VLANs are not handled explicitly. For outgoing frames you need to register a proper
 * LWIP_HOOK_VLAN_SET as shown below. Since LwIP only passes it's netif structure, we register the
 * pointer to our own instance in the free-to-use state member.
 *
 * \code{.cpp}
 * int32_t vlanForNetif(const struct netif* netif)
 * {
 *     auto* interface = reinterpret_cast<EthernetSystem::Interface*>(netif->state);
 *     return interface->hasVlanId() ? -1 : static_cast<int32_t>(interface->getVlanId());
 * }
 * \endcode
 *
 * For incoming frames you also need to implement the filtering for the VLAN on your own. For an
 * already existing implementation you can check the NetworkInterfaceCollection.
 */
template<typename D>
class NetworkInterface
{
public:
    using DriverType = D;

    static constexpr uint16_t UNTAGGED_VLAN_ID = 0xffffU;

public:
    /**
     * Constructor to create a network interface with the given VLAN ID and IPv4 address.
     */
    NetworkInterface(
        DriverType& driver,
        uint16_t vlanId,
        ::ip::IPAddress address,
        ::ip::IPAddress netmask,
        ::ip::IPAddress gateway)
    : _netif()
    , _driver(driver)
    , _vlanId(vlanId)
    , _address(address)
    , _netmask(netmask)
    , _gateway(gateway)
    {
        _netif.name[0] = '\0';
    }

    /**
     * Constructor to create a network interface with the given VLAN ID and no IPv4 address.
     */
    NetworkInterface(DriverType& driver, uint16_t vlanId)
    : _netif()
    , _driver(driver)
    , _vlanId(vlanId)
    , _address(::ip::make_ip4(0U, 0U, 0U, 0U))
    , _netmask(::ip::make_ip4(0U, 0U, 0U, 0U))
    , _gateway(::ip::make_ip4(0U, 0U, 0U, 0U))
    {
        _netif.name[0] = '\0';
    }

    /**
     * Returns whether the network interface has a VLAN ID assigned.
     *
     * In case the network interface has a VLAN ID assigned, it should only handle incoming frames
     * of this VLAN. Furthermore, outgoing frames need to be tagged with this VLAN ID.
     */
    bool hasVlanId() const { return _vlanId != UNTAGGED_VLAN_ID; }

    /**
     * Returns the VLAN ID assigned to this network interface.
     */
    uint16_t getVlanId() const { return _vlanId; }

    /**
     * Sets the interface up and adds it to the LwIP stack.
     */
    void up()
    {
        _netif.linkoutput = &linkoutput;

        ip4_addr address;
        ::etl::copy(
            ip::packed(_address),
            ::etl::span<uint8_t>(reinterpret_cast<uint8_t*>(&address.addr), sizeof(address.addr)));

        ip4_addr netmask;
        ::etl::copy(
            ip::packed(_netmask),
            ::etl::span<uint8_t>(reinterpret_cast<uint8_t*>(&netmask.addr), sizeof(netmask.addr)));

        ip4_addr gateway;
        ::etl::copy(
            ip::packed(_gateway),
            ::etl::span<uint8_t>(reinterpret_cast<uint8_t*>(&gateway.addr), sizeof(gateway.addr)));

        netif_add(&_netif, &address, &netmask, &gateway, this, &initnetif, nullptr);
        netif_set_up(&_netif);
    }

    /**
     * Sets the interface down and removes it from the LwIP stack.
     */
    void down()
    {
        netif_set_down(&_netif);
        netif_remove(&_netif);
    }

    /**
     * Notifies about the link state of the underlying physical interface.
     */
    void linkUp() { netif_set_link_up(&_netif); }

    /**
     * Notifies about the link state of the underlying physical interface.
     */
    void linkDown() { netif_set_link_down(&_netif); }

    /**
     * Processes a frame received from the low-lever device driver.
     */
    void input(struct pbuf* buffer) { _netif.input(buffer, &_netif); }

private:
    static err_t linkoutput(struct netif* netif, struct pbuf* buffer)
    {
        auto* interface = reinterpret_cast<NetworkInterface*>(netif->state);
        return interface->_driver.writeFrame(&interface->_netif, buffer);
    }

    static err_t initnetif(struct netif* netif)
    {
        auto const* interface = reinterpret_cast<NetworkInterface*>(netif->state);

        netif->input  = &ethernet_input;
        netif->output = &etharp_output;

        netif->flags
            |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP);
        netif->mtu = 1500U;

        netif->hwaddr_len = 6U;
        interface->_driver.getMac().copyTo(netif->hwaddr);

        return ERR_OK;
    }

private:
    netif _netif;
    DriverType& _driver;
    uint16_t _vlanId;
    ::ip::IPAddress _address;
    ::ip::IPAddress _netmask;
    ::ip::IPAddress _gateway;
};

} // namespace openbsw::lwip
