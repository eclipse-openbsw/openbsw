/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpClientChannelValidator.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"

namespace someip
{
namespace
{
void sendClientToServerMagicCookie(NetworkChannel& channel)
{
    ::etl::span<uint8_t> const buffer = channel.getOutputBuffer();
    SomeIpMessage cookie(buffer);
    SomeIpMessage::makeClientToServerMagicCookieMessage(cookie);
    // intentionally drop the send result, here socket might be in any state except 'closed'. if
    // the connection is broken then the next time it will be in 'closed' state. if the connection
    // is in SYN_SENT the next time it can be 'established' which means an ACK is received otherwise
    // broken and then closed.
    (void)channel.send(static_cast<uint32_t>(SomeIpConstants::HEADER_LENGTH));

    return;
}
} // namespace

TcpClientChannelValidator::CachedValidator::CachedValidator(TcpClientChannelValidator& validator)
: _validator(validator), _remote(), _localPort(0U), _cachedResult(CachedResult::NO_RESULT)
{}

bool TcpClientChannelValidator::CachedValidator::isChannelEstablished(
    ::ip::IPEndpoint const& remote, uint16_t const localPort)
{
    if (hasResult(remote, localPort) == false)
    {
        _cachedResult = _validator.isChannelEstablished(remote, localPort) == true
                            ? CachedResult::POSITIVE_RESULT
                            : CachedResult::NEGATIVE_RESULT;
    }
    return _cachedResult == CachedResult::POSITIVE_RESULT;
}

void TcpClientChannelValidator::CachedValidator::checkClientChannel(
    ::ip::IPEndpoint const& remote, uint16_t const localPort)
{
    if (_validator._magicCookieEnabled == false)
    {
        return;
    }
    if (hasResult(remote, localPort) == false)
    {
        _validator.checkClientChannel(remote, localPort);
        _cachedResult = CachedResult::RESULT_PENDING;
    }
}

bool TcpClientChannelValidator::CachedValidator::hasResult(
    ::ip::IPEndpoint const& remote, uint16_t const localPort)
{
    bool const res
        = ((_cachedResult != CachedResult::NO_RESULT) && (remote == _remote)
           && (localPort == _localPort));
    if (res == false)
    {
        _remote    = remote;
        _localPort = localPort;
    }
    return res;
}

TcpClientChannelValidator::TcpClientChannelValidator(
    INetwork& network, bool const magicCookieEnabled)
: _network(network), _magicCookieEnabled(magicCookieEnabled)
{}

bool TcpClientChannelValidator::isChannelEstablished(
    ::ip::IPEndpoint const& remote, uint16_t const localPort) const
{
    ::etl::optional<NetworkChannel> channel
        = _network.getRpcChannel(localPort, remote, proto::SD_L4_PROTO_TCP);

    return (channel.has_value() && channel.value().isConnected());
}

void TcpClientChannelValidator::checkClientChannel(
    ::ip::IPEndpoint const& remote, uint16_t const localPort) const
{
    ::etl::optional<NetworkChannel> channel
        = _network.getRpcChannel(localPort, remote, proto::SD_L4_PROTO_TCP);
    if ((channel.has_value() == false) || (channel.value().isMagicCookieEnabled() == false))
    {
        return;
    }
    sendClientToServerMagicCookie(channel.value());
}

} // namespace someip
