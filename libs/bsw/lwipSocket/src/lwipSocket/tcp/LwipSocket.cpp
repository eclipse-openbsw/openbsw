// Copyright 2025 Accenture.

#include "lwipSocket/tcp/LwipSocket.h"

#include "lwipSocket/utils/LwipHelper.h"
#include "lwipSocket/utils/TaskAssert.h"

#include <ip/to_str.h>
#include <tcp/IDataListener.h>
#include <tcp/IDataSendNotificationListener.h>
#include <tcp/TcpLogger.h>

extern "C"
{
// C includes
#include "lwip/ip.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/tcp.h"
}

namespace
{
err_t const ERR_NO_MORE_BUF = -100;
}

namespace tcp
{
namespace logger = ::util::logger;

using ::ip::IPAddress;
using ::ip::IPEndpoint;
using ::ip::make_ip4;
#if LWIP_IPV6
using ::ip::make_ip6;
#endif

LwipSocket::LwipSocket()
: fpHandle(nullptr)
, fpPBufHead(nullptr)
, fBindEndpoint()
, fOffsetInCurrentPBuf(0)
, fPendingTcpData()
, fDelegate()
, fConnecting(false)
, fIsAborted(false)
, fForceCopy(false)
#if LWIP_TCP_KEEPALIVE
, fKeepAliveEnabled(false)
, fKeepAliveIdle(KEEPALIVE_IDLE_DEFAULT)
, fKeepAliveInterval(KEEPALIVE_INTERVAL_DEFAULT)
, fKeepAliveProbes(KEEPALIVE_PROBES_DEFAULT)
#endif
{}

void LwipSocket::setForceCopy(bool const forceCopy) { fForceCopy = forceCopy; }

AbstractSocket::ErrorCode LwipSocket::send(::etl::span<uint8_t const> const& data)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (!isEstablished())
    {
        logger::Logger::warn(logger::TCP, "LwipSocket::send() called on closed or closing socket!");
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OPEN;
    }
    else
    {
        if ((0L == fPendingTcpData.size()) && (data.size() > 0U))
        { // no data is pending
            logger::Logger::debug(
                logger::TCP,
                "LwipSocket::send(%x, %u) no data is pending!",
                data.data(),
                data.size());
            if (available() > 0)
            {
                fPendingTcpData = data;

                err_t const e = sendPendingData(fpHandle);
                if (e == ERR_OK)
                {
                    return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
                }
                else if (e == ERR_NO_MORE_BUF)
                {
                    // wait for send ack before return
                    return AbstractSocket::ErrorCode::SOCKET_ERR_NO_MORE_BUFFER;
                }
                else if (e == ERR_MEM)
                {
                    fPendingTcpData = {};
                    // flush and wait for send ack before return
                    return AbstractSocket::ErrorCode::SOCKET_FLUSH;
                }
                else
                {
                    fPendingTcpData = {};
                    return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
                }
            }
            else
            {
                // wait for send ack before retry
                return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
            }
        }
        else
        {
            return AbstractSocket::ErrorCode::SOCKET_FLUSH;
        }
    }
}

err_t LwipSocket::tcpSentListener(void* const arg, tcp_pcb* const pcb, uint16_t const len)
{
    logger::Logger::debug(logger::TCP, "LwipSocket::tcpSentListener(%x, %x, %d);", arg, pcb, len);
    if (arg == nullptr)
    {
        logger::Logger::critical(
            logger::TCP, "LwipSocket::tcpSentListener(): arg must not be NULL!");
        return ERR_ARG;
    }

    LwipSocket* const pSocket = static_cast<LwipSocket*>(arg);
    return pSocket->sentCallback(pcb, len);
}

err_t LwipSocket::sentCallback(tcp_pcb* const pcb, uint16_t const len)
{
    if (len > 0)
    {
        if (_sendNotificationListener != nullptr)
        {
            _sendNotificationListener->dataSent(
                len, IDataSendNotificationListener::SendResult::DATA_SENT);
        }
    }

    // now there might be space left for sending the next stuff
    return sendPendingData(pcb);
}

