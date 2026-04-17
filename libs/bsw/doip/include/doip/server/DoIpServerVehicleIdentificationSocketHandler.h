// Copyright 2025 Accenture.

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
    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param socket reference to datagram socket for receiving/sending datagrams
     * \param socketGroupId corresponding socket group identifier
     * \param networkInterfaceConfigKey key that statically identifies the network address to
     * bind to \param multicastAddress multicast address to use for IPv6, or if IPv4 broadcast
     * address 255.255.255.255 then this will be used and not the directed subnet broadcast
     * \param unicastAddressArray array for storing unicast addresses along with the socket
     * \param config config to use
     * \param announceCount number of announcements (ISO13400: A_DoIP_Announce_Num)
     */
    DoIpServerVehicleIdentificationSocketHandler(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::udp::AbstractDatagramSocket& socket,
        uint8_t socketGroupId,
        ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
        ::ip::IPAddress const& multicastAddress,
        ::etl::ivector<::ip::IPAddress>& unicastAddresses,
        DoIpServerVehicleIdentificationConfig& config,
        uint8_t announceCount);

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

private:
    static constexpr size_t MAX_NUM_UNICAST = 8U;

    using StaticPayloadSendJobType = declare::DoIpStaticPayloadSendJob<32>;

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
    DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
    createResponseIdentification();
    DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
    createResponseEntityStatus();
    DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
    createResponseDiagnosticPowerModeInfo();
    DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
    createResponseOemMessage(IDoIpUdpOemMessageHandler const* const oemMessageHandler);
    DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
    createResponseHeaderNack(uint8_t nackCode);
    StaticPayloadSendJobType&
    createResponse(DoIpServerVehicleIdentificationRequest::Type const& type, uint8_t nackCode);

    StaticPayloadSendJobType& allocateSendJob(uint16_t payloadType, uint8_t payloadLength);
    void releaseSendJob();
    void releaseSendJobAndSendNext(DoIpStaticPayloadSendJob& sendJob, bool success);

    DoIpUdpConnection _connection;
    ::ip::IPAddress _multicastAddress;
    ::ip::IPEndpoint _destinationEndpoint;
    DoIpServerVehicleIdentificationConfig& _config;
    IDoIpVehicleAnnouncementListener* _vehicleAnnouncementListener;
    ::async::Function _enqueueInitialBroadcastsUnicastAsync;
    ::async::Function _configChangedContinuationAsync;
    ::ip::ConfigChangedSlotType _configChangedSlot;
    ::async::TimeoutType _timeoutTimeout;
    ::etl::intrusive_forward_list<DoIpServerVehicleIdentificationRequest, ::etl::forward_link<0>>
        _pendingRequests;
    ::etl::ivector<ip::IPAddress>& _unicastAddresses;
    ::etl::optional<StaticPayloadSendJobType> _sendJob;
    DoIpConstants::ProtocolVersion _protocolVersion;
    ::ip::NetworkInterfaceConfigKey _networkInterfaceConfigKey;
    ::ip::NetworkInterfaceConfig _configChangedNewConfig;
    uint8_t _socketGroupId;
    uint8_t _readBuffer[19];
    ::etl::bitset<MAX_NUM_UNICAST> _newUnicastAddresses;
    uint8_t const _announceCount;
    IDoIpUdpOemMessageHandler const* _currentOemMessageHandler = nullptr;
};

namespace declare
{
/**
 * This class instantiates a socket handler with all needed buffers.
 * \tparam DatagramSocket UDP datagram socket type
 * \tparam NUM_ANNOUNCEMENTS number of announcements (ISO13400: A_DoIP_Announce_Num)
 * \tparam NUM_UNICAST number of unicast addresses to store with the socket handler
 */
template<class DatagramSocket, uint8_t NUM_ANNOUNCEMENTS, size_t NUM_UNICAST = 1U>
class DoIpServerVehicleIdentificationSocketHandler
: public ::doip::DoIpServerVehicleIdentificationSocketHandler
{
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
template<class DatagramSocket, uint8_t NUM_ANNOUNCEMENTS, size_t NUM_UNICAST>
DoIpServerVehicleIdentificationSocketHandler<DatagramSocket, NUM_ANNOUNCEMENTS, NUM_UNICAST>::
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
    NUM_ANNOUNCEMENTS)
{}

// sockets are managed from the outside
template<class DatagramSocket, uint8_t NUM_ANNOUNCEMENTS, size_t NUM_UNICAST>
DatagramSocket&
DoIpServerVehicleIdentificationSocketHandler<DatagramSocket, NUM_ANNOUNCEMENTS, NUM_UNICAST>::
    getSocket()
{
    return _socket;
}

} // namespace declare

} // namespace doip
