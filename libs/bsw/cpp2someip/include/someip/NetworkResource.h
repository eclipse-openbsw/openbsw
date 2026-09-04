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

#include <ip/IPAddress.h>
#include <ip/IPEndpoint.h>
#include <someip/SomeIpConstants.h>

#include <etl/expected.h>
#include <etl/span.h>
#include <cstdint>

namespace someip
{
class INetworkListener;

/**
 * Provides access to a protocol-specific network-resource.
 */
class NetworkResource
{
public:
    static ::ip::IPAddress const INVALID_IP;
    static ::ip::IPEndpoint const INVALID_ADDRESS;

    NetworkResource() = default;

    virtual ~NetworkResource() = default;

    virtual bool isInitialized() const;

    void setInputBuffer(::etl::span<uint8_t> const& buffer) { _pInputBuffer = buffer; }

    ::etl::span<uint8_t> getInputBuffer() const;

    void setOutputBuffer(::etl::span<uint8_t> const& buffer) { _pOutputBuffer = buffer; }

    ::etl::span<uint8_t> getOutputBuffer() const;

    void setListener(INetworkListener& listener) { _pListener = &listener; }

    INetworkListener* getListener() const { return _pListener; }

    virtual ::etl::expected<uint16_t, PortError> getLocalPort() const = 0;
    virtual uint8_t getProto() const                                  = 0;

    virtual bool isOpen() const      = 0;
    virtual bool isConnected() const = 0;
    virtual void close()             = 0;

    inline void tryClose()
    {
        if (_refCounter == 0)
        {
            close();
        }
    }

    virtual bool send(::ip::IPEndpoint const& remoteEndpoint, uint32_t length);
    virtual bool send(uint32_t length);

    void incRefCounter() { ++_refCounter; }

    void decRefCounter() { --_refCounter; }

protected:
    ::etl::span<uint8_t> _pInputBuffer;
    ::etl::span<uint8_t> _pOutputBuffer;

    INetworkListener* _pListener{nullptr};

private:
    size_t _refCounter{0U};
};

} // namespace someip
