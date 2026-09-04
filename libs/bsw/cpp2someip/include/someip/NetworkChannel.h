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

#include <ip/IPEndpoint.h>
#include <someip/SomeIpConstants.h>

#include <etl/expected.h>
#include <etl/span.h>
#include <cstdint>

namespace someip
{
class NetworkResource;

/**
 * Wrapper for a protocol specific network resource.
 */
class NetworkChannel
{
public:
    NetworkChannel();

    NetworkChannel(
        NetworkResource& resource,
        ::ip::IPEndpoint const& remoteEndpoint,
        bool isThisMulticast   = false,
        bool enableMagicCookie = false);

    NetworkChannel(NetworkChannel const& channel);

    NetworkChannel& operator=(NetworkChannel const& other);

    ~NetworkChannel();

    NetworkChannel(NetworkChannel&& other) noexcept;
    NetworkChannel& operator=(NetworkChannel&& other) noexcept;

    /**
     * The remote endpoint this channel is associated with.
     */
    ::ip::IPEndpoint const& getRemoteEndpoint() const;

    /**
     * The local port this channel is associated with.
     */
    ::etl::expected<uint16_t, PortError> getLocalPort() const;

    uint8_t getProto() const;

    ::etl::span<uint8_t> getInputBuffer() const;
    ::etl::span<uint8_t> getOutputBuffer() const;

    bool isOpen() const;
    bool isConnected() const;
    bool isMulticast() const;
    void close();

    bool isMagicCookieEnabled() const { return _magicCookieEnabled; }

    bool send(uint32_t length, ::etl::span<uint8_t> const& buffer);
    bool send(uint32_t length);

private:
    NetworkResource* _pResource;

    ::ip::IPEndpoint _remoteEndpoint;
    bool _isMulticast;
    bool _magicCookieEnabled;
};

} // namespace someip
