/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcChannel.h"

#include "someip/INetwork.h"
#include "someip/IRpcSender.h"
#include "someip/NetworkResource.h"
#include "someip/RpcClosure.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <udp/DatagramPacket.h>
#include <util/timeout/ITimeoutManager2.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::udp::DatagramPacket;
using ::util::logger::SOMEIP;

using ::common::ITimeoutManager2;

RpcChannel::RpcChannel(INetwork& network, IRpcSender& sender)
: _network(network)
, _sender(sender)
, _timeoutExpiredFunction(
      ::async::Function::CallType::create<RpcChannel, &RpcChannel::timeoutExpired>(*this))
, _timeoutExpiredTimeout()
{}

bool RpcChannel::isOpen() const
{
    if (_networkChannel.has_value())
    {
        return _networkChannel.value().isOpen();
    }

    return false;
}

void RpcChannel::openUdp(
    service_id::type const serviceId,
    ::ip::IPEndpoint const& remoteEndpoint,
    port::type const localPort)
{
    close();

    _serviceId      = serviceId;
    _networkChannel = _network.openUdpChannel(localPort, remoteEndpoint);
}

void RpcChannel::openTcp(
    service_id::type const serviceId,
    ::ip::IPEndpoint const& remoteEndpoint,
    port::type const localPort)
{
    close();

    _serviceId      = serviceId;
    _networkChannel = _network.openTcpChannel(localPort, remoteEndpoint);
}

void RpcChannel::openTcpWithExternalReassembleBuffer(
    service_id::type const serviceId,
    ::ip::IPEndpoint const& remoteEndpoint,
    port::type const localPort,
    ::etl::span<uint8_t> const buffer)
{
    close();

    _serviceId = serviceId;
    _networkChannel
        = _network.openTcpChannelWithExternalReassembleBuffer(localPort, remoteEndpoint, buffer);
}

void RpcChannel::close()
{
    _networkChannel.reset();
    _serviceId = 0;
}

service_id::type RpcChannel::getServiceId() const { return _serviceId; }

uint16_t RpcChannel::getClientId() const { return _clientId; }

uint16_t RpcChannel::getSessionId() const { return _sessionId; }

void RpcChannel::setSessionId(uint16_t const sessionId) { _sessionId = sessionId; }

void RpcChannel::setClientId(uint16_t const clientId) { _clientId = clientId; }

::ip::IPEndpoint const& RpcChannel::getRemoteIp() const
{
    if (!_networkChannel.has_value())
    {
        return NetworkResource::INVALID_ADDRESS;
    }

    return _networkChannel->getRemoteEndpoint();
}

::etl::expected<uint16_t, PortError> RpcChannel::getLocalPort() const
{
    if (!_networkChannel.has_value())
    {
        return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
    }

    return _networkChannel->getLocalPort();
}

uint8_t RpcChannel::getProto() const
{
    if (!_networkChannel.has_value())
    {
        return SomeIpConstants::INVALID_PROTO;
    }

    return _networkChannel->getProto();
}

ServiceResultCode RpcChannel::callMethod(
    uint16_t const methodId,
    ISomeIpSerializable const* const pRequest,
    uint8_t const interfaceVersion,
    ISomeIpSerializable* const pResponse,
    CallDoneClosure& done,
    uint32_t timeout)
{
    if (!_networkChannel.has_value())
    {
        return COULD_NOT_DELIVER;
    }

    if (_pPendingCallback != nullptr)
    {
        return BUSY_ERROR;
    }

    INFO_LOG(SOMEIP, "RpcChannel::callMethod(%d)", methodId);

    _pPendingResponse = pResponse;
    _pPendingCallback = &done;

    ServiceResultCode const rc = _sender.sendRequest(
        pRequest, _serviceId, methodId, interfaceVersion, true, *this, timeout);

    if (rc != RPC_SENT_SUCCESSFULLY)
    {
        _pPendingCallback = nullptr;
    }

    return rc;
}

ServiceResultCode RpcChannel::callFireAndForgetMethod(
    uint16_t const methodId,
    ISomeIpSerializable const* const pRequest,
    uint8_t const interfaceVersion)
{
    if (!_networkChannel.has_value())
    {
        return COULD_NOT_DELIVER;
    }

    INFO_LOG(SOMEIP, "RpcChannel::callFireAndForgetMethod(%d)", methodId);

    ServiceResultCode const rc
        = _sender.sendRequest(pRequest, _serviceId, methodId, interfaceVersion, false, *this, 0);

    return (rc != RPC_SENT_SUCCESSFULLY) ? rc : RPC_SENT_SUCCESSFULLY_NO_RESPONSE_EXPECTED;
}

ISomeIpSerializable* RpcChannel::getResponse()
{
    return (_pPendingCallback != nullptr) ? _pPendingResponse : nullptr;
}

void RpcChannel::responseReceived(ServiceResultCode result)
{
    INFO_LOG(SOMEIP, "RpcChannel::responseReceived(%d)", result);

    if (_pPendingCallback == nullptr)
    {
        ERROR_LOG(SOMEIP, "RpcChannel::responseReceived() no callback");
        return;
    }

    CallDoneClosure& done = *_pPendingCallback;
    _pPendingCallback     = nullptr;

    done(result);
}

void RpcChannel::cancelTimeout() { _timeoutExpiredTimeout.cancel(); }

void RpcChannel::setTimeout(::async::ContextType const context, uint32_t timeout)
{
    async::schedule(
        context,
        _timeoutExpiredFunction,
        _timeoutExpiredTimeout,
        timeout,
        ::async::TimeUnit::MILLISECONDS);
}

// private
void RpcChannel::timeoutExpired()
{
    WARN_LOG(SOMEIP, "RpcChannel::timeoutExpired()");

    if (_pPendingCallback == nullptr)
    {
        ERROR_LOG(SOMEIP, "RpcChannel::timeoutExpired() no callback");
        return;
    }

    _sender.requestExpired(*this);

    CallDoneClosure& done = *_pPendingCallback;
    _pPendingCallback     = nullptr;

    done(RPC_TIMEOUT);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
