/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/UdpProxy.h"

#include "someip/INetworkListener.h"
#include "someip/NetworkChannel.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <ip/to_str.h>
#include <udp/DatagramPacket.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPAddress;
using ::ip::IPEndpoint;
using ::udp::AbstractDatagramSocket;
using ::udp::DatagramPacket;
using ::util::logger::SOMEIP;

UdpProxy::UdpProxy() : _pSocket(nullptr) {}

bool UdpProxy::isInitialized() const
{
    return NetworkResource::isInitialized() && (_pSocket != nullptr);
}

void UdpProxy::setSocket(AbstractDatagramSocket& socket)
{
    _pSocket = &socket;
    _pSocket->setDataListener(this);
}

AbstractDatagramSocket* UdpProxy::getSocket() const { return _pSocket; }

bool UdpProxy::isOpen() const { return isInitialized() && _pSocket->isBound(); }

bool UdpProxy::isConnected() const { return false; }

::etl::expected<uint16_t, PortError> UdpProxy::getLocalPort() const
{
    if (isInitialized())
    {
        return _pSocket->getLocalPort();
    }

    return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
}

uint8_t UdpProxy::getProto() const { return proto::SD_L4_PROTO_UDP; }

bool UdpProxy::open(IPEndpoint const& localEndpoint)
{
    if (!isInitialized())
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::open() not initialized", this);
        return false;
    }

    if (isOpen())
    {
        return true;
    }

    if (!localEndpoint.isSet())
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::open() no local address", this);
        return false;
    }

    char localEndpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
    INFO_LOG(
        SOMEIP,
        "UdpProxy[%p]::open() at: %s",
        this,
        ::ip::to_str(localEndpoint, localEndpointStr).data());

    AbstractDatagramSocket::ErrorCode const result
        = _pSocket->bind(&localEndpoint.getAddress(), localEndpoint.getPort());

    if (AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK != result)
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::open() bind failed: %d", this, result);
        return false;
    }

    return true;
}

bool UdpProxy::join(IPAddress const& multicastIp)
{
    if (!isInitialized())
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::join() not initialized", this);
        return false;
    }

    if (!isOpen())
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::join() not bound", this);
        return false;
    }

    if (::ip::isUnspecified(multicastIp))
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::join() no multicast address", this);
        close();
        return false;
    }

    char multicastIpStr[::ip::MAX_IP_STRING_LENGTH];
    INFO_LOG(
        SOMEIP,
        "UdpProxy[%p]::join(): joining %s",
        this,
        ::ip::to_str(multicastIp, multicastIpStr).data());

    AbstractDatagramSocket::ErrorCode const result = _pSocket->join(multicastIp);

    if (AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK != result)
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::join() failed: %d", this, result);
        close();
        return false;
    }

    return true;
}

void UdpProxy::close()
{
    if (!isOpen())
    {
        return;
    }

    INFO_LOG(SOMEIP, "UdpProxy[%p]::close()", this);

    _pSocket->close();
}

bool UdpProxy::send(IPEndpoint const& remoteEndpoint, uint32_t const length)
{
    uint8_t const ip0   = remoteEndpoint.getAddress().raw[0];
    uint8_t const ip1   = remoteEndpoint.getAddress().raw[1];
    uint8_t const ip2   = remoteEndpoint.getAddress().raw[2];
    uint8_t const ip3   = remoteEndpoint.getAddress().raw[3];
    uint16_t const port = remoteEndpoint.getPort();

    if (!isOpen())
    {
        ERROR_LOG(
            SOMEIP,
            "UdpProxy[%p]::send() not open. Remote: "
            "(%d,%d,%d,%d), Port: %d.",
            this,
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return false;
    }

    if (length == 0U)
    {
        ERROR_LOG(
            SOMEIP,
            "UdpProxy[%p]::send() no data. Remote: "
            "(%d,%d,%d,%d), Port: %d.",
            this,
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return false;
    }

    if (!remoteEndpoint.isSet())
    {
        ERROR_LOG(
            SOMEIP,
            "UdpProxy[%p]::send() no remote endpoint. Remote: "
            "(%d,%d,%d,%d), Port: %d.",
            this,
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return false;
    }

    if (length > _pOutputBuffer.size())
    {
        ERROR_LOG(
            SOMEIP,
            "UdpProxy[%p]::send() too much data (%d bytes). Remote: "
            "(%d,%d,%d,%d), Port: %d.",
            this,
            length,
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return false;
    }

    DatagramPacket const packet(
        _pOutputBuffer.data(),
        static_cast<uint16_t>(length),
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort());

    AbstractDatagramSocket::ErrorCode const result = _pSocket->send(packet);

    if (AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK != result)
    {
        ERROR_LOG(
            SOMEIP,
            "UdpProxy[%p]::send() failed: %d. Remote: "
            "(%d,%d,%d,%d), Port: %d.",
            this,
            result,
            ip0,
            ip1,
            ip2,
            ip3,
            port);
    }

    return AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK == result;
}

bool UdpProxy::send(uint32_t const length) { return NetworkResource::send(length); }

// virtual
void UdpProxy::dataReceived(
    AbstractDatagramSocket& /* socket */,
    IPAddress const sourceAddress,
    uint16_t const sourcePort,
    IPAddress const destinationAddress,
    uint16_t const length)
{
    if (::ip::isUnspecified(sourceAddress))
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::dataReceived() no source address", this);
        flush(length);
        return;
    }

    if (::ip::isUnspecified(destinationAddress))
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::dataReceived() no destination address", this);
        flush(length);
        return;
    }

    IPEndpoint const remoteEndpoint(sourceAddress, sourcePort);
    bool const isMulticast = ::ip::isMulticastAddress(destinationAddress);

    if (length == 0U)
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::received() no data", this);
        return;
    }

    if (length > _pInputBuffer.size())
    {
        ERROR_LOG(SOMEIP, "UdpProxy[%p]::received() too much data (%d bytes)", this, length);
        flush(length);
        return;
    }

    uint16_t const read
        = static_cast<uint16_t>(_pSocket->read(_pInputBuffer.data(), static_cast<size_t>(length)));

    if (read != length)
    {
        ERROR_LOG(
            SOMEIP, "UdpProxy[%p]::received() read failed (%d / %d bytes)", this, read, length);
        flush(length - read);
        return;
    }
    incRefCounter(); // in case of hanging proxy.
    {
        NetworkChannel channel(*this, remoteEndpoint, isMulticast);
        if (_pListener != nullptr)
        {
            _pListener->received(channel, length);
        }
    }
    decRefCounter();
}

void UdpProxy::flush(uint16_t const length)
{
    if (_pSocket != nullptr)
    {
        (void)_pSocket->read(nullptr, length);
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
