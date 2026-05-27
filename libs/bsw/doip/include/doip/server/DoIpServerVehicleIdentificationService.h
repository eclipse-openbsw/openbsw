/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/common/DoIpConstants.h"
#include "doip/server/DoIpServerVehicleIdentificationConfig.h"
#include "doip/server/DoIpServerVehicleIdentificationSocketHandler.h"

#include <etl/functional.h>
#include <etl/pool.h>
#include <etl/span.h>
#include <etl/vector.h>

namespace doip
{
namespace declare
{
/**
 * Implementation of a DoIP vehicle identification server that can work on a given
 * number of UDP sockets that can be configured separately.
 * \tparam DatagramSocket UDP datagram socket type
 * \tparam NUM_UNICAST number of unicast addresses to store with the socket
 * \tparam NUM_SOCKETS number of sockets needed
 * \tparam NUM_REQUESTS number of requests that can simultaneously be processed
 * \tparam NUM_ANNOUNCEMENTS number of unicast/multicast announcements to be emitted
 */
template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
class DoIpServerVehicleIdentificationService
{
public:
    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param context async context used for scheduling
     * \param vehicleIdentificationCallback reference to interface that delivers the vehicle
     *        identification data on demand
     * \param entityStatusCallback reference to interface that provides information about
     *        entity status
     * \param networkInterfaceConfigRegistry reference to network interface registry
     * \param logicalEntityAddress the logical address of the device this service represents
     * \param parameterProvider reference to parameter provider for the identification service
     */
    DoIpServerVehicleIdentificationService(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::async::ContextType context,
        IDoIpServerVehicleIdentificationCallback& vehicleIdentificationCallback,
        IDoIpServerEntityStatusCallback& entityStatusCallback,
        ::ip::NetworkInterfaceConfigRegistry& networkInterfaceConfigRegistry,
        uint16_t logicalEntityAddress,
        IDoIpServerVehicleAnnouncementParameterProvider const& parameterProvider);

    /**
     * Initializes a UDP socket on which the service should operate.
     * \param socketGroupId corresponding socket group id of server transport layer
     * \param configKey reference to the key that statically identifies the address to bind to
     * \param multicastAddress the multicast address to join to. For a IPv4 socket this argument is
     * ignored \return reference to the socket handler
     */
    DoIpServerVehicleIdentificationSocketHandler<DatagramSocket, NUM_ANNOUNCEMENTS, NUM_UNICAST>&
    addSocket(
        uint8_t socketGroupId,
        ::ip::NetworkInterfaceConfigKey const& configKey,
        ::ip::IPAddress const& multicastAddress);

    /**
     * Set optional vehicle announcement listener to all sockets. The listener will only be set in
     * all already added sockets. Sockets added later will not automatically get the listener set.
     * \param vehicleAnnouncementListener listener to set
     */
    void
    setVehicleAnnouncementListener(IDoIpVehicleAnnouncementListener* vehicleAnnouncementListener);

    /**
     * Update unicast addresses to announce to.
     * \param configKey reference to network interface these addresses are valid for
     * \param unicastAddresses span of addresses to announce
     */
    void updateUnicastAddresses(
        ::ip::NetworkInterfaceConfigKey const& configKey,
        ::etl::span<ip::IPAddress const> unicastAddresses);

    /**
     * Sends a vehicle announcement on all configured sockets.
     */
    void sendAnnouncement();

    /**
     * Starts the vehicle identification service on all configured UDP sockets.
     */
    void start();

    /**
     * Stops the vehicle identification service.
     */
    void shutdown();

private:
    using SocketHandlerVector = ::etl::vector<
        DoIpServerVehicleIdentificationSocketHandler<
            DatagramSocket,
            NUM_ANNOUNCEMENTS,
            NUM_UNICAST>,
        NUM_SOCKETS>;

    DoIpServerVehicleIdentificationConfig _config;
    ::etl::pool<
        DoIpServerVehicleIdentificationRequest,
        NUM_REQUESTS + NUM_ANNOUNCEMENTS + NUM_ANNOUNCEMENTS * NUM_UNICAST>
        _requestPool;
    SocketHandlerVector _socketHandlers;
};

} // namespace declare

/**
 * Inline implementations.
 */
namespace declare
{
template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::
    DoIpServerVehicleIdentificationService(
        DoIpConstants::ProtocolVersion const protocolVersion,
        ::async::ContextType const context,
        IDoIpServerVehicleIdentificationCallback& vehicleIdentificationCallback,
        IDoIpServerEntityStatusCallback& entityStatusCallback,
        ::ip::NetworkInterfaceConfigRegistry& networkInterfaceConfigRegistry,
        uint16_t const logicalEntityAddress,
        IDoIpServerVehicleAnnouncementParameterProvider const& parameterProvider)
: _config(
    protocolVersion,
    context,
    vehicleIdentificationCallback,
    entityStatusCallback,
    networkInterfaceConfigRegistry,
    logicalEntityAddress,
    parameterProvider,
    _requestPool)
{}

template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
DoIpServerVehicleIdentificationSocketHandler<DatagramSocket, NUM_ANNOUNCEMENTS, NUM_UNICAST>&
DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::
    addSocket(
        uint8_t const socketGroupId,
        ::ip::NetworkInterfaceConfigKey const& configKey,
        ::ip::IPAddress const& multicastAddress)
{
    return _socketHandlers.emplace_back(
        _config.getProtocolVersion(),
        socketGroupId,
        configKey,
        multicastAddress,
        ::etl::ref(_config));
}

template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
void DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::setVehicleAnnouncementListener(IDoIpVehicleAnnouncementListener* const
                                                           vehicleAnnouncementListener)
{
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.setVehicleAnnouncementListener(vehicleAnnouncementListener);
    }
}

template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
void DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::
    updateUnicastAddresses(
        ::ip::NetworkInterfaceConfigKey const& configKey,
        ::etl::span<ip::IPAddress const> const unicastAddresses)
{
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.updateUnicastAddresses(configKey, unicastAddresses);
    }
}

template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
void DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::sendAnnouncement()
{
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.sendAnnouncement();
    }
}

template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
void DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::start()
{
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.start();
    }
}

template<
    class DatagramSocket,
    size_t NUM_UNICAST,
    size_t NUM_SOCKETS,
    size_t NUM_REQUESTS,
    uint8_t NUM_ANNOUNCEMENTS>
void DoIpServerVehicleIdentificationService<
    DatagramSocket,
    NUM_UNICAST,
    NUM_SOCKETS,
    NUM_REQUESTS,
    NUM_ANNOUNCEMENTS>::shutdown()
{
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.shutdown();
    }
}

} // namespace declare
} // namespace doip
