/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <app/appConfig.h>
#include <async/Types.h>
#include <doip/server/DoIpServerSocketHandler.h>
#include <doip/server/DoIpServerTransportConnectionPool.h>
#include <doip/server/DoIpServerTransportLayer.h>
#include <doip/server/DoIpServerTransportLayerParameters.h>
#include <doip/server/DoIpServerVehicleIdentification.h>
#include <doip/server/DoIpServerVehicleIdentificationParameters.h>
#include <doip/server/DoIpServerVehicleIdentificationService.h>
#include <doip/server/IDoIpServerConnectionStateCallback.h>
#include <doip/server/IDoIpServerEntityStatusCallback.h>
#include <doip/server/IDoIpServerTransportLayerCallback.h>
#include <etl/delegate.h>
#include <etl/optional.h>
#include <etl/span.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lwipSocket/tcp/LwipServerSocket.h>
#include <lwipSocket/tcp/LwipSocket.h>
#include <lwipSocket/udp/LwipDatagramSocket.h>
#include <transport/ITransportSystem.h>
#include <platform/estdint.h>

namespace doip
{
class DoIpServerSystem
: public ::lifecycle::AsyncLifecycleComponent
, private IDoIpServerTransportLayerCallback
, private IDoIpServerEntityStatusCallback
, private ::async::RunnableType
{
public:
    using FirstRoutingActivatedCallbackType = ::etl::delegate<void()>;
    using VinCallbackType = ::etl::delegate<void(::etl::span<uint8_t, VIN_LENGTH>)>;

    DoIpServerSystem(
        ::transport::ITransportSystem& transportSystem,
        ::ip::NetworkInterfaceConfigRegistry& netIfCfgReg,
        ::async::ContextType asyncContext,
        uint8_t busId,
        uint16_t doipLogicalAddress,
        ::etl::span<uint8_t const, MAC_LENGTH> macAddress,
        ::ip::IPAddress const& vehicleAnnouncementAddress);

    void setFirstRoutingActivatedCallback(
        FirstRoutingActivatedCallbackType firstRoutingActivatedCallback,
        uint16_t firstRoutingSourceAddress);

    void setVinCallback(VinCallbackType vinCallback);

    void init() override;
    void run() override;
    void shutdown() override;

    void
    setVehicleAnnouncementListener(IDoIpVehicleAnnouncementListener& vehicleAnnouncementListener);

    void setConnectionStateCallback(IDoIpServerConnectionStateCallback& connectionStateCallback)
    {
        _connectionStateCallback = &connectionStateCallback;
    }

    ::transport::AbstractTransportLayer* transportLayer() { return &_transportLayer; }

private:
    uint8_t
    getSocketGroupId(uint8_t serverSocketId, ::ip::IPEndpoint const& remoteEndpoint) const override;
    uint8_t getMaxConnectionCount(uint8_t socketGroupId) const override;
    bool filterConnection(uint8_t socketGroupId, ::ip::IPEndpoint const& remoteEndpoint) override;
    RoutingActivationCheckResult checkRoutingActivation(
        uint16_t sourceAddress,
        uint8_t activationType,
        uint8_t socketGroupId,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint,
        ::etl::optional<uint32_t> const oemField,
        bool isResuming) override;
    void routingActive(
        uint16_t sourceAddress,
        uint16_t internalSourceAddress,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint,
        DoIpTcpConnection::ConnectionType type) override;
    void routingInactive(uint16_t internalSourceAddress) override;

    void connectionClosed(uint16_t sourceAddress) override;

    IDoIpServerEntityStatusCallback::EntityStatus
    getEntityStatus(uint8_t socketGroupId) const override;

    void execute() override;

    void getVin(::etl::span<char, VIN_LENGTH>);
    void getGid(::etl::span<uint8_t, GID_LENGTH>);
    void getEid(::etl::span<uint8_t, EID_LENGTH>);
    DoIpConstants::DiagnosticPowerMode getPowerMode();
    void OnVirReceivedCallback();

    void shutdownDone(::transport::AbstractTransportLayer& transportLayer);

    class TransportConnectionCreator
    : public IDoIpServerTransportConnectionCreator<DoIpServerTransportConnection>
    {
    public:
        DoIpServerTransportConnection& createConnection(
            DoIpServerTransportConnection* memory,
            uint8_t socketGroupId,
            ::tcp::AbstractSocket& socket,
            ::etl::ipool& diagnosticSendJobBlockPool,
            ::etl::ipool& protocolSendJobBlockPool,
            DoIpServerTransportConnectionConfig const& config,
            DoIpTcpConnection::ConnectionType type) override;
    };

    static constexpr uint8_t const maxConnectionCount = 5;

    using DoIpServerTransportConnectionPoolType = declare::DoIpServerTransportConnectionPool<
        DoIpServerTransportConnection,
        maxConnectionCount,
        NUM_DIAGNOSTICSENDJOBS,
        NUM_PROTOCOLSENDJOBS>;
    using DoIpServerTransportLayerType = declare::DoIpServerTransportLayer<1>;

    using DoIpServerSocketHandlerType = ::doip::declare::DoIpServerSocketHandler<
        ::tcp::LwipServerSocket,
        ::tcp::LwipSocket,
        2,
        maxConnectionCount + 1>;

    using DoIpServerVehicleIdentificationService = declare::DoIpServerVehicleIdentificationService<
        ::udp::LwipDatagramSocket,
        NUM_UNICAST,
        NUM_SOCKETS,
        NUM_REQUESTS,
        NUM_ANNOUNCEMENTS>;

    ::async::ContextType _asyncContext;
    DoIpServerSocketHandlerType _socketHandler;
    TransportConnectionCreator _transportConnectionCreator;
    DoIpServerTransportConnectionPoolType _transportConnectionPool;
    DoIpServerTransportLayerParameters _transportLayerParameters;
    DoIpServerTransportLayerType _transportLayer;
    DoIpServerVehicleIdentificationParameters _vehicleIdentificationParameters;
    DoIpServerVehicleIdentification _vehicleIdentification;
    DoIpServerVehicleIdentificationService _vehicleIdentificationService;
    ::transport::ITransportSystem& _transportSystem;
    FirstRoutingActivatedCallbackType _firstRoutingActivatedCallback;
    IDoIpServerConnectionStateCallback* _connectionStateCallback;
    VinCallbackType _vinCallback;
    ::async::TimeoutType _timeout;
    uint16_t _firstRoutingSourceAddress;
    uint8_t _busId;
    ::etl::span<uint8_t const, MAC_LENGTH> _macAddress;
};

} // namespace doip
