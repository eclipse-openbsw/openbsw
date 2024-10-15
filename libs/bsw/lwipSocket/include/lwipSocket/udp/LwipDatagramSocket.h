// Copyright 2025 Accenture.

#pragma once

#include <etl/vector.h>
#include <ip/IPAddress.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <platform/estdint.h>

extern "C"
{
#include "lwip/arch.h"
#include "lwip/ip.h"

struct udp_pcb;
struct pbuf;
struct netif;
}

namespace udp
{
/**
 * Implementation of AbstractDatagramSocket using LWIP TCP/IP Stack API.
 *
 *
 * \see AbstractDatagramSocket
 */
class LwipDatagramSocket : public AbstractDatagramSocket
{
public:
    LwipDatagramSocket();

    virtual ~LwipDatagramSocket() {}

    /**
     * \see AbstractDatagramSocket::bind();
     */
    ErrorCode bind(ip::IPAddress const* pIpAddress, uint16_t port) override;

    /**
     * \see AbstractDatagramSocket::join();
     */
    ErrorCode join(ip::IPAddress const& groupAddr) override;

    /**
     * \see AbstractDatagramSocket::isBound();
     */
    bool isBound() const override;

    /**
     * \see AbstractDatagramSocket::close();
     */
    void close() override;

    /**
     * \see AbstractDatagramSocket::isClosed();
     */
    bool isClosed() const override;

    /**
     * \see AbstractDatagramSocket::connect();
     */
    ErrorCode
    connect(ip::IPAddress const& address, uint16_t port, ip::IPAddress* pLocalAddress) override;

    /**
     * \see AbstractDatagramSocket::disconnect();
     */
    void disconnect() override;

    /**
     * \see AbstractDatagramSocket::isConnected();
     */
    bool isConnected() const override;

    /**
     * \see AbstractDatagramSocket::read();
     */
    size_t read(uint8_t* buffer, size_t n) override;

    /**
     * \see AbstractDatagramSocket::send();
     */
    ErrorCode send(::etl::span<uint8_t const> const& data) override;

    /**
     * \see AbstractDatagramSocket::send();
     */
    ErrorCode send(DatagramPacket const& packet) override;

    /**
     * \see AbstractDatagramSocket::getIPAddress();
     */
    ip::IPAddress const* getIPAddress() const override;

    /**
     * \see AbstractDatagramSocket::getLocalIPAddress();
     */
    ip::IPAddress const* getLocalIPAddress() const override;

    /**
     * \see AbstractDatagramSocket::getPort();
     */
    uint16_t getPort() const override;

    /**
     * \see AbstractDatagramSocket::getLocalPort();
     */
    uint16_t getLocalPort() const override;

    virtual void noChksum(bool value);

private:
    enum
    {
        UDP_REFERENCE_DATA = 0,
        UDP_COPY_DATA      = 1
    };

    static uint8_t const NUM_MULTICAST_CONNECTIONS = 3;

    bool isAlreadyJoined(ip::IPAddress const& ip) const;

    static err_t udpWrite(udp_pcb* pcb, void const* data, size_t size);

    static void udpReceiveListener(
        void* arg, udp_pcb* pcb, struct pbuf* p, ip_addr_t const* src_ip, uint16_t src_port);

    udp_pcb* fpRxPcb;
    udp_pcb* fpTxPcb;
    pbuf* fpPBufHead;
    size_t fOffsetInCurrentPBuf;
    ::etl::vector<udp_pcb*, NUM_MULTICAST_CONNECTIONS> fMulticastPcbs;
};

} // namespace udp
