/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/DoIpServerSystem.h"

#include <busid/BusId.h>
#include <doip/common/DoIpConstants.h>
#include <doip/server/DoIpServerLogger.h>
#include <etl/functional.h>
#include <etl/memory.h>
#include <ip/NetworkInterfaceConfig.h>
#include <transport/TransportConfiguration.h>

namespace doip
{
using ::util::logger::DOIP;
using ::util::logger::Logger;

DoIpServerSystem::DoIpServerSystem(
    ::transport::ITransportSystem& transportSystem,
    ::ip::NetworkInterfaceConfigRegistry& netIfCfgReg,
    ::async::ContextType asyncContext,
    uint8_t busId,
    uint16_t doipLogicalAddress,
    ::etl::span<uint8_t const, MAC_LENGTH> macAddress,
    ::ip::IPAddress const& vehicleAnnouncementBroadcastAddress)
: _asyncContext(asyncContext)
, _socketHandler(netIfCfgReg)
, _transportConnectionCreator()
, _transportConnectionPool(_transportConnectionCreator)
, _transportLayerParameters(
      INITIAL_INACTIVITY_TIMEOUT,
      GENERAL_INACTIVITY_TIMEOUT,
      ALIVE_CHECK_TIMEOUT,
      MAX_PAYLOAD_LENGTH)
, _transportLayer(
      busId,
      doipLogicalAddress,
      _asyncContext,
      _socketHandler,
      _transportConnectionPool,
      _transportLayerParameters)
, _vehicleIdentificationParameters(ANNOUNCE_WAIT_TOUT, ANNOUNCE_INTERVAL, NUM_ANNOUNCEMENTS)
, _vehicleIdentification(
      DoIpServerVehicleIdentification::GetVinCallback::
          create<DoIpServerSystem, &DoIpServerSystem::getVin>(*this),
      DoIpServerVehicleIdentification::GetGidCallback::
          create<DoIpServerSystem, &DoIpServerSystem::getGid>(*this),
      DoIpServerVehicleIdentification::GetEidCallback::
          create<DoIpServerSystem, &DoIpServerSystem::getEid>(*this),
      DoIpServerVehicleIdentification::GetPowerModeCallback::
          create<DoIpServerSystem, &DoIpServerSystem::getPowerMode>(*this),
      DoIpServerVehicleIdentification::OnVirReceivedCallback::
          create<DoIpServerSystem, &DoIpServerSystem::OnVirReceivedCallback>(*this),
      nullptr) // No OEM Message Handlers set yet)
, _vehicleIdentificationService(
      DoIpConstants::ProtocolVersion::version02Iso2012,
      _asyncContext,
      _vehicleIdentification,
      *this,
      netIfCfgReg,
      doipLogicalAddress,
      _vehicleIdentificationParameters)
, _transportSystem(transportSystem)
, _busId(busId)
, _macAddress(macAddress)
{
    _socketHandler.addServerSocket(SOCKET_GROUP, _busId);

    _vehicleIdentificationService.addSocket(
        SOCKET_GROUP, _busId, vehicleAnnouncementBroadcastAddress);

    // Tell the lifecycle manager in which context to execute init/run/shutdown
    setTransitionContext(_asyncContext);
}

void DoIpServerSystem::setFirstRoutingActivatedCallback(
    FirstRoutingActivatedCallbackType const firstRoutingActivatedCallback,
    uint16_t const firstRoutingSourceAddress)
{
    _firstRoutingActivatedCallback = firstRoutingActivatedCallback;
    _firstRoutingSourceAddress     = firstRoutingSourceAddress;
}

void DoIpServerSystem::setVinCallback(VinCallbackType vinCallback) { _vinCallback = vinCallback; }

void DoIpServerSystem::init()
{
    // Inform the lifecycle manager that the transition has been completed
    transitionDone();
}

void DoIpServerSystem::run()
{
    _transportLayer.setCallback(*this);
    _transportSystem.addTransportLayer(_transportLayer);
    _transportLayer.init();
    _vehicleIdentificationService.start();
    Logger::info(DOIP, "DoIp Initialized");

    // Inform the lifecycle manager that the transition has been completed
    transitionDone();
}

void DoIpServerSystem::shutdown()
{
    _transportSystem.removeTransportLayer(_transportLayer);
    _transportLayer.shutdown(::transport::AbstractTransportLayer::ShutdownDelegate::
                                 create<DoIpServerSystem, &DoIpServerSystem::shutdownDone>(*this));
    _transportLayer.closeAllConnections(IDoIpTcpConnection::CloseMode::ABORT);
    _vehicleIdentificationService.shutdown();
    // TODO: remove timeout when DoIP uses RST to terminate the connection
    ::async::schedule(_asyncContext, *this, _timeout, 500U, ::async::TimeUnit::MILLISECONDS);

    // Inform the lifecycle manager that the transition has been completed
    transitionDone();
}

void DoIpServerSystem::setVehicleAnnouncementListener(
    IDoIpVehicleAnnouncementListener& vehicleAnnouncementListener)
{
    _vehicleIdentificationService.setVehicleAnnouncementListener(&vehicleAnnouncementListener);
}

uint8_t DoIpServerSystem::getSocketGroupId(
    uint8_t serverSocketId, ::ip::IPEndpoint const& /*remoteEndpoint*/) const
{
    return serverSocketId;
}

uint8_t DoIpServerSystem::getMaxConnectionCount(uint8_t /*socketGroupId*/) const
{
    return maxConnectionCount;
}

bool DoIpServerSystem::filterConnection(
    uint8_t /*socketGroupId*/, ::ip::IPEndpoint const& /*remoteEndpoint*/)
{
    return true;
}

IDoIpServerConnectionFilter::RoutingActivationCheckResult DoIpServerSystem::checkRoutingActivation(
    uint16_t sourceAddress,
    uint8_t activationType,
    uint8_t /*socketGroupId*/,
    ::ip::IPEndpoint const& /*localEndpoint*/,
    ::ip::IPEndpoint const& /*remoteEndpoint*/,
    ::etl::optional<uint32_t> const /*oemField*/,
    bool /*isResuming*/)
{
    if (activationType > 0x01)
    {
        return RoutingActivationCheckResult()
            .setAction(IDoIpServerConnectionFilter::Action::REJECT)
            .setResponseCode(
                DoIpConstants::RoutingResponseCodes::ROUTING_UNSUPPORTED_ACTIVATION_TYPE);
    }
    if (transport::TransportConfiguration::isTesterAddress(sourceAddress))
    {
        return RoutingActivationCheckResult();
    }
    return RoutingActivationCheckResult()
        .setAction(IDoIpServerConnectionFilter::Action::REJECT)
        .setResponseCode(DoIpConstants::RoutingResponseCodes::ROUTING_UNKNOWN_SOURCE_ADDRESS);
}

void DoIpServerSystem::routingActive(
    uint16_t sourceAddress,
    uint16_t internalSourceAddress,
    ::ip::IPEndpoint const& /*localEndpoint*/,
    ::ip::IPEndpoint const& /*remoteEndpoint*/,
    DoIpTcpConnection::ConnectionType type)
{
    Logger::info(DOIP, "routing active %x", sourceAddress);
    if (_firstRoutingActivatedCallback && (internalSourceAddress == _firstRoutingSourceAddress))
    {
        FirstRoutingActivatedCallbackType firstRoutingActivatedCallback
            = _firstRoutingActivatedCallback;
        _firstRoutingActivatedCallback = FirstRoutingActivatedCallbackType();
        if (firstRoutingActivatedCallback.is_valid())
        {
            firstRoutingActivatedCallback();
        }
    }

    if (_connectionStateCallback != nullptr)
    {
        _connectionStateCallback->connectionRoutingActive(sourceAddress, type);
    }
}

void DoIpServerSystem::routingInactive(uint16_t /*internalSourceAddress*/) {}

void DoIpServerSystem::connectionClosed(uint16_t sourceAddress)
{
    if (_connectionStateCallback != nullptr)
    {
        _connectionStateCallback->connectionClosed(sourceAddress);
    }
}

IDoIpServerEntityStatusCallback::EntityStatus
DoIpServerSystem::getEntityStatus(uint8_t socketGroupId) const
{
    return IDoIpServerEntityStatusCallback::EntityStatus(
        ::doip::DoIpConstants::EntityStatusNodeType::NODE_TYPE_NODE,
        getMaxConnectionCount(socketGroupId),
        _transportLayer.getConnectionCount(socketGroupId),
        MAX_DATA_SIZE_LOGICAL_REQUESTS);
}

void DoIpServerSystem::execute() {}

void DoIpServerSystem::getVin(::etl::span<char, VIN_LENGTH> vin)
{
    if (_vinCallback.is_valid())
    {
        _vinCallback(vin.reinterpret_as<uint8_t>());
    }
}

void DoIpServerSystem::getGid(::etl::span<uint8_t, GID_LENGTH> lGid)
{
    static_assert(
        decltype(_macAddress)::extent <= decltype(lGid)::extent,
        "GID span not big enough to hold MAC");
    ::etl::mem_copy(_macAddress.begin(), _macAddress.end(), lGid.begin());
}

void DoIpServerSystem::getEid(::etl::span<uint8_t, EID_LENGTH> lEid)
{
    static_assert(
        decltype(_macAddress)::extent <= decltype(lEid)::extent,
        "EID span not big enough to hold MAC");
    ::etl::mem_copy(_macAddress.begin(), _macAddress.end(), lEid.begin());
}

DoIpConstants::DiagnosticPowerMode DoIpServerSystem::getPowerMode()
{
    return DoIpConstants::DiagnosticPowerMode::READY;
}

void DoIpServerSystem::OnVirReceivedCallback() { Logger::info(DOIP, "VIR Received"); }

void DoIpServerSystem::shutdownDone(::transport::AbstractTransportLayer& /*transportLayer*/) {}

DoIpServerTransportConnection& DoIpServerSystem::TransportConnectionCreator::createConnection(
    DoIpServerTransportConnection* memory,
    uint8_t const socketGroupId,
    ::tcp::AbstractSocket& socket,
    ::etl::ipool& diagnosticSendJobBlockPool,
    ::etl::ipool& protocolSendJobBlockPool,
    DoIpServerTransportConnectionConfig const& config,
    DoIpTcpConnection::ConnectionType /*type*/)
{
    // type Defaulted to PLAIN Connection Type
    auto* const p = new (memory) DoIpServerTransportConnection(
        doip::DoIpConstants::ProtocolVersion::version02Iso2012,
        socketGroupId,
        ::etl::ref(socket),
        config,
        ::etl::ref(diagnosticSendJobBlockPool),
        ::etl::ref(protocolSendJobBlockPool),
        DoIpTcpConnection::ConnectionType::PLAIN);

    return *p;
}

} // namespace doip