err_t LwipSocket::sendPendingData(tcp_pcb* const pcb)
{
    if (fPendingTcpData.size() == 0L)
    {
        return checkResult(ERR_OK);
    }

    size_t const availableBuffer = static_cast<size_t>(tcp_sndbuf(pcb));
    if (availableBuffer == 0U)
    {
        return checkResult(ERR_MEM);
    }

    size_t const pendingBytes = fPendingTcpData.size();
    bool const sendAll        = (pendingBytes <= availableBuffer) || fForceCopy;
    size_t const bytesToWrite = sendAll ? pendingBytes : availableBuffer;

    logger::Logger::debug(
        logger::TCP,
        "LwipSocket::tcpSentListener() on %x data:%x size:%d buffersize:%d",
        this,
        fPendingTcpData.data(),
        pendingBytes,
        availableBuffer);

    estd_assert(bytesToWrite <= UINT16_MAX);
    err_t result = tcp_write(pcb, fPendingTcpData.data(), static_cast<uint16_t>(bytesToWrite), 1);
    if (result == ERR_OK)
    {
        if (sendAll)
        {
            fPendingTcpData = {};
        }
        else
        {
            fPendingTcpData.advance(bytesToWrite);
            result = ERR_NO_MORE_BUF;
        }

        if (_sendNotificationListener != nullptr)
        {
            _sendNotificationListener->dataSent(
                static_cast<uint16_t>(bytesToWrite),
                IDataSendNotificationListener::SendResult::DATA_QUEUED);
        }
    }

    return checkResult(result);
}

err_t LwipSocket::tcpReceiveListener(
    void* const arg, tcp_pcb* const pcb, pbuf* const p, err_t const result)
{
    if (arg == nullptr)
    {
        logger::Logger::critical(
            logger::TCP, "LwipSocket::tcpReceiveListener(): arg must not be NULL!");
        return ERR_ARG;
    }

    LwipSocket* const pSocket = static_cast<LwipSocket*>(arg);
    return pSocket->receiveCallback(pcb, p, result);
}

err_t LwipSocket::receiveCallback(tcp_pcb const* const pcb, pbuf* const p, err_t const result)
{
    if (p == nullptr)
    {
        logger::Logger::info(
            logger::TCP, "LwipSocket: connection at port %d closed", pcb->local_port);
        if (fpHandle != nullptr)
        {
            (void)tcp_close(fpHandle);
            resetSocket();
        }

        if (_dataListener != nullptr)
        {
            _dataListener->connectionClosed(IDataListener::ErrorCode::ERR_CONNECTION_CLOSED);
        }
        else
        {
            logger::Logger::warn(logger::TCP, "No connectionClosed listener registered!");
        }
        discardData();
    }
    else
    {
        if (fpPBufHead != nullptr)
        {
            pbuf_cat(fpPBufHead, p);
        }
        else
        {
            fpPBufHead           = p;
            fOffsetInCurrentPBuf = 0;
        }

        size_t const readLen = fpPBufHead->tot_len - fOffsetInCurrentPBuf;

        if (_dataListener != nullptr)
        {
            _dataListener->dataReceived(static_cast<uint16_t>(readLen));
        }
        else
        {
            logger::Logger::error(
                logger::TCP, "LwipSocket::tcpReceiveListener(): no datalistener!");
            (void)read(nullptr, readLen);
        }
    }

    return checkResult(result);
}

err_t LwipSocket::tcpConnectedListener(void* const arg, tcp_pcb* const pcb, err_t const result)
{
    if (arg == nullptr)
    {
        logger::Logger::critical(
            logger::TCP, "LwipSocket::tcpReceiveListener(): arg must not be NULL!");
        return ERR_ARG;
    }

    LwipSocket* const pSocket = static_cast<LwipSocket*>(arg);
    return pSocket->connectCallback(pcb, result);
}

err_t LwipSocket::connectCallback(tcp_pcb const* const /*pcb*/, err_t const result)
{
    logger::Logger::debug(logger::TCP, "LwipSocket::tcpConnectedListener(%d)", result);
    fConnecting = false;
    fDelegate(
        (result == ERR_OK) ? AbstractSocket::ErrorCode::SOCKET_ERR_OK
                           : AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK);
    return checkResult(ERR_OK);
}

