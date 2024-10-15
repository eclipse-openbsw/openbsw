// Copyright 2025 Accenture.

#include "lwipSocket/udp/LwipDatagramSocket.h"

#include "lwipSocket/utils/LwipHelper.h"
#include "lwipSocket/utils/TaskAssert.h"

#include <ip/to_str.h>
#include <udp/DatagramPacket.h>
#include <udp/IDataListener.h>
#include <udp/UdpLogger.h>

extern "C"
{
#include "lwip/igmp.h"
#include "lwip/ip4.h"
#include "lwip/ip6.h"
#include "lwip/mld6.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#if LWIP_IPV4_SRC_ROUTING
struct netif* custom_ip_route_src(ip4_addr_t const* const /*dest*/, ip4_addr_t const* const src)
{
    if (src != NULL)
    {
        for (struct netif* netif = netif_list; netif != NULL; netif = netif->next)
        {
            /* is the netif up, does it have a link and a valid address? */
            bool const isNetifUp = netif_is_up(netif) != 0U;
            bool const isLinkUp  = netif_is_link_up(netif) != 0U;
            if (isNetifUp && isLinkUp && (!ip4_addr_isany_val(*netif_ip4_addr(netif))))
            {
                if (ip4_addr_cmp(src, netif_ip4_addr(netif)))
                {
                    /* return netif on which to forward IP packet */
                    return netif;
                }
            }
        }
    }
    return NULL;
}
#endif
}

namespace udp
{
namespace logger = ::util::logger;
using ::ip::IPAddress;

LwipDatagramSocket::LwipDatagramSocket()
: AbstractDatagramSocket()
, fpRxPcb(nullptr)
, fpTxPcb(nullptr)
, fpPBufHead(nullptr)
, fOffsetInCurrentPBuf(0)
, fMulticastPcbs()
{}

bool LwipDatagramSocket::isBound() const { return (fpRxPcb != nullptr); }

bool LwipDatagramSocket::isConnected() const { return (fpTxPcb != nullptr); }

bool LwipDatagramSocket::isClosed() const { return (!(isBound() && isConnected())); }

AbstractDatagramSocket::ErrorCode
LwipDatagramSocket::bind(ip::IPAddress const* pIpAddress, uint16_t port)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (_dataListener == nullptr)
    {
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NO_DATA_LISTENER;
    }

    close();

    lwip_ip_addr_type type = IPADDR_TYPE_ANY;
    if (pIpAddress != nullptr)
    {
        type = (ip::isIp4Address(*pIpAddress)) ? IPADDR_TYPE_V4 : IPADDR_TYPE_V6;
    }
    fpRxPcb = udp_new_ip_type(static_cast<uint8_t>(type));
    if (fpRxPcb == nullptr)
    {
        logger::Logger::error(logger::UDP, " LwipDatagramSocket::bind(): udp_new failed!");
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
    }
    fpRxPcb->so_options |= SOF_REUSEADDR;
    err_t status;
    if (pIpAddress != nullptr)
    {
        ip_addr_t const tmpIpAddress = lwiputils::to_lwipIp(*pIpAddress);
        status                       = udp_bind(fpRxPcb, &tmpIpAddress, port);
    }
    else
    {
        status = udp_bind(fpRxPcb, nullptr, port);
    }
    if (status != 0)
    {
        logger::Logger::error(
            logger::UDP, " LwipDatagramSocket::bind(): udp_bind failed with status %d!", status);
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
    }
    udp_recv(fpRxPcb, &udpReceiveListener, static_cast<void*>(this));
    logger::Logger::info(logger::UDP, "DatagramSocket bound to UDP port %d", getLocalPort());
    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
}

