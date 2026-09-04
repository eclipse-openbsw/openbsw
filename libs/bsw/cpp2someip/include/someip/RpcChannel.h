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

#include "someip/INetwork.h"
#include "someip/IRpcChannel.h"
#include "someip/IRpcSender.h"
#include "someip/NetworkChannel.h"

#include <async/Types.h>
#include <async/util/Call.h>

#include <etl/optional.h>
#include <cstdint>

namespace someip
{
class RpcChannel : public IRpcChannel
{
public:
    RpcChannel(INetwork& network, IRpcSender& sender);

    bool isOpen() const;

    void
    openUdp(service_id::type serviceId, ::ip::IPEndpoint const& remoteEndpoint, uint16_t localPort);

    void
    openTcp(service_id::type serviceId, ::ip::IPEndpoint const& remoteEndpoint, uint16_t localPort);
    void openTcpWithExternalReassembleBuffer(
        service_id::type serviceId,
        ::ip::IPEndpoint const& remoteEndpoint,
        port::type localPort,
        ::etl::span<uint8_t> buffer);

    void close();

    /**
     * \see rpc::IRpcChannel::getServiceId()
     */
    service_id::type getServiceId() const override;

    /**
     * \see rpc::IRpcChannel::getClientId()
     */
    uint16_t getClientId() const override;

    void setClientId(uint16_t clientId);

    uint16_t getSessionId() const override;

    void setSessionId(uint16_t sessionId) override;
    /**
     * \see rpc::IRpcChannel::getRemoteIp()
     */
    ::ip::IPEndpoint const& getRemoteIp() const override;

    /**
     * \see rpc::IRpcChannel::getLocalPort()
     */
    ::etl::expected<uint16_t, PortError> getLocalPort() const override;

    /**
     * \see rpc::IRpcChannel::getProto()
     */
    uint8_t getProto() const override;

    /**
     * \see rpc::IRpcChannel::cancelTimeout()
     */
    virtual void cancelTimeout() override;

    /**
     * \see rpc::IRpcChannel::setTimeout()
     */
    virtual void setTimeout(::async::ContextType const context, uint32_t timeout) override;

    ServiceResultCode callMethod(
        uint16_t methodId,
        ISomeIpSerializable const* pRequest,
        uint8_t interfaceVersion,
        ISomeIpSerializable* pResponse,
        CallDoneClosure& done,
        uint32_t timeout) override;

    ServiceResultCode callFireAndForgetMethod(
        uint16_t methodId, ISomeIpSerializable const* pRequest, uint8_t interfaceVersion) override;

    /**
     * \see rpc::IRpcChannel::getResponse()
     */
    ISomeIpSerializable* getResponse() override;

    /**
     * \see rpc::IRpcChannel::responseReceived()
     */
    void responseReceived(ServiceResultCode result) override;

private:
    void timeoutExpired();

    INetwork& _network;
    IRpcSender& _sender;

    ::async::Function _timeoutExpiredFunction;
    ::async::TimeoutType _timeoutExpiredTimeout;

    service_id::type _serviceId = 0;
    uint16_t _clientId          = 0;
    uint16_t _sessionId         = 0;

    ::etl::optional<NetworkChannel> _networkChannel;

    ISomeIpSerializable* _pPendingResponse = nullptr;
    CallDoneClosure* _pPendingCallback     = nullptr;
};

} // namespace someip