AbstractSocket::ErrorCode
LwipSocket::connect(IPAddress const& ipAddr, uint16_t const port, ConnectedDelegate delegate)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpHandle != nullptr)
    {
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
    }

    if (ip::isUnspecified(ipAddr))
    {
        logger::Logger::error(
            logger::TCP, "LwipSocket::connect(): IP address is neither IPv4 nor IPv6!");
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
    }

    fpHandle = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (fpHandle == nullptr)
    {
        logger::Logger::error(logger::TCP, "LwipSocket::connect(): tcp_new() failed");
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
    }

    fPendingTcpData           = {};
    ip_addr_t const connectIp = lwiputils::to_lwipIp(ipAddr);

    fDelegate   = delegate;
    fConnecting = true;
    fIsAborted  = false;
    tcp_arg(fpHandle, this);
    tcp_sent(fpHandle, &tcpSentListener);
    tcp_recv(fpHandle, &tcpReceiveListener);
    tcp_err(fpHandle, &tcpErrorListener);
    tcp_poll(fpHandle, &tcpPollListener, 1U);
#if LWIP_TCP_KEEPALIVE
    setKeepAlive();
#endif
    if (fBindEndpoint.isSet())
    {
        ip_addr_t const tmpIpAddress = lwiputils::to_lwipIp(fBindEndpoint.getAddress());
        err_t const bindStatus       = tcp_bind(fpHandle, &tmpIpAddress, fBindEndpoint.getPort());
        if (bindStatus != ERR_OK)
        {
            logger::Logger::error(
                logger::TCP,
                "LwipSocket::connect(): tcp_bind() failed with status: %d",
                bindStatus);
            tcp_abandon(fpHandle, 0);
            resetSocket();
            return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
        }
    }
    err_t const connectStatus = tcp_connect(fpHandle, &connectIp, port, &tcpConnectedListener);
    if (connectStatus != ERR_OK)
    {
        logger::Logger::error(logger::TCP, "LwipSocket::connect(): tcp_connect() failed");
        abort();
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
    }

    // connectIp is converted from ipAddr, thus it is the same
    char ipAddrBuffer[ip::MAX_IP_STRING_LENGTH];
    logger::Logger::info(
        logger::TCP, "Socket connecting to %s:%d", ip::to_str(ipAddr, ipAddrBuffer).data(), port);

    return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
}

size_t LwipSocket::available()
{
    if (!isEstablished())
    {
        return 0;
    }
    return tcp_sndbuf(fpHandle);
}

bool LwipSocket::isClosed() const
{
    if (fpHandle != nullptr)
    {
        return (CLOSED == fpHandle->state);
    }
    return true;
}

bool LwipSocket::isEstablished() const
{
    if (fpHandle != nullptr)
    {
        return (ESTABLISHED == fpHandle->state);
    }
    return false;
}

void LwipSocket::open(tcp_pcb* const handle)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (isClosed())
    {
        fpHandle             = handle;
        fIsAborted           = false;
        fpPBufHead           = nullptr;
        fOffsetInCurrentPBuf = 0;
        fPendingTcpData      = {};
#if LWIP_TCP_KEEPALIVE
        setKeepAlive();
#endif
    }
    else
    {
        logger::Logger::error(logger::TCP, "LwipSocket::open() called in illegal state != CLOSED");
    }
}

AbstractSocket::ErrorCode LwipSocket::shutdown(int32_t shut_rx, int32_t shut_tx)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (isEstablished())
    {
        if (shut_rx != 0)
        {
            discardData();
        }
        if (tcp_shutdown(fpHandle, shut_rx, shut_tx) != ERR_OK)
        {
            return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
        }
        return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
    }
    return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OPEN;
}

AbstractSocket::ErrorCode LwipSocket::close()
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpHandle == nullptr)
    {
        return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
    }

    discardData();

    // From this point the socket PCB is handled internally by the TCP stack
    tcp_err(fpHandle, nullptr);
    tcp_recv(fpHandle, nullptr);
    tcp_sent(fpHandle, nullptr);
    tcp_poll(fpHandle, nullptr, 0U);

    if (tcp_close(fpHandle) != ERR_OK)
    {
        abort();
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
    }

    resetSocket();
    return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
}

