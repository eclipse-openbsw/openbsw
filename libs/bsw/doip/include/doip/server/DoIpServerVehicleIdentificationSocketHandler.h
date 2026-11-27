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
#include "doip/common/DoIpStaticPayloadSendJob.h"
#include "doip/common/DoIpUdpConnection.h"
#include "doip/server/DoIpServerVehicleIdentificationRequest.h"

#include <async/Types.h>
#include <async/util/Call.h>
#include <etl/intrusive_links.h>
#include <ip/NetworkInterfaceConfig.h>

#include <etl/array.h>
#include <etl/bitset.h>
#include <etl/intrusive_forward_list.h>
#include <etl/optional.h>
#include <etl/span.h>
#include <etl/vector.h>

namespace doip
{
class DoIpServerVehicleIdentificationConfig;
class IDoIpVehicleAnnouncementListener;
class IDoIpUdpOemMessageHandler;

/**
 * Implements a vehicle identification service.
 */
class DoIpServerVehicleIdentificationSocketHandler
: private IDoIpConnectionHandler
, private ::async::RunnableType
{
public:
    // accommodate at least the largest amount of data we must receive: Vehicle Announcement (17
    // bytes vin + 2 bytes address)
    static constexpr size_t MIN_RX_BUFFER_SIZE = 19U;
    // accommodate at least the largest amount of data we must send: Vehicle Announcement (17 bytes
    // vin + 2 bytes address + EID + GID + Further Action)
    static constexpr size_t MIN_TX_BUFFER_SIZE = 32U;

    /**
     * Set optional vehicle announcement listener.
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
     * Get the unicast addresses stored with the socket.
     * \return span of unicast addresses
     */
    ::etl::span<ip::IPAddress const> getUnicastAddresses() const;

    /**
     * Schedule a single announcement.
     */
    void sendAnnouncement();

    /**
     * Starts the handler. Waits for a valid network configuration and starts sending out as
     * soon as configured.
     */
    void start();

    /**
     * Stops the handler.
     */
    void shutdown();

protected:
    /**
     * Constructor.
     * Protected to make sure that only the memory-owning declare:: derivation can
     * instatiate it
     * \param protocolVersion doip protocol version used for all communication
     * \param socket reference to datagram socket for receiving/sending datagrams
     * \param socketGroupId corresponding socket group identifier
     * \param networkInterfaceConfigKey key that statically identifies the network address to bind
     * to
     * \param multicastAddress multicast address to use for IPv6, or if IPv4 broadcast address
     * 255.255.255.255 then this will be used and not the directed subnet broadcast
     * \param unicastAddresses array for storing unicast addresses along with the socket
     * \param config config to use
     * \param announceCount number of announcements (ISO13400: A_DoIP_Announce_Num)
     * \param recvBuffer buffer used for receiving data
     * \param sendBuffer buffer used for sending data
     */
    DoIpServerVehicleIdentificationSocketHandler(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::udp::AbstractDatagramSocket& socket,
        uint8_t socketGroupId,
        ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
        ::ip::IPAddress const& multicastAddress,
        ::etl::ivector<::ip::IPAddress>& unicastAddresses,
        DoIpServerVehicleIdentificationConfig& config,
        uint8_t announceCount,
        ::etl::span<uint8_t> const& recvBuffer,
        ::etl::span<uint8_t> const& sendBuffer);

    ~DoIpServerVehicleIdentificationSocketHandler() = default;

private:
    static constexpr size_t MAX_NUM_UNICAST = 8U;

    // static uint16_t const EVENT_UNICAST = 0U;
    // static uint16_t const EVENT_TIMEOUT = 1U;

    void connectionClosed(bool closedByRemotePeer) override;
    IDoIpConnectionHandler::HeaderReceivedContinuation
    headerReceived(DoIpHeader const& header) override;

    void enqueueInitialBroadcastsUnicastAsync();
    void configChangedContinuationAsync();

    void execute() override;

    void
    configChanged(::ip::NetworkInterfaceConfigKey key, ::ip::NetworkInterfaceConfig const& config);
    void startAnnounce();

    bool checkVersion(DoIpHeader const& header);
    bool handleRequest(DoIpHeader const& header);

    bool handleVehicleIdentificationRequestMessage(DoIpHeader const& header);
    bool handleVehicleIdentificationRequestMessageEid(DoIpHeader const& header);
    bool handleVehicleIdentificationRequestMessageVin(DoIpHeader const& header);
    bool handleEntityStatusRequest(DoIpHeader const& header);
    bool handleDiagnosticPowerModeInformationRequest(DoIpHeader const& header);
    bool handleVehicleAnnouncementMessage(DoIpHeader const& header);
    bool handleOemRequest(DoIpHeader const& header);
    void vehicleIdentificationEidRequestReceived(::etl::span<uint8_t const> payload);
    void vehicleIdentificationVinRequestReceived(::etl::span<uint8_t const> payload);
    void vehicleAnnouncementPayloadReceived(::etl::span<uint8_t const> payload);
    void oemMessagePayloadReceived(::etl::span<uint8_t const> payload);

    void enqueueResponse(DoIpServerVehicleIdentificationRequest::Type const& type);
    void enqueueNack(uint8_t nackCode);
    void enqueueInitialBroadcasts(::ip::IPEndpoint const& endpoint);
    void enqueueAny(
        DoIpServerVehicleIdentificationRequest::Type const& type,
        uint8_t nackCode,
        ::ip::IPEndpoint const& endpoint,
        uint16_t timeoutInMs);
    void scheduleMessage();

    void trySendResponse();
    void sendResponse(
        ::ip::IPEndpoint const& destinationEndpoint,
        DoIpServerVehicleIdentificationRequest::Type const& type,
        uint8_t nackCode);
    DoIpStaticPayloadSendJob& createResponseIdentification();
    DoIpStaticPayloadSendJob& createResponseEntityStatus();
    DoIpStaticPayloadSendJob& createResponseDiagnosticPowerModeInfo();
    DoIpStaticPayloadSendJob&
    createResponseOemMessage(IDoIpUdpOemMessageHandler const* const oemMessageHandler);
    DoIpStaticPayloadSendJob& createResponseHeaderNack(uint8_t nackCode);
    DoIpStaticPayloadSendJob&
    createResponse(DoIpServerVehicleIdentificationRequest::Type const& type, uint8_t nackCode);

    DoIpStaticPayloadSendJob& allocateSendJob(uint16_t payloadType, uint8_t payloadLength);
    void releaseSendJob();
    void releaseSendJobAndSendNext(DoIpStaticPayloadSendJob& sendJob, bool success);

    DoIpUdpConnection _connection;
    ::ip::IPAddress _multicastAddress;
    ::ip::IPEndpoint _destinationEndpoint;
    DoIpServerVehicleIdentificationConfig& _config;
    IDoIpVehicleAnnouncementListener* _vehicleAnnouncementListener{nullptr};
    ::async::Function _enqueueInitialBroadcastsUnicastAsync;
    ::async::Function _configChangedContinuationAsync;
    ::ip::ConfigChangedSlotType _configChangedSlot;
    ::async::TimeoutType _timeoutTimeout;
    ::etl::intrusive_forward_list<DoIpServerVehicleIdentificationRequest, ::etl::forward_link<0>>
        _pendingRequests;
    ::etl::ivector<ip::IPAddress>& _unicastAddresses;
    ::etl::optional<DoIpStaticPayloadSendJob> _sendJob;
    DoIpConstants::ProtocolVersion _protocolVersion;
    ::ip::NetworkInterfaceConfigKey _networkInterfaceConfigKey;
    ::ip::NetworkInterfaceConfig _configChangedNewConfig;
    uint8_t _socketGroupId;
    ::etl::bitset<MAX_NUM_UNICAST> _newUnicastAddresses;
    uint8_t const _announceCount;
    IDoIpUdpOemMessageHandler const* _currentOemMessageHandler = nullptr;
    ::etl::span<uint8_t> const _rxBuffer;
    ::etl::span<uint8_t> const _txBuffer;
};

namespace declare
{
/**
 * This class instantiates a socket handler with all needed buffers.
 * \tparam DatagramSocket UDP datagram socket type
 * \tparam NUM_ANNOUNCEMENTS number of announcements (ISO13400: A_DoIP_Announce_Num)
 * \tparam RX_LEN size of buffer used for reading
 * \tparam TX_LEN size of buffer used for sending
 * \tparam NUM_UNICAST number of unicast addresses to store with the socket handler
 */
template<
    class DatagramSocket,
    uint8_t NUM_ANNOUNCEMENTS,
    size_t RX_LEN,
    size_t TX_LEN,
    size_t NUM_UNICAST = 1U>
class DoIpServerVehicleIdentificationSocketHandler final
: public ::doip::DoIpServerVehicleIdentificationSocketHandler
{
    ETL_STATIC_ASSERT(
        RX_LEN >= MIN_RX_BUFFER_SIZE,
        "DoIpServerVehicleIdentificationSocketHandlerConfig must allow reception of Vehicle "
        "Announcement");
    ETL_STATIC_ASSERT(
        TX_LEN >= MIN_TX_BUFFER_SIZE,
        "DoIpServerVehicleIdentificationSocketHandlerConfig must allow sending of Vehicle "
        "Announcement");

public:
    DoIpServerVehicleIdentificationSocketHandler(
        DoIpConstants::ProtocolVersion const protocolVersion,
        uint8_t socketGroupId,
        ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
        ::ip::IPAddress const& multicastAddress,
        DoIpServerVehicleIdentificationConfig& config);

    DatagramSocket& getSocket();

private:
    DatagramSocket _socket;
    ::etl::vector<::ip::IPAddress, NUM_UNICAST> _unicastAddresses;
    ::etl::array<uint8_t, RX_LEN> _rxBuffer;
    ::etl::array<uint8_t, TX_LEN> _txBuffer;
};

} // namespace declare

/**
 * Implementations.
 */
inline void DoIpServerVehicleIdentificationSocketHandler::setVehicleAnnouncementListener(
    IDoIpVehicleAnnouncementListener* const vehicleAnnouncementListener)
{
    _vehicleAnnouncementListener = vehicleAnnouncementListener;
}

namespace declare
{
template<
    class DatagramSocket,
    uint8_t NUM_ANNOUNCEMENTS,
    size_t RX_LEN,
    size_t TX_LEN,
    size_t NUM_UNICAST>
DoIpServerVehicleIdentificationSocketHandler<
    DatagramSocket,
    NUM_ANNOUNCEMENTS,
    RX_LEN,
    TX_LEN,
    NUM_UNICAST>::
    DoIpServerVehicleIdentificationSocketHandler(
        DoIpConstants::ProtocolVersion const protocolVersion,
        uint8_t const socketGroupId,
        ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
        ::ip::IPAddress const& multicastAddress,
        DoIpServerVehicleIdentificationConfig& config)
: ::doip::DoIpServerVehicleIdentificationSocketHandler(
    protocolVersion,
    _socket,
    socketGroupId,
    networkInterfaceConfigKey,
    multicastAddress,
    _unicastAddresses,
    config,
    NUM_ANNOUNCEMENTS,
    _rxBuffer,
    _txBuffer)
{}

// sockets are managed from the outside
template<
    class DatagramSocket,
    uint8_t NUM_ANNOUNCEMENTS,
    size_t RX_LEN,
    size_t TX_LEN,
    size_t NUM_UNICAST>
DatagramSocket& DoIpServerVehicleIdentificationSocketHandler<
    DatagramSocket,
    NUM_ANNOUNCEMENTS,
    RX_LEN,
    TX_LEN,
    NUM_UNICAST>::getSocket()
{
    return _socket;
}

} // namespace declare

} // namespace doip
