// Copyright 2025 Accenture.

#pragma once

#include <ip/IPAddress.h>
#include <tcp/socket/AbstractServerSocket.h>

#include <platform/estdint.h>

extern "C"
{
#include "lwip/tcp.h"

struct tcp_pcb;
}

namespace tcp
{
class AbstractSocket;

/**
 * Implementation of AbstractServerSocket for LWIP TCP/IP stack.
 *
 *
 * \see AbstractServerSocket
 */
class LwipServerSocket : public AbstractServerSocket
{
    LwipServerSocket(LwipServerSocket const&) = delete;

public:
    LwipServerSocket();

    /**
     * constructor
     * \param port    the port to open
     * \param providingListener   ISocketProvidingConnectionListener attached
     *          to this LwipServerSocket
     */
    LwipServerSocket(uint16_t port, ISocketProvidingConnectionListener& providingListener);

    virtual ~LwipServerSocket() {}

    /**
     * \see AbstractServerSocket::accept()
     */
    bool accept() override;

    /**
     * \see AbstractServerSocket::bind()
     */
    bool bind(ip::IPAddress const& localIpAddress, uint16_t port) override;

    /**
     * \see AbstractServerSocket::close()
     */
    void close() override;

    bool isClosed() const override;

private:
    /**
     * Callback for LWIP TCP/IP stack
     */
    static err_t tcpAcceptListener(void* arg, tcp_pcb* pcb, err_t result);

    tcp_pcb* fpPCB;
};

} // namespace tcp
