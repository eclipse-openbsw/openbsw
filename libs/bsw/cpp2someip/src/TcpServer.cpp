/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpServer.h"

#include "someip/TcpConfig.h"
#include "someip/logger.h"

#include <etl/error_handler.h>
#include <ip/to_str.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPAddress;
using ::ip::IPEndpoint;
using ::tcp::AbstractServerSocket;
using ::tcp::AbstractSocket;
using ::util::logger::SOMEIP;

TcpServer::TcpServer(::tcp::AbstractServerSocket& socket, TcpProxyConfig& tcpClientPool)
: _socket(socket), _tcpClientPool(tcpClientPool), _pClientBufferPool(nullptr)
{
    _socket.setSocketProvidingConnectionListener(*this);
}

bool TcpServer::isInitialized() const { return _tcpClientPool.getSize() > 0U; }

AbstractServerSocket& TcpServer::getServerSocket() const { return _socket; }

size_t TcpServer::getNumProxies() const { return _tcpClientPool.getSize(); }

void TcpServer::setClientBufferPool(ClientBufferPool& buffers)
{
    ETL_ASSERT(
        _tcpClientPool.getBufferType() == TcpProxyConfig::BufferType::External,
        ETL_ERROR_GENERIC("tcp client pool must use external buffer type"));
    _pClientBufferPool = &buffers;
}

TcpProxy* TcpServer::getProxy(size_t const pos) const { return _tcpClientPool.getProxy(pos); }

bool TcpServer::isOpen() const { return isInitialized() && (!_socket.isClosed()); }

bool TcpServer::open(IPEndpoint const& localEndpoint)
{
    if ((_tcpClientPool.getBufferType() == TcpProxyConfig::BufferType::External)
        && (_pClientBufferPool == nullptr))
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::open() no client reassembly buffer", this);
        return false;
    }

    if (!isInitialized())
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::open() not initialized", this);
        return false;
    }

    if (isOpen())
    {
        return true;
    }

    if (!localEndpoint.isSet())
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::open() no local endpoint", this);
        return false;
    }

    char localEndpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
    INFO_LOG(
        SOMEIP,
        "TcpServer[%p]::open() at: %s",
        this,
        ::ip::to_str(localEndpoint, localEndpointStr).data());

    if (!_socket.bind(localEndpoint.getAddress(), localEndpoint.getPort()))
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::open() bind failed", this);
        return false;
    }

    if (!_socket.accept())
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::open() accept failed", this);
        return false;
    }

    return true;
}

void TcpServer::close()
{
    if (!isOpen())
    {
        return;
    }

    INFO_LOG(SOMEIP, "TcpServer[%p]: close", this);

    _socket.close();
}

::etl::expected<uint16_t, PortError> TcpServer::getLocalPort() const
{
    if (isInitialized())
    {
        return _socket.getLocalPort();
    }

    return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
}

// virtual
AbstractSocket* TcpServer::getSocket(IPAddress const& ipAddr, uint16_t const port)
{
    if (!isInitialized())
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::getSocket() not initialized", this);
        return nullptr;
    }

    if (::ip::isUnspecified(ipAddr))
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::getSocket() no address", this);
        return nullptr;
    }

    // First search if this is the same client connecting again.
    // This would be a sign that we didn't catch the connection close correctly. In this case we
    // have to abort the existing connection and reuse it.
    // This also means that only one TCP client connection is allowed between
    // a remote and the local ECU, which holds true by current requirements.

    ::ip::IPEndpoint const endpoint(ipAddr, port);
    auto const localPortResult = getLocalPort();
    TcpProxy* proxy            = nullptr;

    if (localPortResult.has_value())
    {
        proxy = _tcpClientPool.getOpenProxy(localPortResult.value(), endpoint);
    }

    if (proxy != nullptr)
    {
        proxy->abort();
        WARN_LOG(SOMEIP, "TcpServer[%p]: reusing proxy[%p]", this, proxy);
    }
    else
    {
        proxy = _tcpClientPool.nextProxy();
    }

    if (proxy != nullptr)
    {
        if (_tcpClientPool.getBufferType() == TcpProxyConfig::BufferType::External)
        {
            ETL_ASSERT(
                _pClientBufferPool != nullptr,
                ETL_ERROR_GENERIC("client buffer pool must not be null"));
            if (_pClientBufferPool->empty())
            {
                ERROR_LOG(SOMEIP, "TcpServer[%p]::getSocket() no free client socket buffer", this);
                return nullptr;
            }

            proxy->setInternalBuffer(_pClientBufferPool->back());
            _pClientBufferPool->pop_back();
        }

        IPEndpoint const remote(ipAddr, port);
        char endpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];

        INFO_LOG(
            SOMEIP,
            "TcpServer[%p]: acquire proxy[%p] for: %s",
            this,
            proxy,
            ::ip::to_str(remote, endpointStr).data());
        proxy->_pParentServerListener = this;
        return &(proxy->getSocket());
    }

    ERROR_LOG(SOMEIP, "TcpServer[%p]::getSocket() no free socket", this);
    return nullptr;
}

// virtual
void TcpServer::connectionAccepted(::tcp::AbstractSocket& socket)
{
    socket.disableNagleAlgorithm();

    IPEndpoint const remote(socket.getRemoteIPAddress(), socket.getRemotePort());
    char endpointStr[::ip::MAX_ENDPOINT_STRING_LENGTH];

    uint16_t const localPort = socket.getLocalPort();

    INFO_LOG(
        SOMEIP,
        "TcpServer[%p]: accepted connection at port: %d from : %s",
        this,
        localPort,
        ::ip::to_str(remote, endpointStr).data());

    TcpProxy* const proxy = _tcpClientPool.getProxy(socket);
    if (proxy == nullptr)
    {
        ERROR_LOG(SOMEIP, "TcpServer[%p]::connectionAccepted() invalid socket", this);
    }
    else
    {
        proxy->incRefCounter();
        proxy->openConnection(localPort);
    }
}

void TcpServer::connectionChanged(TcpProxy& proxy)
{
    if (!proxy.isOpen())
    {
        proxy.decRefCounter();
        if (_tcpClientPool.getBufferType() == TcpProxyConfig::BufferType::External)
        {
            ETL_ASSERT(
                _pClientBufferPool != nullptr,
                ETL_ERROR_GENERIC("client buffer pool must not be null"));
            _pClientBufferPool->push_back(proxy.getInternalBuffer());
        }
        proxy._pParentServerListener = nullptr;
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