void LwipSocket::abort()
{
    if ((fpHandle != nullptr) && (fpHandle->state != CLOSED))
    {
        fIsAborted = true;
        discardData();
        tcp_abort(fpHandle);
        resetSocket();
    }
}

AbstractSocket::ErrorCode LwipSocket::flush()
{
    lwiputils::TASK_ASSERT_HOOK();

    if (isEstablished())
    {
        bool const isNagleActive = !tcp_nagle_disabled(fpHandle);
        if (isNagleActive)
        {
            tcp_nagle_disable(fpHandle);
        }
        err_t const status = tcp_output(fpHandle);
        if (isNagleActive)
        {
            tcp_nagle_enable(fpHandle);
        }
        if (status != ERR_OK)
        {
            return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
        }
        return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
    }
    else
    {
        return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OPEN;
    }
}

uint8_t LwipSocket::read(uint8_t& byte)
{
    uint8_t value[1];
    if (1U == read(&value[0], 1U))
    {
        byte = value[0];
        return 1U;
    }
    else
    {
        return 0U;
    }
}

size_t LwipSocket::read(uint8_t* buffer, size_t n)
{
    lwiputils::TASK_ASSERT_HOOK();

    if ((fpPBufHead != nullptr) && ((fpPBufHead->tot_len - fOffsetInCurrentPBuf) >= n))
    {
        if (buffer != nullptr)
        {
            (void)pbuf_copy_partial(
                fpPBufHead,
                buffer,
                static_cast<uint16_t>(n),
                static_cast<uint16_t>(fOffsetInCurrentPBuf));
        }
        size_t removedBytes = 0;
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
                removedBytes += fpPBufHead->len - fOffsetInCurrentPBuf;
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
        if (fpHandle != nullptr)
        {
            tcp_recved(fpHandle, static_cast<uint16_t>(n));
        }
        return n;
    }
    else
    {
        return 0;
    }
}

void LwipSocket::discardData()
{
    if ((fpPBufHead != nullptr) && (fpPBufHead->ref > 0))
    {
        (void)pbuf_free(fpPBufHead);
        fpPBufHead = nullptr;
    }
}

err_t LwipSocket::tcpPollListener(void* const arg, tcp_pcb* const pcb)
{
    if ((pcb != nullptr) && (pcb->state == CLOSE_WAIT))
    {
        (void)tcp_close(pcb);
    }

    LwipSocket* const pSocket = static_cast<LwipSocket*>(arg);
    (void)pSocket->sendPendingData(pcb);

    return ERR_OK;
}

void LwipSocket::tcpErrorListener(void* const arg, err_t const result)
{
    if (arg == nullptr)
    {
        logger::Logger::critical(
            logger::TCP, "LwipSocket::tcpErrorListener(): arg must not be NULL!");
        return;
    }

    LwipSocket* const pSocket = static_cast<LwipSocket*>(arg);
    pSocket->errorCallback(result);
}

void LwipSocket::errorCallback(err_t const result)
{
    // clear the handle in any error case
    // lwip documentation of tcp_err_fn is clear, that the application is not allowed
    // to use the pcb anymore if the error callback was called
    resetSocket();

    if (_dataListener != nullptr)
    {
        IDataListener::ErrorCode err;
        switch (result)
        {
            case ERR_RST:
                logger::Logger::debug(logger::TCP, "LwipSocket: connection reset");
                err = IDataListener::ErrorCode::ERR_CONNECTION_RESET;
                break;
            case ERR_ABRT:
                logger::Logger::debug(logger::TCP, "LwipSocket: connection aborted");
                err = IDataListener::ErrorCode::ERR_UNKNOWN;
                break;
            case ERR_CLSD:
                logger::Logger::debug(
                    logger::TCP,
                    "LwipSocket: Connection closed although the application has only shut down the "
                    "tx side ");
                err = IDataListener::ErrorCode::ERR_UNKNOWN;
                break;
            default:
                logger::Logger::warn(logger::TCP, "LwipSocket::tcpErrorListener(0x%x)", result);
                err = IDataListener::ErrorCode::ERR_UNKNOWN;
                break;
        }
        _dataListener->connectionClosed(err);
    }

    // TODO only do this when fConnecting was set, else call the connectionClosed() callback
    if (fConnecting)
    {
        fConnecting = false;
        fDelegate(AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK);
    }
    discardData();
}

