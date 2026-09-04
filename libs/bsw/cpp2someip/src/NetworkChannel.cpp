/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkChannel.h"

#include "someip/NetworkResource.h"
#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

NetworkChannel::NetworkChannel()
: _pResource(nullptr), _remoteEndpoint(), _isMulticast(false), _magicCookieEnabled(false)
{
    _remoteEndpoint.setAddress(NetworkResource::INVALID_IP);
    _remoteEndpoint.setPort(port::INVALID);
}

NetworkChannel::NetworkChannel(
    NetworkResource& resource,
    ::ip::IPEndpoint const& remoteEndpoint,
    bool const isThisMulticast,
    bool const enableMagicCookie)
: _pResource(&resource)
, _remoteEndpoint(remoteEndpoint)
, _isMulticast(::ip::isMulticastAddress(remoteEndpoint.getAddress()) || isThisMulticast)
, _magicCookieEnabled(enableMagicCookie)
{
    if (_pResource != nullptr)
    {
        _pResource->incRefCounter();
    }
    else
    {
        ERROR_LOG(SOMEIP, "NetworkChannel: _pResource invalid!");
    }
}

NetworkChannel::NetworkChannel(NetworkChannel const& channel)
: _pResource(channel._pResource)
, _remoteEndpoint(channel._remoteEndpoint)
, _isMulticast(channel._isMulticast)
, _magicCookieEnabled(channel._magicCookieEnabled)
{
    if (_pResource != nullptr)
    {
        _pResource->incRefCounter();
    }
}

NetworkChannel::~NetworkChannel() { close(); }

NetworkChannel& NetworkChannel::operator=(NetworkChannel const& other)
{
    if (this != &other)
    {
        if (_pResource != other._pResource)
        {
            close();
            _pResource = other._pResource;
            if (_pResource != nullptr)
            {
                _pResource->incRefCounter();
            }
        }
        _remoteEndpoint     = other._remoteEndpoint;
        _isMulticast        = other._isMulticast;
        _magicCookieEnabled = other._magicCookieEnabled;
    }

    return *this;
}

NetworkChannel::NetworkChannel(NetworkChannel&& other) noexcept
: _pResource(other._pResource)
, _remoteEndpoint(other._remoteEndpoint)
, _isMulticast(other._isMulticast)
, _magicCookieEnabled(other._magicCookieEnabled)
{
    other._pResource = nullptr;
}

NetworkChannel& NetworkChannel::operator=(NetworkChannel&& other) noexcept
{
    if (this != &other)
    {
        close();
        _pResource          = other._pResource;
        _remoteEndpoint     = other._remoteEndpoint;
        _isMulticast        = other._isMulticast;
        _magicCookieEnabled = other._magicCookieEnabled;
        other._pResource    = nullptr;
    }
    return *this;
}

::ip::IPEndpoint const& NetworkChannel::getRemoteEndpoint() const { return _remoteEndpoint; }

::etl::expected<uint16_t, PortError> NetworkChannel::getLocalPort() const
{
    if (_pResource == nullptr)
    {
        return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
    }

    return _pResource->getLocalPort();
}

uint8_t NetworkChannel::getProto() const
{
    if (_pResource == nullptr)
    {
        return SomeIpConstants::INVALID_PROTO;
    }

    return _pResource->getProto();
}

::etl::span<uint8_t> NetworkChannel::getInputBuffer() const
{
    if (_pResource == nullptr)
    {
        return {};
    }

    return _pResource->getInputBuffer();
}

::etl::span<uint8_t> NetworkChannel::getOutputBuffer() const
{
    if (_pResource == nullptr)
    {
        return {};
    }

    return _pResource->getOutputBuffer();
}

bool NetworkChannel::isOpen() const
{
    if (_pResource == nullptr)
    {
        return false;
    }

    return _pResource->isOpen();
}

bool NetworkChannel::isConnected() const
{
    if (_pResource == nullptr)
    {
        return false;
    }

    return _pResource->isConnected();
}

bool NetworkChannel::isMulticast() const
{
    if (_pResource == nullptr)
    {
        return false;
    }

    if (!_pResource->isOpen())
    {
        return false;
    }

    return _isMulticast;
}

void NetworkChannel::close()
{
    if (_pResource != nullptr)
    {
        _pResource->decRefCounter();
        _pResource->tryClose();
        _pResource = nullptr;
    }
}

bool NetworkChannel::send(uint32_t const length, ::etl::span<uint8_t> const& buffer)
{
    if (_pResource == nullptr)
    {
        return false;
    }

    auto const output = _pResource->getOutputBuffer();
    _pResource->setOutputBuffer(buffer);

    bool const result = send(length);
    _pResource->setOutputBuffer(output);

    return result;
}

bool NetworkChannel::send(uint32_t const length)
{
    if (_pResource == nullptr)
    {
        return false;
    }

    if (!_pResource->isOpen())
    {
        return false;
    }

    if (_pResource->isConnected())
    {
        return _pResource->send(length);
    }

    return _pResource->send(_remoteEndpoint, length);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
