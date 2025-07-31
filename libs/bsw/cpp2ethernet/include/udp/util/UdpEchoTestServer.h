// Copyright 2025 Accenture.

#pragma once

#include "udp/IDataListener.h"
#include "udp/socket/AbstractDatagramSocket.h"

#include <etl/array.h>

namespace udp
{

/**
 * A simple class that listens on a given IP address and port and will echo received data back to
 * the sender.
 */
class UdpEchoTestServer : public ::udp::IDataListener
{
public:
    /**
     * Constructs an instance of UdpEchoTestServer at a given \p ipAddr and \p rxPort.
     * \param ipAddr    IP address of the netif to listen on. IP address 0.0.0.0 will listen on all
     *                  netifs.
     * \param rxPort    Port to listen on.
     */
    UdpEchoTestServer(
        ::ip::IPAddress const& ipAddr, uint16_t rxPort, AbstractDatagramSocket& socket);

    /**
     * Binds to the IP address and port passed in the constructor.
     * \return true if UdpEchoTestServer is successfully started and listening, false otherwise.
     */
    bool start();

    /**
     * Closes the listening socket of this UdpEchoTestServer.
     */
    void stop();

    void dataReceived(
        ::udp::AbstractDatagramSocket& socket,
        ::ip::IPAddress sourceAddress,
        uint16_t sourcePort,
        ::ip::IPAddress destinationAddress,
        uint16_t length) override;

private:
    ip::IPAddress const _ipAddr;
    uint16_t const _rxPort;
    ::udp::AbstractDatagramSocket& _socket;
    static constexpr size_t UDP_RECV_SIZE{1518U};
    ::etl::array<uint8_t, UDP_RECV_SIZE> _receiveData;
};

} // namespace udp
