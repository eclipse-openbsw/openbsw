// Copyright 2025 Accenture.

#pragma once

#include "lwip/err.h"

#include <ip/IPEndpoint.h>
#include <tcp/IDataListener.h>
#include <tcp/socket/AbstractSocket.h>

#include <platform/estdint.h>

extern "C"
{
#include "lwip/arch.h"
#include "lwip/tcp.h"

struct tcp_pcb;
struct pbuf;
struct ip_addr;
}

namespace tcp
{
/**
 * Socket implementation for tcp stack of LWIP.
 *
 *
 * \see AbstractSocket
 */
class LwipSocket : public AbstractSocket
{
    LwipSocket(LwipSocket const&) = delete;

public:
    LwipSocket();

    virtual ~LwipSocket() {}

    void setForceCopy(bool forceCopy);

    void open(tcp_pcb* handle);

    // AbstractSocket
    ErrorCode
    connect(ip::IPAddress const& ipAddr, uint16_t port, ConnectedDelegate delegate) override;

    virtual ErrorCode shutdown(int32_t shut_rx, int32_t shut_tx);

    ErrorCode close() override;

    void abort() override;

    ErrorCode flush() override;

    uint8_t read(uint8_t& byte) override;

    size_t read(uint8_t* buffer, size_t n) override;

    void discardData() override;

    ErrorCode send(::etl::span<uint8_t const> const& data) override;

    size_t available() override;

    bool isClosed() const override;

    bool isEstablished() const override;

    void setNagle(bool enable);

    ErrorCode bind(ip::IPAddress const& ipAddr, uint16_t const port) override;

    ip::IPAddress getRemoteIPAddress() const override;

    ip::IPAddress getLocalIPAddress() const override;

    uint16_t getRemotePort() const override;

    uint16_t getLocalPort() const override;

    void disableNagleAlgorithm() override;

    void enableKeepAlive(uint32_t idle, uint32_t interval, uint32_t probes) override;

    void disableKeepAlive() override;

private:
    friend class LwipServerSocket;

    void resetSocket();

    err_t sendPendingData(tcp_pcb* pcb);

    err_t sentCallback(tcp_pcb* pcb, uint16_t len);

    err_t receiveCallback(tcp_pcb const* pcb, pbuf* p, err_t result);

    err_t connectCallback(tcp_pcb const* pcb, err_t result);

    void errorCallback(err_t result);

    err_t checkResult(err_t error) const;

    void notifyConnectionClosed(IDataListener::ErrorCode errorCode);

    // TCP stack callback functions
    static err_t tcpSentListener(void* arg, tcp_pcb* pcb, uint16_t len);
    static void tcpErrorListener(void* arg, err_t result);

    static err_t tcpReceiveListener(void* arg, tcp_pcb* pcb, pbuf* p, err_t result);

    static err_t tcpConnectedListener(void* arg, tcp_pcb* pcb, err_t result);

    static err_t tcpPollListener(void* arg, tcp_pcb* pcb);

#if LWIP_TCP_KEEPALIVE
    void setKeepAlive();
#endif

    tcp_pcb* fpHandle;
    pbuf* fpPBufHead;
    ::ip::IPEndpoint fBindEndpoint;
    size_t fOffsetInCurrentPBuf;
    ::etl::span<uint8_t const> fPendingTcpData;
    ConnectedDelegate fDelegate;
    bool fConnecting;
    bool fIsAborted;
    bool fForceCopy;

#if LWIP_TCP_KEEPALIVE
    static constexpr uint32_t KEEPALIVE_IDLE_DEFAULT     = 7200000U;
    static constexpr uint32_t KEEPALIVE_INTERVAL_DEFAULT = 75000U;
    static constexpr uint32_t KEEPALIVE_PROBES_DEFAULT   = 9U;

    bool fKeepAliveEnabled;
    uint32_t fKeepAliveIdle;
    uint32_t fKeepAliveInterval;
    uint32_t fKeepAliveProbes;
#endif
};

} // namespace tcp