AbstractDatagramSocket::ErrorCode LwipDatagramSocket::join(ip::IPAddress const& groupAddr)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (ip::isIp4Address(groupAddr))
    {
#if LWIP_IPV4 && LWIP_IGMP
        if (isBound() && (!isAlreadyJoined(groupAddr)) && (!fMulticastPcbs.full()))
        {
            ip_addr_t const lwipGroupAddr = lwiputils::to_lwipIp(groupAddr);
            udp_pcb* pRxMulticastPcb      = nullptr;
            if (!IP_IS_ANY_TYPE_VAL(fpRxPcb->local_ip))
            {
                pRxMulticastPcb = udp_new_ip_type(IPADDR_TYPE_V4);
                if (pRxMulticastPcb == nullptr)
                {
                    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
                }
                pRxMulticastPcb->so_options |= SOF_REUSEADDR;
                (void)udp_bind(pRxMulticastPcb, &lwipGroupAddr, fpRxPcb->local_port);
                udp_recv(pRxMulticastPcb, &udpReceiveListener, static_cast<void*>(this));
            }
            err_t const status
                = igmp_joingroup(ip_2_ip4(&fpRxPcb->local_ip), ip_2_ip4(&lwipGroupAddr));
            if (status == ERR_OK)
            {
                logger::Logger::info(logger::UDP, "DatagramSocket joined the group");
                fMulticastPcbs.push_back(pRxMulticastPcb);
                return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
            }
            else
            {
                logger::Logger::error(
                    logger::UDP,
                    " LwipDatagramSocket::join(): igmp_joingroup failed with status %d!",
                    status);
                return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
            }
        }
#else
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
#endif // LWIP_IGMP
    }
    else
    {
#if LWIP_IPV6
        if (isBound() && (!isAlreadyJoined(groupAddr)) && (!fMulticastPcbs.full())
            && ip::isIp6Address(groupAddr))
        {
            ip_addr_t const lwipGroupAddr = lwiputils::to_lwipIp(groupAddr);
            udp_pcb* pRxMulticastPcb      = nullptr;
            if (!IP_IS_ANY_TYPE_VAL(fpRxPcb->local_ip))
            {
                pRxMulticastPcb = udp_new_ip_type(IPADDR_TYPE_V6);
                if (pRxMulticastPcb == nullptr)
                {
                    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
                }
                pRxMulticastPcb->so_options |= SOF_REUSEADDR;
                (void)udp_bind(pRxMulticastPcb, &lwipGroupAddr, fpRxPcb->local_port);
                // storing the local_ip of the socket will make dispatching incoming multicast
                // packets from different VLANs possible.
                pRxMulticastPcb->remote_ip = fpRxPcb->local_ip;
                udp_recv(pRxMulticastPcb, &udpReceiveListener, static_cast<void*>(this));
            }
            if (mld6_joingroup(ip_2_ip6(&fpRxPcb->local_ip), ip_2_ip6(&lwipGroupAddr)) == ERR_OK)
            {
                fMulticastPcbs.push_back(pRxMulticastPcb);
                return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
            }
        }
#else
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
#endif // LWIP_IPV6
    }

    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
}

bool LwipDatagramSocket::isAlreadyJoined(ip::IPAddress const& ip) const
{
    ip_addr_t groupAddr = lwiputils::to_lwipIp(ip);

    for (auto it = fMulticastPcbs.begin(); it != fMulticastPcbs.end(); ++it)
    {
        if (ip_addr_cmp(&(*it)->local_ip, &groupAddr) != 0)
        {
            return true;
        }
    }
    return false;
}

void LwipDatagramSocket::udpReceiveListener(
    void* const arg,
    udp_pcb* const /* pcb */,
    struct pbuf* const p,
    ip_addr_t const* const src_ip,
    uint16_t const src_port)
{
    if (arg == nullptr)
    {
        logger::Logger::error(
            logger::UDP, "LwipDatagramSocket::udpReceiveListener(): arg is NULL!");
        return;
    }

    LwipDatagramSocket* const pSocket = static_cast<LwipDatagramSocket*>(arg);
    if (pSocket->_dataListener != nullptr)
    {
        pbuf_ref(p);
        if (pSocket->fpPBufHead != nullptr)
        {
            pbuf_cat(pSocket->fpPBufHead, p);
        }
        else
        {
            pSocket->fpPBufHead           = p;
            pSocket->fOffsetInCurrentPBuf = 0;
        }
        pSocket->_dataListener->dataReceived(
            *pSocket,
            lwiputils::from_lwipIp(*src_ip),
            src_port,
            lwiputils::from_lwipIp(*ip_current_dest_addr()),
            static_cast<uint16_t>(pSocket->fpPBufHead->tot_len - pSocket->fOffsetInCurrentPBuf));
        (void)pbuf_free(p);
    }
}

