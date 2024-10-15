// Copyright 2025 Accenture.

#include "lwipSocket/tcp/LwipServerSocket.h"

#include "lwipSocket/tcp/LwipSocket.h"
#include "lwipSocket/utils/LwipHelper.h"

#include <ip/IPAddress.h>
#include <tcp/TcpLogger.h>
#include <tcp/socket/ISocketProvidingConnectionListener.h>

extern "C"
{
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
}

using ip::IPAddress;

namespace tcp
{
namespace logger = ::util::logger;

LwipServerSocket::LwipServerSocket() : AbstractServerSocket(), fpPCB(nullptr) {}

LwipServerSocket::LwipServerSocket(
    uint16_t const port, ISocketProvidingConnectionListener& providingListener)
: AbstractServerSocket(port, providingListener), fpPCB(nullptr)
{}

bool LwipServerSocket::accept()
{
    if ((_port == 0) || (_socketProvidingConnectionListener == nullptr)
        || ((fpPCB != nullptr) && (CLOSED != fpPCB->state)))
    {
        return false;
    }

    if (fpPCB == nullptr)
    {
        fpPCB = tcp_new_ip_type(IPADDR_TYPE_ANY);
        if (fpPCB == nullptr)
        {
            logger::Logger::error(logger::TCP, "LwipServerSocket::bind(): tcp_new() failed");
            return false;
        }
    }
    if (fpPCB->local_port != _port)
    {
        err_t const status = tcp_bind(fpPCB, nullptr, _port);
        if (status != ERR_OK)
        {
            logger::Logger::error(logger::TCP, "LwipServerSocket::accept(): tcp_bind() failed");
            tcp_abort(fpPCB);
            fpPCB = nullptr;
            return false;
        }
    }
    fpPCB = tcp_listen(fpPCB);
    if (fpPCB == nullptr)
    {
        return false;
    }
    tcp_arg(fpPCB, this);
    tcp_accept(fpPCB, &tcpAcceptListener);
    logger::Logger::info(logger::TCP, "Socket prepared at port %d", _port);
    return true;
}

bool LwipServerSocket::bind(IPAddress const& localIpAddress, uint16_t port)
{
    _port = port;
    if ((_port == 0) || (_socketProvidingConnectionListener == nullptr))
    {
        return false;
    }
    if (fpPCB == nullptr)
    {
        fpPCB = tcp_new_ip_type(IPADDR_TYPE_ANY);
        if (fpPCB == nullptr)
        {
            logger::Logger::error(logger::TCP, "LwipServerSocket::bind(): tcp_new() failed");
            return false;
        }
    }
    fpPCB->so_options |= SOF_REUSEADDR;

    err_t status = ERR_ABRT;

    if (!ip::isUnspecified(localIpAddress))
    {
        ip_addr_t const tmpIpAddress = lwiputils::to_lwipIp(localIpAddress);
        status                       = tcp_bind(fpPCB, &tmpIpAddress, _port);
    }
    else
    {
        status = tcp_bind(fpPCB, IP4_ADDR_ANY, _port);
    }

    if (status != ERR_OK)
    {
        logger::Logger::error(logger::TCP, "LwipServerSocket::bind(): tcp_bind() failed");
        // Do not call tcp_close here - bind can be retried in accept function
        return false;
    }
    return true;
}

void LwipServerSocket::close()
{
    if (fpPCB != nullptr)
    {
        logger::Logger::info(logger::TCP, "Socket at port %d closed", _port);
        (void)tcp_close(fpPCB);
        fpPCB = nullptr;
    }
}

bool LwipServerSocket::isClosed() const { return (fpPCB == nullptr); }

err_t LwipServerSocket::tcpAcceptListener(void* const arg, tcp_pcb* const pcb, err_t const result)
{
    if (result != ERR_OK)
    {
        logger::Logger::error(
            logger::TCP, "LwipServerSocket::tcpAcceptListener(): Failed to allocate PCB");
        return ERR_MEM;
    }

    if (arg == nullptr)
    {
        logger::Logger::error(logger::TCP, "LwipServerSocket::tcpAcceptListener(): arg not set!");
        return ERR_ARG;
    }

    LwipServerSocket const* const pServerSocket = static_cast<LwipServerSocket*>(arg);
    estd_assert(pServerSocket->_socketProvidingConnectionListener != nullptr);
    ip::IPAddress const rAddr = lwiputils::from_lwipIp(pcb->remote_ip);

    LwipSocket* const pSocket = static_cast<LwipSocket*>(
        pServerSocket->_socketProvidingConnectionListener->getSocket(rAddr, pcb->remote_port));
    if (pSocket == nullptr)
    {
        logger::Logger::debug(
            logger::TCP,
            "LwipServerSocket::tcpAcceptListener(): SocketProvidingConnectionListener provided no "
            "socket");
        return ERR_MEM;
    }
    logger::Logger::debug(
        logger::TCP, "LwipServerSocket: accepted connection at port %d", pServerSocket->_port);

    pcb->so_options |= SOF_REUSEADDR;

    pSocket->open(pcb);
    tcp_arg(pcb, pSocket);
    tcp_sent(pcb, &LwipSocket::tcpSentListener);
    tcp_recv(pcb, &LwipSocket::tcpReceiveListener);
    tcp_err(pcb, &LwipSocket::tcpErrorListener);
    tcp_poll(pcb, &LwipSocket::tcpPollListener, 1);

    pServerSocket->_socketProvidingConnectionListener->connectionAccepted(*pSocket);
    return ERR_OK;
}

} /*namespace tcp*/