void LwipSocket::setNagle(bool const enable)
{
    if (fpHandle != nullptr)
    {
        if (enable)
        {
            fpHandle->flags &= ~TF_NODELAY;
        }
        else
        {
            fpHandle->flags |= TF_NODELAY;
        }
    }
}

IPAddress LwipSocket::getRemoteIPAddress() const
{
    if (fpHandle != nullptr)
    {
        return lwiputils::from_lwipIp(fpHandle->remote_ip);
    }
    return IPAddress();
}

IPAddress LwipSocket::getLocalIPAddress() const
{
    if (fpHandle != nullptr)
    {
        return lwiputils::from_lwipIp(fpHandle->local_ip);
    }
    return IPAddress();
}

uint16_t LwipSocket::getRemotePort() const
{
    return (fpHandle != nullptr) ? fpHandle->remote_port : 0U;
}

uint16_t LwipSocket::getLocalPort() const
{
    return (fpHandle != nullptr) ? fpHandle->local_port : 0U;
}

AbstractSocket::ErrorCode LwipSocket::bind(IPAddress const& ipAddr, uint16_t const port)
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpHandle == nullptr)
    {
        fBindEndpoint = IPEndpoint(ipAddr, port);
        return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
    }
    return AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK;
}

void LwipSocket::disableNagleAlgorithm()
{
    lwiputils::TASK_ASSERT_HOOK();

    setNagle(false);
}

void LwipSocket::resetSocket()
{
    lwiputils::TASK_ASSERT_HOOK();

    if (fpHandle != nullptr)
    {
        tcp_arg(fpHandle, nullptr);
        tcp_sent(fpHandle, nullptr);
        tcp_recv(fpHandle, nullptr);
        tcp_err(fpHandle, nullptr);
        tcp_poll(fpHandle, nullptr, 0U);
        fpHandle = nullptr;
    }
}

err_t LwipSocket::checkResult(err_t const error) const
{
    if (fIsAborted)
    {
        return ERR_ABRT;
    }
    else
    {
        return error;
    }
}

void LwipSocket::enableKeepAlive(
    uint32_t const idle, uint32_t const interval, uint32_t const probes)
{
#if LWIP_TCP_KEEPALIVE
    fKeepAliveEnabled  = true;
    fKeepAliveIdle     = idle;
    fKeepAliveInterval = interval;
    fKeepAliveProbes   = probes;

    setKeepAlive();
#else
    static_cast<void>(idle);
    static_cast<void>(interval);
    static_cast<void>(probes);
#endif
}

void LwipSocket::disableKeepAlive()
{
#if LWIP_TCP_KEEPALIVE
    fKeepAliveEnabled  = false;
    fKeepAliveIdle     = KEEPALIVE_IDLE_DEFAULT;
    fKeepAliveInterval = KEEPALIVE_INTERVAL_DEFAULT;
    fKeepAliveProbes   = KEEPALIVE_PROBES_DEFAULT;

    setKeepAlive();
#else
    static_cast<void>(0); // nop
#endif
}

#if LWIP_TCP_KEEPALIVE
void LwipSocket::setKeepAlive()
{
    if (fpHandle == nullptr)
    {
        return;
    }

    if (fKeepAliveEnabled)
    {
        fpHandle->so_options |= SOF_KEEPALIVE;
    }
    else
    {
        fpHandle->so_options &= ~SOF_KEEPALIVE;
    }

    fpHandle->keep_idle  = fKeepAliveIdle;
    fpHandle->keep_intvl = fKeepAliveInterval;
    fpHandle->keep_cnt   = fKeepAliveProbes;
}
#endif

} // namespace tcp