size_t LwipDatagramSocket::read(uint8_t* buffer, size_t n)
{
    if (fpPBufHead != nullptr)
    {
        if (fpPBufHead->tot_len - fOffsetInCurrentPBuf < n)
        {
            n = fpPBufHead->tot_len - fOffsetInCurrentPBuf;
        }

        if (buffer != nullptr)
        {
            (void)pbuf_copy_partial(
                fpPBufHead,
                buffer,
                static_cast<uint16_t>(n),
                static_cast<uint16_t>(fOffsetInCurrentPBuf));
        }
        size_t removedBytes = 0U;
        while ((fpPBufHead != nullptr) && (removedBytes < n))
        { // remove n bytes from the pbuf chain
            size_t const remainingBytes2Read = n - removedBytes;
            if ((fOffsetInCurrentPBuf + remainingBytes2Read) < fpPBufHead->len)
            { // we remove a part from the pbuf chain head
                fOffsetInCurrentPBuf += remainingBytes2Read;
                removedBytes += remainingBytes2Read;
            }
            else
            { // remove head of pbuf chain
                removedBytes += (fpPBufHead->len - fOffsetInCurrentPBuf);
                fOffsetInCurrentPBuf  = 0;
                pbuf* const pNextPBuf = fpPBufHead->next;
                fpPBufHead->next      = nullptr;
                if (pNextPBuf != nullptr)
                {
                    pNextPBuf->tot_len = fpPBufHead->tot_len - fpPBufHead->len;
                }
                (void)pbuf_free(fpPBufHead);
                fpPBufHead = pNextPBuf;
            }
        }
        return n;
    }
    else
    {
        return 0;
    }
}

AbstractDatagramSocket::ErrorCode LwipDatagramSocket::connect(
    ip::IPAddress const& address, uint16_t port, ip::IPAddress* pLocalAddress)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpTxPcb != nullptr)
    {
        logger::Logger::warn(
            logger::UDP, "LwipDatagramSocket::connect() called although already connected");
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
    }
    uint8_t type = IPADDR_TYPE_ANY;
    if (isIp4Address(address))
    {
        type = IPADDR_TYPE_V4;
    }
    else if (isIp6Address(address))
    {
        type = IPADDR_TYPE_V6;
    }
    else
    {
        logger::Logger::error(
            logger::UDP, "LwipDatagramSocket::connect(): IP address is neither IPv4 nor IPv6!");
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
    }

    fpTxPcb = udp_new_ip_type(type);
    if (fpTxPcb == nullptr)
    {
        logger::Logger::error(logger::UDP, "LwipDatagramSocket::connect(): udp_new failed!");
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
    }
    fpTxPcb->so_options |= SOF_REUSEADDR;
    if (pLocalAddress != nullptr)
    {
        lwiputils::to_lwipIp(*pLocalAddress, &fpTxPcb->local_ip);
    }
    ip_addr_t const remoteIp  = lwiputils::to_lwipIp(address);
    err_t const connectStatus = udp_connect(fpTxPcb, &remoteIp, port);
    if (fpRxPcb != nullptr)
    {
        err_t const bindStatus = udp_bind(fpTxPcb, &fpRxPcb->local_ip, getLocalPort());
        if (bindStatus != ERR_OK)
        {
            logger::Logger::error(
                logger::UDP,
                " LwipDatagramSocket::connect(): udp_bind failed with status %d!",
                bindStatus);
            disconnect();
            return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
        }
        udp_recv(fpTxPcb, &udpReceiveListener, static_cast<void*>(this));
    }
    if (connectStatus != ERR_OK)
    {
        logger::Logger::error(
            logger::UDP,
            " LwipDatagramSocket::connect(): udp_connect failed with status %d!",
            connectStatus);
        // free pcb
        udp_remove(fpTxPcb);
        fpTxPcb = nullptr;
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
    }

    // remoteIp is converted from address, thus it is the same
    char ipAddrBuffer[ip::MAX_IP_STRING_LENGTH];
    logger::Logger::info(
        logger::UDP,
        "DatagramSocket connecting to %s:%d",
        ip::to_str(address, ipAddrBuffer).data(),
        port);
    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
}

void LwipDatagramSocket::disconnect()
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpTxPcb != nullptr)
    {
        IPAddress const remoteIpAddr = lwiputils::from_lwipIp(fpTxPcb->remote_ip);

        char ipAddrBuffer[ip::MAX_IP_STRING_LENGTH];
        logger::Logger::info(
            logger::UDP,
            "DatagramSocket to %s:%d disconnected",
            ip::to_str(remoteIpAddr, ipAddrBuffer).data(),
            getPort());

        udp_remove(fpTxPcb);
        fpTxPcb = nullptr;
    }
}

err_t LwipDatagramSocket::udpWrite(udp_pcb* const pcb, void const* const data, size_t const size)
{
    struct pbuf* const p = pbuf_alloc(PBUF_TRANSPORT, static_cast<u16_t>(size), PBUF_RAM);
    if (p != nullptr)
    {
        // p->payload = (void*)data;
        memcpy(p->payload, data, size); // p is never a pbuf chain if PBUF_RAM is used
        err_t const err = udp_send(pcb, p);
        (void)pbuf_free(p);
        return err;
    }
    return ERR_MEM;
}

