/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpProxy.h"

#include "someip/INetworkListener.h"
#include "someip/NetworkChannel.h"
#include "someip/SomeIpMessage.h"
#include "someip/logger.h"

#include <ip/to_str.h>

#include <etl/algorithm.h>
#include <etl/error_handler.h>

#include <tuple>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::tcp::AbstractSocket;
using ::util::logger::SOMEIP;

TcpProxy::TcpProxy(::tcp::AbstractSocket& socket)
: _socket(socket)
, _pConnectionListener(nullptr)
, _pParentServerListener(nullptr)
, _pBuffer()
, _bufferOffset(0U)
, _dynamicLocalPort(0U)
{
    _socket.setDataListener(this);
}

bool TcpProxy::isInitialized() const { return NetworkResource::isInitialized(); }

AbstractSocket& TcpProxy::getSocket() const { return _socket; }

void TcpProxy::setConnectionListener(IConnectionListener* const listener)
{
    _pConnectionListener = listener;
}

bool TcpProxy::isOpen() const { return !_socket.isClosed(); }

bool TcpProxy::isConnected() const { return isInitialized() && _socket.isEstablished(); }

bool TcpProxy::isIdle() const
{
    return ((isInitialized()) && (_pParentServerListener == nullptr) && (!isOpen()));
}

::etl::expected<uint16_t, PortError> TcpProxy::getLocalPort() const
{
    if (isInitialized())
    {
        return _socket.getLocalPort();
    }

    return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
}

uint8_t TcpProxy::getProto() const { return proto::SD_L4_PROTO_TCP; }

::ip::IPEndpoint TcpProxy::getRemoteEndpoint() const
{
    if (isInitialized())
    {
        return ::ip::IPEndpoint(_socket.getRemoteIPAddress(), _socket.getRemotePort());
    }

    return NetworkResource::INVALID_ADDRESS;
}

bool TcpProxy::open(::ip::IPEndpoint const& localEndpoint, ::ip::IPEndpoint const& remoteEndpoint)
{
    if (_pBuffer.size() == 0U)
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::open() no reassembly buffer", this);
        return false;
    }

    if (!isInitialized())
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::open() not initialized", this);
        return false;
    }

    if (isOpen())
    {
        return true;
    }

    if (!localEndpoint.isSet())
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::open() no local endpoint", this);
        return false;
    }

    if (!remoteEndpoint.isSet())
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::open() no remote endpoint", this);
        return false;
    }

    char localEndpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
    char remoteEndpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
    INFO_LOG(
        SOMEIP,
        "TcpProxy[%p]::open() at: %s, to: %s",
        this,
        ::ip::to_str(localEndpoint, localEndpointStr).data(),
        ::ip::to_str(remoteEndpoint, remoteEndpointStr).data());

    // We need to make modifiable copy of localEndpoint
    ::ip::IPEndpoint modLocalEndpoint(localEndpoint);

    PortRangeReturnCode rc;
    etl::tie(_dynamicLocalPort, rc)
        = computeNextLocalPort(modLocalEndpoint.getPort(), _dynamicLocalPort);
    if (rc == PortRangeReturnCode::ERROR_REQUESTED_PORT_OUT_OF_RANGE)
    {
        ERROR_LOG(SOMEIP, "Requested port is not in a list of available port ranges");
        return false;
    }

    if (rc == PortRangeReturnCode::WARNING_INVALID_CURRENT_PORT)
    {
        WARN_LOG(
            SOMEIP,
            "Port passed to port range function was invalid, but valid port should be returned.");
    }

    modLocalEndpoint.setPort(_dynamicLocalPort);

    AbstractSocket::ErrorCode result
        = _socket.bind(modLocalEndpoint.getAddress(), modLocalEndpoint.getPort());

    if (AbstractSocket::ErrorCode::SOCKET_ERR_OK != result)
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::open() bind failed: %d", this, result);
        return false;
    }

    _socket.disableNagleAlgorithm();

    result = _socket.connect(
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        AbstractSocket::ConnectedDelegate::create<TcpProxy, &TcpProxy::connected>(*this));

    if (AbstractSocket::ErrorCode::SOCKET_ERR_OK != result)
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::open() connect failed: %d", this, result);
        (void)_socket.close();
        return false;
    }

    return true;
}

void TcpProxy::close()
{
    if ((!isInitialized()) || (!isOpen()))
    {
        return;
    }

    INFO_LOG(SOMEIP, "TcpProxy[%p]: close", this);

    AbstractSocket::ErrorCode const result = _socket.close();
    if (AbstractSocket::ErrorCode::SOCKET_ERR_OK != result)
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::close() failed: %d", this, result);
    }

    closeConnection();
}

void TcpProxy::abort()
{
    if (!isOpen())
    {
        return;
    }

    INFO_LOG(SOMEIP, "TcpProxy[%p]: abort", this);
    _socket.abort();
    closeConnection();
}

bool TcpProxy::send(::ip::IPEndpoint const& remoteEndpoint, uint32_t const length)
{
    return NetworkResource::send(remoteEndpoint, length);
}

