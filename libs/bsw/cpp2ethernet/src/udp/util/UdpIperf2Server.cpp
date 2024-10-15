// Copyright 2025 Accenture.

#include "udp/util/UdpIperf2Server.h"

#include "bsp/timer/SystemTimer.h"
#include "udp/UdpLogger.h"

#include <etl/math.h>
#include <util/estd/big_endian.h>

namespace udp
{

using ::util::logger::Logger;
using ::util::logger::UDP;

UdpIperf2Server::UdpIperf2Server(
    ip::IPAddress const& ipAddr, uint16_t const rxPort, AbstractDatagramSocket& socket)
: _ipAddr(ipAddr), _rxPort(rxPort), _socket(socket), _receiveData()
{}

bool UdpIperf2Server::start()
{
    Logger::info(UDP, "UDP Iperf2 server initialisation");

    _socket.setDataListener(this);
    if (_socket.bind(&_ipAddr, _rxPort) == ::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK)
    {
        _socket.setDataListener(this);
        Logger::info(UDP, "Listening on port %d.", _rxPort);
        return true;
    }

    return false;
}

void UdpIperf2Server::stop()
{
    Logger::info(UDP, "Closing port %d.", _rxPort);
    _socket.close();
}

namespace
{ // Anonymous namespace

/*
 * The UDPiPerf2ServerResponse structure facilitates the server
 * report of jitter and loss on the client side.
 */

// Data structure format asper iperf version 2.1.5 (3 December 2021)
struct UDPiPerf2ServerResponse
{
    ::etl::array<uint8_t, 24U> header;     // [0-23] bytes
    ::etl::array<uint8_t, 4U> totalBytes;  // [24-27] bytes
    ::etl::array<uint8_t, 4U> totalTimeS;  // [28-31] bytes
    ::etl::array<uint8_t, 4U> totalTimeUs; // [32-35] bytes
    ::etl::array<uint8_t, 4U> packetLoss;  // [36-39] bytes
    ::etl::array<uint8_t, 4U> outOfOrder;  // [40-43] bytes
    ::etl::array<uint8_t, 4U> packetTotal; // [44-47 bytes
    ::etl::array<uint8_t, 4U> jitterS;     // [48-51] bytes
    ::etl::array<uint8_t, 4U> jitterUs;    // [52-55] bytes
};
} // namespace

void UdpIperf2Server::dataReceived(
    ::udp::AbstractDatagramSocket& /*socket*/,
    ::ip::IPAddress sourceAddress,
    uint16_t sourcePort,
    ::ip::IPAddress /*destinationAddress*/,
    uint16_t length)
{
    if (length > sizeof(_receiveData))
    {
        // Discard Data
        (void)_socket.read(nullptr, length);
        Logger::error(UDP, "Received oversized packet.");
    }
    else
    {
        (void)_socket.read(&_receiveData[0U], length);

        int32_t prevSequenceNo = _sequenceNo;
        _sequenceNo = (_receiveData[0] << 24) | (_receiveData[1] << 16) | (_receiveData[2] << 8)
                      | _receiveData[3];

        _totalPacket++;
        _totalBytes += length;

        // TCP packets for test indicated by positive sequence number
        if (_sequenceNo > 0)
        {
            // Start of test indicated by sequence number 1
            if (_sequenceNo == 1)
            {
                _startTimeUs = getSystemTimeUs32Bit();
            }

            // Check for out-of-order packets, considering sequence number wrap-around
            if ((_sequenceNo != (prevSequenceNo + 1))
                && !((prevSequenceNo == INT32_MAX) && (_sequenceNo == 0)))
            {
                _packetOutOfOrder++;
            }
        }
        // End of test indicated by negative sequence number
        else
        {
            // Test Duration calculation
            uint32_t endTimeUs = getSystemTimeUs32Bit();
            uint32_t diffUs    = 0U;

            if (endTimeUs >= _startTimeUs)
            {
                diffUs = endTimeUs - _startTimeUs;
            }
            else
            {
                // Handle overflow
                diffUs = (UINT32_MAX - _startTimeUs + 1) + endTimeUs;
            }

            uint32_t durationS  = diffUs / 1'000'000U;
            uint32_t durationUs = diffUs % 1'000'000U;

            // Lost Packet calculation
            uint32_t packetLoss = 0U;

            // sequenceNo + end of test frame
            if (static_cast<uint32_t>(prevSequenceNo + 1) > _totalPacket)
            {
                packetLoss = static_cast<uint32_t>(prevSequenceNo + 1) - _totalPacket;
            }

            // Fill the value for report (wite in big endian)
            UDPiPerf2ServerResponse response{};
            ::estd::write_be<uint32_t>(&response.totalBytes[0U], _totalBytes);
            ::estd::write_be<uint32_t>(&response.totalTimeS[0U], durationS);
            ::estd::write_be<uint32_t>(&response.totalTimeUs[0U], durationUs);
            ::estd::write_be<uint32_t>(&response.packetLoss[0U], packetLoss);
            ::estd::write_be<uint32_t>(&response.outOfOrder[0U], _packetOutOfOrder);
            ::estd::write_be<uint32_t>(&response.packetTotal[0U], _totalPacket);

            _totalPacket = 0U;
            _totalBytes  = 0U;

            // Send report
            (void)_socket.send(::udp::DatagramPacket(
                reinterpret_cast<uint8_t const*>(&response),
                sizeof(response),
                sourceAddress,
                sourcePort));
            Logger::debug(UDP, "Iperf Server Response send: %x bytes.", sizeof(response));
        }
    }
}

} // namespace udp