AbstractDatagramSocket::ErrorCode LwipDatagramSocket::send(::etl::span<uint8_t const> const& data)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpTxPcb != nullptr)
    {
        err_t const status = udpWrite(fpTxPcb, data.data(), data.size());
        if (status != ERR_OK)
        {
            logger::Logger::error(
                logger::UDP, "LwipDatagramSocket::send() failed (status:0x%x)!", status);
            return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
        }
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
    }
    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
}

LwipDatagramSocket::ErrorCode LwipDatagramSocket::send(DatagramPacket const& packet)
{
    lwiputils::TASK_ASSERT_HOOK();

    netif* pNetif         = nullptr;
    ip_addr_t destination = lwiputils::to_lwipIp(packet.getAddress());

    if (isBound())
    {
        pNetif = ip_route(&fpRxPcb->local_ip, &destination);
    }

    if (pNetif != nullptr)
    {
        struct pbuf* const p = pbuf_alloc(PBUF_TRANSPORT, packet.getLength(), PBUF_RAM);
        if (p == nullptr)
        {
            return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
        }
        (void)memcpy(
            p->payload,
            packet.getData(),
            packet.getLength()); // p is never a pbuf chain if PBUF_RAM is used
        err_t const status = udp_sendto_if(fpRxPcb, p, &destination, packet.getPort(), pNetif);
        (void)pbuf_free(p);
        if (status != 0)
        {
            logger::Logger::error(
                logger::UDP, "LwipDatagramSocket::send() failed (status:0x%x)!", status);
            return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
        }
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
    }
    else
    {
        udp_pcb* const ppcb = udp_new_ip_type(IP_GET_TYPE(&destination));
        if (ppcb == nullptr)
        {
            logger::Logger::error(logger::UDP, " LwipDatagramSocket::send(): udp_new failed!");
            return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
        }
        ppcb->so_options |= SOF_REUSEADDR;

        (void)udp_connect(ppcb, &destination, packet.getPort());
        err_t const status = udpWrite(ppcb, packet.getData(), packet.getLength());
        udp_remove(ppcb);
        if (status != ERR_OK)
        {
            logger::Logger::error(
                logger::UDP, "LwipDatagramSocket::send() failed (status:0x%x)!", status);
            return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK;
        }
        return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
    }
}

void LwipDatagramSocket::close()
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpRxPcb != nullptr)
    {
        logger::Logger::info(logger::UDP, "DatagramSocket @ port %d closed", getLocalPort());
        udp_remove(fpRxPcb);
        fpRxPcb = nullptr;
    }
    for (auto it = fMulticastPcbs.begin(); it != fMulticastPcbs.end(); ++it)
    {
        udp_remove(*it);
    }
    fMulticastPcbs.clear();
}

uint16_t LwipDatagramSocket::getLocalPort() const
{
    if (fpRxPcb != nullptr)
    {
        return fpRxPcb->local_port;
    }
    return AbstractDatagramSocket::INVALID_PORT;
}

ip::IPAddress const* LwipDatagramSocket::getLocalIPAddress() const
{
    static IPAddress localIp;
    if (fpTxPcb != nullptr)
    {
        localIp = lwiputils::from_lwipIp(fpTxPcb->local_ip);
        return &localIp;
    }
    return nullptr;
}

uint16_t LwipDatagramSocket::getPort() const
{
    if (fpTxPcb != nullptr)
    {
        return fpTxPcb->remote_port;
    }
    return AbstractDatagramSocket::INVALID_PORT;
}

ip::IPAddress const* LwipDatagramSocket::getIPAddress() const
{
    if (fpTxPcb != nullptr)
    {
        static IPAddress remoteIpStorage;
        remoteIpStorage = lwiputils::from_lwipIp(fpTxPcb->remote_ip);
        return &remoteIpStorage;
    }
    return nullptr;
}

void LwipDatagramSocket::noChksum(bool value)
{
    uint8_t const flagChecksum = UDP_FLAGS_NOCHKSUM;
    if (value)
    {
        if (fpTxPcb != nullptr)
        {
            fpTxPcb->flags |= flagChecksum;
        }
        if (fpRxPcb != nullptr)
        {
            fpRxPcb->flags |= flagChecksum;
        }
    }
    else
    {
        if (fpTxPcb != nullptr)
        {
            fpTxPcb->flags &= ~flagChecksum;
        }
        if (fpRxPcb != nullptr)
        {
            fpRxPcb->flags &= ~flagChecksum;
        }
    }
}

} // namespace udp