bool TcpProxy::send(uint32_t length)
{
    if (!isConnected())
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::send() not connected", this);
        return false;
    }

    if (length == 0U)
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::send() no data", this);
        return false;
    }

    if (length > _pOutputBuffer.size())
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::send() too much data (%d bytes)", this, length);
        return false;
    }

    uint32_t offset = 0U;
    while (length > 0U)
    {
        uint32_t const chunk = (length > UINT16_MAX) ? static_cast<uint32_t>(UINT16_MAX) : length;

        AbstractSocket::ErrorCode const result = _socket.send(
            ::etl::span<uint8_t const>(&_pOutputBuffer[offset], static_cast<uint16_t>(chunk)));

        if (AbstractSocket::ErrorCode::SOCKET_ERR_OK != result)
        {
            ERROR_LOG(SOMEIP, "TcpProxy[%p]::send() failed %d", this, result);
            return false;
        }
        offset += chunk;
        length -= chunk;
    }

    AbstractSocket::ErrorCode const flushResult = _socket.flush();
    if (AbstractSocket::ErrorCode::SOCKET_ERR_OK != flushResult)
    {
        ERROR_LOG(SOMEIP, "TcpProxy[%p]::send() flush failed %d", this, flushResult);
        return false;
    }

    return true;
}

void TcpProxy::dataReceived(uint16_t length)
{
    ::ip::IPEndpoint const endpoint(_socket.getRemoteIPAddress(), _socket.getRemotePort());

    while (length > 0U)
    {
        uint32_t const freeBytes = static_cast<uint32_t>(_pBuffer.size() - _bufferOffset);

        if (freeBytes == 0U)
        {
            ERROR_LOG(
                SOMEIP,
                "TcpProxy[%p]::received() buffer exhausted (%d bytes)",
                this,
                _bufferOffset);
            _bufferOffset = 0U;
            return;
        }

        uint32_t const chunk = (length > freeBytes) ? freeBytes : static_cast<uint32_t>(length);

        uint32_t const read = static_cast<uint32_t>(
            _socket.read(&_pBuffer[_bufferOffset], static_cast<size_t>(chunk)));

        if (read != chunk)
        {
            ERROR_LOG(
                SOMEIP, "TcpProxy[%p]::received() read failed (%d / %d bytes)", this, read, chunk);
            _bufferOffset = 0U;
            return;
        }

        _bufferOffset += chunk;
        length -= static_cast<uint16_t>(chunk);

        uint32_t validBytes = 0U;
        while (true)
        {
            uint32_t const messageLength = parseMessageLength(validBytes);
            if ((messageLength == 0U) || (messageLength > (_bufferOffset - validBytes)))
            {
                break;
            }

            validBytes += messageLength;
        }

        if (validBytes > 0U)
        {
            etl::copy_n(_pBuffer.begin(), validBytes, _pInputBuffer.begin());
            incRefCounter(); // in case of hanging proxy.
            {
                NetworkChannel channel(*this, endpoint);
                if (_pListener != nullptr)
                {
                    _pListener->received(channel, validBytes);
                }
            }
            decRefCounter();
            _bufferOffset -= validBytes;
            if (_bufferOffset > 0U)
            {
                auto src = _pBuffer.subspan(validBytes, _bufferOffset);
                etl::copy(src.begin(), src.end(), _pBuffer.begin());
            }
        }
    }
}

uint32_t TcpProxy::parseMessageLength(uint32_t const offset) const
{
    uint32_t const length = _bufferOffset - offset;
    if (length < SomeIpMessage::OFFSET_PAYLOAD)
    {
        return 0U;
    }

    SomeIpMessage const message(_pBuffer.subspan(offset, length));
    return SomeIpMessage::OFFSET_PAYLOAD + message.getPayloadLength();
}

void TcpProxy::connected(AbstractSocket::ErrorCode const status)
{
    if (AbstractSocket::ErrorCode::SOCKET_ERR_OK == status)
    {
        _socket.disableNagleAlgorithm();
        openConnection(_socket.getLocalPort());
    }
}

void TcpProxy::connectionClosed(IDataListener::ErrorCode const /* status */) { closeConnection(); }

void TcpProxy::openConnection(port::type const localPort)
{
    ETL_ASSERT(
        _pBuffer.size() > 0U, ETL_ERROR_GENERIC("tcp proxy buffer size must be greater than 0"));
    INFO_LOG(SOMEIP, "TcpProxy[%p]: open connection at port: %d", this, localPort);

    _bufferOffset = 0U;

    if (nullptr != _pConnectionListener)
    {
        _pConnectionListener->connectionChanged(*this);
    }
}

void TcpProxy::closeConnection()
{
    INFO_LOG(SOMEIP, "TcpProxy[%p]: close connection", this);

    _bufferOffset = 0U;

    if (nullptr != _pParentServerListener)
    {
        _pParentServerListener->connectionChanged(*this);
    }

    if (nullptr != _pConnectionListener)
    {
        _pConnectionListener->connectionChanged(*this);
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
