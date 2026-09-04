/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/NetworkResource.h"
#include "someip/SomeIpConstants.h"

#include <ip/IPEndpoint.h>
#include <tcp/IDataListener.h>
#include <tcp/socket/AbstractSocket.h>

#include <etl/span.h>
#include <etl/tuple.h>

#include <cstdint>

// These classes and functions are in the global scope for backwards compatiblity
etl::tuple<uint16_t, ::someip::PortRangeReturnCode> computeNextLocalPort(uint16_t, uint16_t);

namespace someip
{

class TcpServer;

/**
 * A TCP proxy for a protocol specific network-resource.
 */
class TcpProxy
: public NetworkResource
, public ::tcp::IDataListener
{
public:
    class IConnectionListener
    {
    public:
        virtual ~IConnectionListener() = default;

        virtual void connectionChanged(TcpProxy& proxy) = 0;
    };

    explicit TcpProxy(::tcp::AbstractSocket& socket);

    bool isInitialized() const override;

    ::tcp::AbstractSocket& getSocket() const;

    void setInternalBuffer(::etl::span<uint8_t> const buffer) { _pBuffer = buffer; }

    ::etl::span<uint8_t> getInternalBuffer() const { return _pBuffer; }

    void setConnectionListener(IConnectionListener* listener);

    bool isOpen() const override;
    bool isConnected() const override;

    bool isIdle() const;

    ::etl::expected<uint16_t, PortError> getLocalPort() const override;
    uint8_t getProto() const override;

    ::ip::IPEndpoint getRemoteEndpoint() const;

    bool open(::ip::IPEndpoint const& localEndpoint, ::ip::IPEndpoint const& remoteEndpoint);

    void close() override;

    bool send(::ip::IPEndpoint const& remoteEndpoint, uint32_t length) override;
    bool send(uint32_t length) override;

    /** \see IDataListener */
    void dataReceived(uint16_t length) override;

    /** \see IDataListener */
    void connectionClosed(::tcp::IDataListener::ErrorCode status) override;

    void abort();

private:
    friend class TcpServer;

    void connected(::tcp::AbstractSocket::ErrorCode status);

    void openConnection(uint16_t localPort);
    void closeConnection();

    /**
     * Return length of message starting at given offset in the internal buffer,
     * or 0 if not enough header data available.
     */
    uint32_t parseMessageLength(uint32_t offset) const;

    ::tcp::AbstractSocket& _socket;
    IConnectionListener* _pConnectionListener;
    IConnectionListener* _pParentServerListener;

    ::etl::span<uint8_t> _pBuffer;
    uint32_t _bufferOffset;
    uint16_t _dynamicLocalPort;
};

} // namespace someip
