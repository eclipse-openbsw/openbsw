// Copyright 2025 Accenture.

#include "doip/server/DoIpServerVehicleIdentificationSocketHandler.h"

#include "bsp/timer/SystemTimerMock.h"
#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpVehicleAnnouncementListenerMock.h"
#include "doip/server/DoIpServerEntityStatusCallbackMock.h"
#include "doip/server/DoIpServerVehicleAnnouncementParameterProviderMock.h"
#include "doip/server/DoIpServerVehicleIdentificationCallbackMock.h"
#include "doip/server/DoIpServerVehicleIdentificationConfig.h"
#include "doip/server/DoIpUdpOemMessageHandlerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <common/busid/BusId.h>
#include <cpp2ethernet/gtest_extensions.h>
#include <ip/NetworkInterfaceConfigRegistryMock.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <estd/memory.h>

#include <gtest/esr_extensions.h>

namespace
{
using namespace ::testing;
using namespace ::doip;
using namespace ::ethernet::test;

static uint8_t vin[]
    = {'B', 'A', 'U', 'I', 'E', 'E', '4', 'M', 'Z', '1', '5', '0', '1', '2', '9', '0', '8'};
static uint8_t eid[] = {0x00, 0x1e, 0xae, 0x01, 0x02, 0x42};
static uint8_t gid[] = {0x01, 0x2f, 0xae, 0x01, 0x02, 0x44};

MATCHER_P(IsDatagram, endpoint, "")
{
    ::udp::DatagramPacket const& argDatagram = arg;
    return (argDatagram.getEndpoint() == endpoint);
}

MATCHER_P2(IsDatagram, endpoint, buffer, "")
{
    ::udp::DatagramPacket const& argDatagram = arg;
    return (argDatagram.getEndpoint() == endpoint)
           && (::estd::memory::is_equal(
               ::estd::slice<uint8_t const>::from_pointer(
                   argDatagram.getData(), argDatagram.getLength()),
               buffer));
}

ACTION_P(CopySlice, src) { ::estd::memory::copy(arg0.template reinterpret_as<uint8_t>(), src); }

struct DoIpServerVehicleIdentificationSocketHandlerTest : Test
{
    DoIpServerVehicleIdentificationSocketHandlerTest()
    : asyncMock()
    , asyncContext(1U)
    , testContext(asyncContext)
    , fConfig(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          asyncContext,
          fVehicleIdentificationCallbackMock,
          fEntityStatusCallbackMock,
          fNetworkInterfaceConfigRegistryMock,
          0x0401U,
          fParametersMock,
          fRequestPool)
    , fBuffer()
    , fFreeBuffer(fBuffer)
    {
        ON_CALL(fParametersMock, getAnnounceWait()).WillByDefault(Return(0x150U));
        ON_CALL(fParametersMock, getAnnounceInterval()).WillByDefault(Return(0x500U));
    }

    ~DoIpServerVehicleIdentificationSocketHandlerTest() override {}

    void SetUp() override { testContext.handleAll(); }

    void expectAnnouncement(
        ::ip::IPEndpoint const& remoteEndpoint, uint32_t epochTime, Sequence* seq = nullptr);
    void expectNackResponse(
        ::ip::IPEndpoint const& destinationEndpoint, uint8_t nackCode, Sequence* seq = nullptr);
    void expectNoResponse(::ip::IPEndpoint const& destinationEndpoint);
    void expectVehicleIdentificationResponse(
        ::ip::IPEndpoint const& destinationEndpoint, Sequence* seq = nullptr);
    void expectDiagnosticPowerModeResponse(
        ::ip::IPEndpoint const& destinationEndpoint, ::estd::slice<uint8_t const> const& response);
    void expectEntityStatusResponse(
        ::ip::IPEndpoint const& destinationEndpoint,
        uint8_t socketGroupId,
        ::estd::slice<uint8_t const> const& response);
    void tick(uint32_t epochTime);
    void receiveRequest(
        ::ip::NetworkInterfaceConfig& config,
        ::ip::IPEndpoint& remoteEndpoint,
        ::estd::slice<uint8_t const> const& request,
        uint32_t epochTime,
        Sequence* seq = nullptr);
    void requestInvalid(
        ::ip::NetworkInterfaceConfig& config,
        ::ip::IPEndpoint& remoteEndpoint,
        ::estd::slice<uint8_t const> const& request);

    ::estd::slice<uint8_t> allocateBuffer(uint32_t size)
    {
        ::estd::slice<uint8_t> buffer = fFreeBuffer.subslice(size);
        fFreeBuffer                   = fFreeBuffer.offset(size);
        return buffer;
    }

    ::testing::NiceMock<DoIpServerVehicleAnnouncementParameterProviderMock> fParametersMock;
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::testing::StrictMock<SystemTimerMock> timerMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    ::udp::AbstractDatagramSocketMock* fSocketMock = nullptr;
    DoIpServerEntityStatusCallbackMock fEntityStatusCallbackMock;
    DoIpServerVehicleIdentificationCallbackMock fVehicleIdentificationCallbackMock;
    ::testing::StrictMock<DoIpUdpOemMessageHandlerMock> oemMessageHandlerMock;
    DoIpVehicleAnnouncementListenerMock fVehicleAnnouncementListenerMock;
    ::ip::NetworkInterfaceConfigRegistryMock fNetworkInterfaceConfigRegistryMock;
    DoIpServerVehicleIdentificationConfig fConfig;
    ::estd::declare::object_pool<DoIpServerVehicleIdentificationRequest, 9U> fRequestPool;
    uint8_t fBuffer[2000];
    ::estd::slice<uint8_t> fFreeBuffer;
};

TEST_F(
    DoIpServerVehicleIdentificationSocketHandlerTest,
    LifecycleWithBroadcastsWhenAddressIsConfigured)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                13U,
                configKey,
                multicastAddress,
                fConfig);

    // expect no unicast addresses to be stored with the instance
    EXPECT_EQ(0U, cut.getUnicastAddresses().size());

    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    ::ip::IPAddress bindAddress;
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(DoAll(
            SaveArgPointee<0>(&bindAddress),
            Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK)));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    uint32_t timestamp = 0;
    cut.start();
    testContext.expireAndExecute();
    EXPECT_EQ(bindAddress, config.ipAddress());
    Mock::VerifyAndClearExpectations(&timerMock);

    // wait up to end of wait time and expect first announcement
    timestamp += fParametersMock.getAnnounceWait() - 1;
    tick(timestamp);
    timestamp += 1;
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    // wait up to end of interval time and expect second message
    timestamp += fParametersMock.getAnnounceInterval() - 1;
    tick(timestamp);
    timestamp += 1;
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    // wait up to end of interval time and expect third message
    timestamp += fParametersMock.getAnnounceInterval() - 1;
    tick(timestamp);
    timestamp += 1;
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    // No more message should be emitted
    timestamp += fParametersMock.getAnnounceWait() * 1000;
    tick(timestamp);

    // now shutdown
    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, JoinMulticastGroupIfIPv6Configured)
{
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getVin(_))
        .WillRepeatedly(CopySlice(::estd::make_slice(vin)));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getEid(_))
        .WillRepeatedly(CopySlice(::estd::make_slice(eid)));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getGid(_))
        .WillRepeatedly(CopySlice(::estd::make_slice(gid)));

    uint32_t const multicast[] = {0x123U, 0x34234U, 0x84984U, 0x3482348U};
    uint32_t const local[]     = {0x983423U, 0x34234U, 0x84984U, 0x43894829U};
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip6(multicast);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                14U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(local);

    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey))
        .WillOnce(Return(::ip::NetworkInterfaceConfig()));

    cut.start();
    fSocketMock = &cut.getSocket();
    ::ip::IPAddress bindAddress;
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(DoAll(
            SaveArgPointee<0>(&bindAddress),
            Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK)));
    EXPECT_CALL(*fSocketMock, join(multicastAddress))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));

    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0));
    fNetworkInterfaceConfigRegistryMock.configChangedSignal(configKey, config);
    testContext.expireAndExecute();
    EXPECT_EQ(bindAddress, config.ipAddress());

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();

    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, IgnoreUnknownNetworkConfig)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                15U,
                configKey,
                multicastAddress,
                fConfig);

    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);

    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey))
        .WillOnce(Return(::ip::NetworkInterfaceConfig()));
    cut.start();

    fNetworkInterfaceConfigRegistryMock.configChangedSignal(
        ::ip::NetworkInterfaceConfigKey(1U), config);

    cut.shutdown();

    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, VehicleIdentificationRequest)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                16U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));

    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    uint32_t timestamp = 0;
    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&timerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    timestamp += fParametersMock.getAnnounceWait() - 1;
    tick(timestamp);
    timestamp += 1;
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    // invalid payload length
    receiveRequest(
        config, remoteEndpoint, {{0xff, 0x0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01}}, timestamp);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    timestamp += 1;
    tick(timestamp);

    // unknown payload type (with version 0xff)
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(_)).Times(0);
    receiveRequest(
        config, remoteEndpoint, {{0xff, 0x0, 0x40, 0x0f, 0x00, 0x00, 0x00, 0x01}}, timestamp);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE);
    tick(timestamp);

    // unknown payload type (with version 0x02)
    timestamp += 1;
    receiveRequest(
        config, remoteEndpoint, {{0x02, 0xfd, 0x40, 0x0f, 0x00, 0x00, 0x00, 0x01}}, timestamp);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE);
    timestamp += 1;
    tick(timestamp);

    // now a valid request with vehicle identification version
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(1U));
    receiveRequest(
        config, remoteEndpoint, {{0xff, 0x0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}}, timestamp);
    timestamp += 1;
    expectAnnouncement(remoteEndpoint, timestamp);
    tick(timestamp);

    // a valid request with default version
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(0U));
    receiveRequest(
        config, remoteEndpoint, {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}}, timestamp);
    expectAnnouncement(remoteEndpoint, timestamp);
    tick(timestamp);

    // no response
    requestInvalid(config, remoteEndpoint, {{0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}});
    expectNoResponse(remoteEndpoint);
    tick(timestamp);

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, VehicleIdentificationRequestWithEid)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                1U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    uint32_t timestamp = 0;
    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&timerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    timestamp = fParametersMock.getAnnounceWait();
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    // Invalid payload length
    timestamp += 1;
    tick(timestamp);
    receiveRequest(
        config, remoteEndpoint, {{0xff, 0x0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05}}, timestamp);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(timestamp);

    // With invalid EID
    timestamp += 1;
    requestInvalid(
        config,
        remoteEndpoint,
        {{0xff, 0x0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x1e, 0xae, 0x04, 0x07, 0x42}});
    expectNoResponse(remoteEndpoint);
    tick(timestamp);

    // Now with valid EID
    timestamp += 1;
    tick(timestamp);
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(2U));
    receiveRequest(
        config,
        remoteEndpoint,
        {{0xff, 0x0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x1e, 0xae, 0x01, 0x02, 0x42}},
        timestamp);
    timestamp += 1;
    expectNoResponse(remoteEndpoint);
    tick(timestamp);
    timestamp += 1;
    expectAnnouncement(remoteEndpoint, timestamp);
    tick(timestamp);

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, VehicleIdentificationRequestWithVin)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                2U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    uint32_t timestamp = 0;
    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&timerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    timestamp += fParametersMock.getAnnounceWait() - 1;
    tick(timestamp);
    // wait timeout should end
    timestamp += 1;
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    // Invalid payload length
    receiveRequest(
        config,
        remoteEndpoint,
        {{0xff, 0x0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10}},
        fParametersMock.getAnnounceWait());
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(timestamp);

    // With invalid VIN
    uint8_t const invalidVin[]
        = {0xff, 0x0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11, 'B', 'E', 'U', 'I', 'E',
           'E',  '4', 'M',  'Z',  '1',  '5',  '0',  '1',  '2', '7', '0', '8'};
    requestInvalid(config, remoteEndpoint, invalidVin);
    expectNoResponse(remoteEndpoint);
    tick(timestamp);

    // Now with valid VIN
    uint8_t const validRequest[]
        = {0xff, 0x0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11, 'B', 'A', 'U', 'I', 'E',
           'E',  '4', 'M',  'Z',  '1',  '5',  '0',  '1',  '2', '9', '0', '8'};

    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(55U));
    receiveRequest(config, remoteEndpoint, validRequest, timestamp);
    timestamp += 54;
    tick(timestamp);
    timestamp += 1;
    expectAnnouncement(remoteEndpoint, timestamp);
    tick(timestamp);

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, EntityStatusRequest)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                3U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    cut.start();
    testContext.expireAndExecute();
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    expectAnnouncement(broadcastEndpoint, fParametersMock.getAnnounceWait());
    tick(fParametersMock.getAnnounceWait());

    // Invalid payload length
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x40, 0x01, 0x00, 0x00, 0x00, 0x01}},
        fParametersMock.getAnnounceWait());
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(fParametersMock.getAnnounceWait());
    tick(fParametersMock.getAnnounceWait() + 1);

    // Now valid request
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00}},
        fParametersMock.getAnnounceWait() + 1);
    expectEntityStatusResponse(
        remoteEndpoint,
        3U,
        {{0x02,
          0xfd,
          0x40,
          0x02,
          0x00,
          0x00,
          0x00,
          0x07,
          0x13,
          0x04,
          0x03,
          0x00,
          0x00,
          0x12,
          0x34}});
    tick(fParametersMock.getAnnounceWait() + 2);

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, DiagnosticPowerModeInfoRequest)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                5U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    cut.start();
    testContext.expireAndExecute();
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    expectAnnouncement(broadcastEndpoint, fParametersMock.getAnnounceWait());
    tick(fParametersMock.getAnnounceWait());
    // Invalid payload length
    tick(fParametersMock.getAnnounceWait() + 1);
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x40, 0x03, 0x00, 0x00, 0x00, 0x01}},
        fParametersMock.getAnnounceWait() + 1);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(fParametersMock.getAnnounceWait() + 1);

    // Now valid request
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x40, 0x03, 0x00, 0x00, 0x00, 0x00}},
        fParametersMock.getAnnounceWait() + 1);
    expectDiagnosticPowerModeResponse(
        remoteEndpoint, {{0x02, 0xfd, 0x40, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01}});
    tick(fParametersMock.getAnnounceWait() + 1);

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, VehicleAnnouncementMessage)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                6U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    cut.start();
    testContext.expireAndExecute();
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    expectAnnouncement(broadcastEndpoint, fParametersMock.getAnnounceWait());

    // Packet discarded if no listener
    {
        requestInvalid(config, remoteEndpoint, {{0x02, 0xfd, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00}});
        expectNoResponse(remoteEndpoint);
        tick(fParametersMock.getAnnounceWait());
    }

    // set listener
    cut.setVehicleAnnouncementListener(&fVehicleAnnouncementListenerMock);

    // Invalid payload length (too short)
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x00, 0x04, 0x00, 0x00, 0x00, 0x1f}},
        fParametersMock.getAnnounceWait());
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(fParametersMock.getAnnounceWait());

    // Invalid payload length (too long)
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x00, 0x04, 0x00, 0x00, 0x00, 0x22}},
        fParametersMock.getAnnounceWait() + 1);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(fParametersMock.getAnnounceWait() + 1);

    // Now valid request
    {
        uint8_t const validRequest[]
            = {0x02, 0xfd, 0x00, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
               0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x34, 0x56};
        EXPECT_CALL(*fSocketMock, getLocalPort())
            .WillOnce(Return(DoIpConstants::Ports::UDP_DISCOVERY));
        EXPECT_CALL(*fSocketMock, read(NotNull(), 8U))
            .WillOnce(Invoke(
                ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(validRequest, 8U))));
        EXPECT_CALL(*fSocketMock, read(NotNull(), 19U))
            .WillOnce(Invoke(
                ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(validRequest + 8, 19U))));
        EXPECT_CALL(*fSocketMock, read(0, 13U)).WillOnce(Return(13U));
        EXPECT_CALL(fVehicleAnnouncementListenerMock, vehicleAnnouncementReceived(0x3456, _, _));
        fSocketMock->getDataListener()->dataReceived(
            *fSocketMock,
            remoteEndpoint.getAddress(),
            remoteEndpoint.getPort(),
            config.ipAddress(),
            40U);
        testContext.expireAndExecute();
    }

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, IgnoreUnknownPayloadTypes)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                7U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    cut.start();
    testContext.expireAndExecute();
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    expectAnnouncement(broadcastEndpoint, fParametersMock.getAnnounceWait());

    // Unknown payload type with incorrect pattern
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x03, 0xfe, 0x40, 0x09, 0x00, 0x00, 0x00, 0x00}},
        fParametersMock.getAnnounceWait());
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INCORRECT_PATTERN);
    tick(fParametersMock.getAnnounceWait());

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, SendAnnouncementsToUnicastAddresses)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U, 2U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                8U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    cut.start();
    testContext.expireAndExecute();

    // wait up to end of wait time and expect first announcement
    expectAnnouncement(broadcastEndpoint, fParametersMock.getAnnounceWait());
    tick(fParametersMock.getAnnounceWait());

    // set IP addresses
    ::ip::IPAddress unicastAddresses[2];
    unicastAddresses[0] = ::ip::make_ip4(0x434U);
    unicastAddresses[1] = ::ip::make_ip4(0x234383U);
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit())
        .WillRepeatedly(Return(fParametersMock.getAnnounceWait()));
    cut.updateUnicastAddresses(configKey, unicastAddresses);
    testContext.expireAndExecute();
    testContext.elapse(1000U);
    testContext.expireAndExecute();

    // expect unicast packets first broadcast
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[0], DoIpConstants::Ports::UDP_DISCOVERY),
        2 * fParametersMock.getAnnounceWait());
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[1], DoIpConstants::Ports::UDP_DISCOVERY),
        2 * fParametersMock.getAnnounceWait());
    tick(2 * fParametersMock.getAnnounceWait());

    // expect multicast packets second broadcast
    expectAnnouncement(
        broadcastEndpoint,
        fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());
    tick(fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());

    // expect unicast packets second broadcast
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[0], DoIpConstants::Ports::UDP_DISCOVERY),
        2 * fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[1], DoIpConstants::Ports::UDP_DISCOVERY),
        2 * fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());
    tick(2 * fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());

    // expect multicast packets third broadcast
    expectAnnouncement(
        broadcastEndpoint,
        fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval());
    tick(fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval());

    // expect unicast packets third broadcast
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[0], DoIpConstants::Ports::UDP_DISCOVERY),
        2 * fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval());
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[1], DoIpConstants::Ports::UDP_DISCOVERY),
        2 * fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval());
    tick(2 * fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval());

    // expect nothing more
    uint16_t timeBase
        = 2 * fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval() + 10000;
    tick(timeBase);

    // set IP addresses for another network interface => nothing should happen
    ::ip::NetworkInterfaceConfigKey otherConfigKey(1U);
    unicastAddresses[0] = ::ip::make_ip4(0x4534U);
    unicastAddresses[1] = ::ip::make_ip4(0x234383U);
    cut.updateUnicastAddresses(otherConfigKey, unicastAddresses);
    tick(timeBase + fParametersMock.getAnnounceWait());

    // Change a single address and expect only announcement to the new address
    unicastAddresses[0] = ::ip::make_ip4(0x23438U);
    unicastAddresses[1] = ::ip::make_ip4(0x434U);
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit())
        .WillRepeatedly(Return(timeBase + fParametersMock.getAnnounceWait()));
    cut.updateUnicastAddresses(configKey, unicastAddresses);
    tick(timeBase + fParametersMock.getAnnounceWait());

    // wait up to end of wait time and expect first announcement
    tick(timeBase + 2 * fParametersMock.getAnnounceWait() - 1);
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[0], DoIpConstants::Ports::UDP_DISCOVERY),
        timeBase + 2 * fParametersMock.getAnnounceWait());
    tick(timeBase + 2 * fParametersMock.getAnnounceWait());

    // expect second packet after announce interval
    tick(
        timeBase + 2 * fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval()
        - 1);
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[0], DoIpConstants::Ports::UDP_DISCOVERY),
        timeBase + 2 * fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());
    tick(timeBase + 2 * fParametersMock.getAnnounceWait() + fParametersMock.getAnnounceInterval());

    // expect third packet after announce interval
    tick(
        timeBase + 2 * fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval()
        - 1);
    expectAnnouncement(
        ::ip::IPEndpoint(unicastAddresses[0], DoIpConstants::Ports::UDP_DISCOVERY),
        timeBase + 2 * fParametersMock.getAnnounceWait()
            + 2 * fParametersMock.getAnnounceInterval());
    tick(
        timeBase + 2 * fParametersMock.getAnnounceWait()
        + 2 * fParametersMock.getAnnounceInterval());

    // expect nothing more
    tick(
        timeBase + 2 * fParametersMock.getAnnounceWait() + 2 * fParametersMock.getAnnounceInterval()
        + 100000);
    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, HandleTooManyRequestsWithoutSendSuccess)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                9U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    uint timestamp = 0;
    cut.start();
    testContext.expireAndExecute();
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    timestamp = fParametersMock.getAnnounceWait();
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    {
        // two requests in queue at this point (the initial VAMs); queue size is set to 9
        //  add 8 more requests, the last one won't be responded
        uint8_t const request[] = {0xff, 0x0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
        Sequence seq;
        for (int cnt = 0; cnt < 8; cnt++)
        {
            EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(1U));
            receiveRequest(config, remoteEndpoint, request, timestamp, &seq);
        }
        timestamp += 1;
        for (int cnt = 0; cnt < 7; cnt++)
        {
            expectAnnouncement(remoteEndpoint, timestamp, &seq);
        }
        tick(timestamp);
        expectNoResponse(remoteEndpoint);
        timestamp += 8;
        tick(timestamp);
    }

    {
        Sequence seq;
        // two requests in queue at this point (the initial VAMs); queue size is set to 9
        for (int cnt = 1; cnt <= 8; cnt++)
        {
            receiveRequest(
                config,
                remoteEndpoint,
                {{0xff, 0x0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03}},
                timestamp,
                &seq);
        }

        // expect 7 responses (one was immediately sent, 6 were queued)
        for (int cnt = 1; cnt <= 7; cnt++)
        {
            expectNackResponse(
                remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH, &seq);
        }
        tick(timestamp);
        // run for 100 ms more; nothing should happen
        timestamp += 100;
        tick(timestamp);

        Mock::VerifyAndClearExpectations(&timerMock);
    }

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, AnnouncementTimeouts)
{
    uint32_t epochTime = 0;

    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                13U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    ::ip::IPAddress bindAddress;
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(DoAll(
            SaveArgPointee<0>(&bindAddress),
            Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK)));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(epochTime));
    cut.start();
    testContext.expireAndExecute();
    EXPECT_EQ(bindAddress, config.ipAddress());
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // test plan
    //  0    -> 336  start-up announcement
    //  5    -> 5    VIR nack
    //  50   -> 300  VIR
    //  60   -> 280  VIR
    //  1600 -> 1700 announcement
    //  0    -> 1616 start-up announcement
    //  1800 -> 1800 entitystatus
    //  0    -> 2896 start-up announcement

    // VIR nack epoch time = 5      , requested @5
    receiveRequest(config, remoteEndpoint, {{0xff, 0x0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01}}, 5);
    expectNackResponse(remoteEndpoint, DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(5U);
    // VIR   epoch time = 50     , responded @ 300
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(250U));
    receiveRequest(config, remoteEndpoint, {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}}, 50U);
    // VIR   epoch time = 60     , responded @ 280
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(220U));
    receiveRequest(config, remoteEndpoint, {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}}, 60U);
    // VAM   epoch time = 280    , requested @ 60
    expectAnnouncement(remoteEndpoint, 280U);
    tick(280U);
    // VAM   epoch time = 300    , requested @ 50
    expectAnnouncement(remoteEndpoint, 300U);
    tick(300U);
    // VAM   epoch time = 336    , requested @ 0
    expectAnnouncement(broadcastEndpoint, 336U);
    tick(336U);
    // VIR   epoch time = 1600   , responded @ 1700
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(100U));
    receiveRequest(
        config, remoteEndpoint, {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}}, 1600U);
    // VAM   epoch time = 1616   , requested @ 0
    expectAnnouncement(broadcastEndpoint, 1616U);
    tick(1616U);
    // VAM   epoch time = 1700   , requested @ 1600
    expectAnnouncement(remoteEndpoint, 1700);
    tick(1700U);
    // ES    epoch time = 1800   , requested @ 1800
    receiveRequest(
        config, remoteEndpoint, {{0x02, 0xfd, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00}}, 1800U);
    expectEntityStatusResponse(
        remoteEndpoint,
        13U,
        {{0x02,
          0xfd,
          0x40,
          0x02,
          0x00,
          0x00,
          0x00,
          0x07,
          0x13,
          0x04,
          0x03,
          0x00,
          0x00,
          0x12,
          0x34}});
    tick(1800U);
    // VAM   epoch time = 2896   , requested @ 0
    expectAnnouncement(broadcastEndpoint, 2896U);
    tick(2896U);

    // 10 more seconds pass, nothing should happen
    testContext.elapse(static_cast<uint64_t>(10000U * 1000U));
    testContext.expireAndExecute();
    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, TestImpreciseTime)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                13U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    ::ip::IPAddress bindAddress;
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(DoAll(
            SaveArgPointee<0>(&bindAddress),
            Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK)));
    uint32_t timestamp = 0;
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(timestamp));
    cut.start();
    testContext.expireAndExecute();
    EXPECT_EQ(bindAddress, config.ipAddress());
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    Sequence defaultSeq;
    // test dsync between async and system clock
    // request @ async time = 0; clock time = 0; random = 100;
    // request @ async time = 10; clock time = 5; random = 95;
    // request @ async time = 10; clock time = 15; random = 90;

    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(100U));
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}},
        timestamp,
        &defaultSeq);
    timestamp = 10U;
    tick(timestamp);
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(95U));
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}},
        timestamp - 5U,
        &defaultSeq);
    tick(timestamp);
    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillOnce(Return(90U));
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}},
        timestamp + 5U,
        &defaultSeq);
    timestamp = 95U;
    expectAnnouncement(remoteEndpoint, timestamp, &defaultSeq);
    tick(timestamp);
    timestamp = 100U;
    expectAnnouncement(remoteEndpoint, timestamp, &defaultSeq);
    tick(timestamp);
    timestamp = 105U;
    expectAnnouncement(remoteEndpoint, timestamp, &defaultSeq);
    tick(timestamp);

    testContext.expireAndExecute();
    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

void DoIpServerVehicleIdentificationSocketHandlerTest::expectAnnouncement(
    ::ip::IPEndpoint const& remoteEndpoint, uint32_t epochTime, Sequence* seq)
{
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(epochTime));
    expectVehicleIdentificationResponse(remoteEndpoint, seq);
}

void DoIpServerVehicleIdentificationSocketHandlerTest::expectNackResponse(
    ::ip::IPEndpoint const& destinationEndpoint, uint8_t nackCode, Sequence* seq)
{
    uint8_t const response[] = {0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, nackCode};
    ::estd::slice<uint8_t> responseBuffer = allocateBuffer(sizeof(response));
    ::estd::memory::copy(responseBuffer, response);

    Sequence defaultSeq;
    EXPECT_CALL(
        *fSocketMock,
        send(
            Matcher<::udp::DatagramPacket const&>(IsDatagram(destinationEndpoint, responseBuffer))))
        .InSequence((seq != nullptr) ? *seq : defaultSeq)
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
}

void DoIpServerVehicleIdentificationSocketHandlerTest::expectNoResponse(
    ::ip::IPEndpoint const& destinationEndpoint)
{
    Sequence defaultSeq;
    EXPECT_CALL(
        *fSocketMock, send(Matcher<::udp::DatagramPacket const&>(IsDatagram(destinationEndpoint))))
        .Times(Exactly(0U));
}

void DoIpServerVehicleIdentificationSocketHandlerTest::expectVehicleIdentificationResponse(
    ::ip::IPEndpoint const& destinationEndpoint, Sequence* seq)
{
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getVin(_))
        .WillRepeatedly(CopySlice(::estd::make_slice(vin)));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getEid(_))
        .WillRepeatedly(CopySlice(::estd::make_slice(eid)));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getGid(_))
        .WillRepeatedly(CopySlice(::estd::make_slice(gid)));
    uint8_t const response[]
        = {0x02,
           0xFD,
           0x00,
           0x04,
           0x00,
           0x00,
           0x00,
           0x20,
           // VIN
           'B',
           'A',
           'U',
           'I',
           'E',
           'E',
           '4',
           'M',
           'Z',
           '1',
           '5',
           '0',
           '1',
           '2',
           '9',
           '0',
           '8',
           // Entity Address
           0x04,
           0x01,
           // EID
           0x00,
           0x1e,
           0xae,
           0x01,
           0x02,
           0x42,
           // GID
           0x01,
           0x2f,
           0xae,
           0x01,
           0x02,
           0x44,
           // further action
           0x00};
    ::estd::slice<uint8_t> responseBuffer = allocateBuffer(sizeof(response));
    ::estd::memory::copy(responseBuffer, response);

    Sequence defaultSeq;
    EXPECT_CALL(
        *fSocketMock,
        send(
            Matcher<::udp::DatagramPacket const&>(IsDatagram(destinationEndpoint, responseBuffer))))
        .InSequence(seq ? *seq : defaultSeq)
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, SendAnnouncement)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 0U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                16U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));

    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0));
    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&timerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // wait up to end of wait time and expect first announcement
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).Times(2U).WillRepeatedly(Return(0U));
    cut.sendAnnouncement();
    expectAnnouncement(broadcastEndpoint, fParametersMock.getAnnounceWait());
    tick(fParametersMock.getAnnounceWait());

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

void DoIpServerVehicleIdentificationSocketHandlerTest::expectDiagnosticPowerModeResponse(
    ::ip::IPEndpoint const& destinationEndpoint, ::estd::slice<uint8_t const> const& response)
{
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getPowerMode())
        .WillOnce(Return(DoIpConstants::DiagnosticPowerMode::READY));
    EXPECT_CALL(
        *fSocketMock,
        send(Matcher<::udp::DatagramPacket const&>(
            IsDatagram(destinationEndpoint, ::estd::slice<uint8_t const>(response)))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
}

void DoIpServerVehicleIdentificationSocketHandlerTest::expectEntityStatusResponse(
    ::ip::IPEndpoint const& destinationEndpoint,
    uint8_t socketGroupId,
    ::estd::slice<uint8_t const> const& response)
{
    EXPECT_CALL(fEntityStatusCallbackMock, getEntityStatus(socketGroupId))
        .WillOnce(
            Return(IDoIpServerEntityStatusCallback::EntityStatus(0x13U, 0x04U, 0x03U, 0x1234U)));
    EXPECT_CALL(
        *fSocketMock,
        send(Matcher<::udp::DatagramPacket const&>(
            IsDatagram(destinationEndpoint, ::estd::slice<uint8_t const>(response)))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
}

void DoIpServerVehicleIdentificationSocketHandlerTest::tick(uint32_t epochTime)
{
    testContext.setNow(static_cast<uint64_t>(epochTime) * 1000U);
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(fSocketMock);
    Mock::VerifyAndClearExpectations(&timerMock);
}

void DoIpServerVehicleIdentificationSocketHandlerTest::receiveRequest(
    ::ip::NetworkInterfaceConfig& config,
    ::ip::IPEndpoint& remoteEndpoint,
    ::estd::slice<uint8_t const> const& request,
    uint32_t epochTime,
    Sequence* seq)
{
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(epochTime));
    EXPECT_CALL(*fSocketMock, getLocalPort()).WillOnce(Return(DoIpConstants::Ports::UDP_DISCOVERY));
    Sequence defaultSeq;
    if (seq == nullptr)
    {
        seq = &defaultSeq;
    }
    EXPECT_CALL(*fSocketMock, read(NotNull(), 8))
        .InSequence(*seq)
        .WillOnce(Invoke(ReadBytesFrom(request.subslice(8))));
    // payload
    if (request.size() > 8)
    {
        EXPECT_CALL(*fSocketMock, read(NotNull(), request.size() - 8))
            .InSequence(*seq)
            .WillOnce(Invoke(ReadBytesFrom(request.offset(8))));
    }
    fSocketMock->getDataListener()->dataReceived(
        *fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        config.ipAddress(),
        request.size());
}

void DoIpServerVehicleIdentificationSocketHandlerTest::requestInvalid(
    ::ip::NetworkInterfaceConfig& config,
    ::ip::IPEndpoint& remoteEndpoint,
    ::estd::slice<uint8_t const> const& request)
{
    EXPECT_CALL(*fSocketMock, getLocalPort()).WillOnce(Return(DoIpConstants::Ports::UDP_DISCOVERY));
    // header
    EXPECT_CALL(*fSocketMock, read(NotNull(), 8))
        .WillOnce(Invoke(ReadBytesFrom(request.subslice(8))));
    // payload
    if (request.size() > 8)
    {
        EXPECT_CALL(*fSocketMock, read(NotNull(), request.size() - 8))
            .WillOnce(Invoke(ReadBytesFrom(request.offset(8))));
    }
    expectNoResponse(remoteEndpoint);
    fSocketMock->getDataListener()->dataReceived(
        *fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        config.ipAddress(),
        request.size());
}

TEST_F(DoIpServerVehicleIdentificationSocketHandlerTest, TestOemRequests)
{
    ::ip::NetworkInterfaceConfigKey configKey(0U);
    ::ip::IPAddress multicastAddress = ::ip::make_ip4(0x34384U);
    ::doip::declare::
        DoIpServerVehicleIdentificationSocketHandler<::udp::AbstractDatagramSocketMock, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                1U,
                configKey,
                multicastAddress,
                fConfig);
    ::ip::NetworkInterfaceConfig config(0xc0a80001U, 0xc0a8ffffU, 0x0U);
    ::ip::IPEndpoint broadcastEndpoint(
        config.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey)).WillOnce(Return(config));
    fSocketMock = &cut.getSocket();
    EXPECT_CALL(*fSocketMock, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    uint32_t timestamp = 0;
    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&timerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x4834U), 123U);

    // an OEM request of unknown payload type without payload
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(0xFEFFU))
        .WillOnce(Return(nullptr));
    receiveRequest(
        config, remoteEndpoint, {{0x02, 0xfd, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x00}}, timestamp);
    expectNackResponse(remoteEndpoint, doip::DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE);
    tick(timestamp);
    Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);

    // an OEM request of unknown payload type with payload
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(timestamp));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(0xFEFFU))
        .WillOnce(Return(nullptr));
    uint8_t request[] = {0x02, 0xfd, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x01, 0xFF};
    EXPECT_CALL(*fSocketMock, getLocalPort()).WillOnce(Return(DoIpConstants::Ports::UDP_DISCOVERY));
    // the payload will be discarded
    EXPECT_CALL(*fSocketMock, read(NotNull(), 8))
        .WillOnce(Invoke(ReadBytesFrom(::estd::slice<uint8_t const, 8>(request))));
    fSocketMock->getDataListener()->dataReceived(
        *fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        config.ipAddress(),
        8U);
    expectNackResponse(remoteEndpoint, doip::DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE);
    tick(timestamp);
    Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);

    // an OEM request of known payload type without payload
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(0xFEFF))
        .WillRepeatedly(Return(&oemMessageHandlerMock));
    EXPECT_CALL(oemMessageHandlerMock, getRequestPayloadSize()).WillOnce(Return(0x00U));
    EXPECT_CALL(oemMessageHandlerMock, getRequestPayloadType()).WillOnce(Return(0xFEFF));
    receiveRequest(
        config, remoteEndpoint, {{0x02, 0xfd, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x00}}, timestamp);
    EXPECT_CALL(oemMessageHandlerMock, getResponsePayloadSize()).WillOnce(Return(3U));
    EXPECT_CALL(oemMessageHandlerMock, getResponsePayloadType()).WillOnce(Return(0xFEFE));
    uint8_t responsePayload[] = {0x01, 0x02, 0x03};
    EXPECT_CALL(oemMessageHandlerMock, createResponse(_))
        .WillOnce(Invoke(
            [responsePayload](::estd::slice<uint8_t> response)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    response[i] = responsePayload[i];
                }
            }));
    uint8_t response[] = {0x02, 0xfd, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x03, 0x1, 0x2, 0x3};
    EXPECT_CALL(
        *fSocketMock,
        send(Matcher<::udp::DatagramPacket const&>(
            IsDatagram(remoteEndpoint, ::estd::slice<uint8_t const>(response)))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    tick(timestamp);
    Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    Mock::VerifyAndClearExpectations(&oemMessageHandlerMock);

    // an OEM Request of known payload type with payload, but payload length is incorrect
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(timestamp));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(0xFEFF))
        .WillRepeatedly(Return(&oemMessageHandlerMock));
    EXPECT_CALL(oemMessageHandlerMock, getRequestPayloadSize()).WillOnce(Return(0x02U));
    uint8_t requestWithPayload[] = {0x02, 0xfd, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x01, 0xFF};
    EXPECT_CALL(*fSocketMock, getLocalPort()).WillOnce(Return(DoIpConstants::Ports::UDP_DISCOVERY));
    // the payload will be discarded
    EXPECT_CALL(*fSocketMock, read(NotNull(), 8))
        .WillOnce(Invoke(ReadBytesFrom(::estd::slice<uint8_t const, 8>(requestWithPayload))));
    fSocketMock->getDataListener()->dataReceived(
        *fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        config.ipAddress(),
        8U);
    expectNackResponse(remoteEndpoint, doip::DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    tick(timestamp);
    Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    Mock::VerifyAndClearExpectations(&oemMessageHandlerMock);

    // an OEM Request of known payload type with payload, payload length is correct, but the payload
    // contents are rejected
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(timestamp));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(0xFEFF))
        .WillRepeatedly(Return(&oemMessageHandlerMock));
    EXPECT_CALL(oemMessageHandlerMock, getRequestPayloadSize()).WillOnce(Return(0x01U));
    uint8_t const receivedPayload[] = {0xFF};
    uint8_t expectedNackCode        = 0x33U;
    EXPECT_CALL(
        oemMessageHandlerMock,
        onPayloadReceived(::testing::ElementsAreArray(receivedPayload, 1), _))
        .WillOnce(DoAll(SetArgReferee<1>(expectedNackCode), Return(false)));
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x01, 0xFF}},
        timestamp);
    expectNackResponse(remoteEndpoint, expectedNackCode);
    tick(timestamp);
    Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    Mock::VerifyAndClearExpectations(&oemMessageHandlerMock);

    // an OEM Request of known payload type with payload, payload length is correct, payload
    // contents are accepted
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(timestamp));
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getOemMessageHandler(0xFEFF))
        .WillRepeatedly(Return(&oemMessageHandlerMock));
    EXPECT_CALL(oemMessageHandlerMock, getRequestPayloadType()).WillOnce(Return(0xFEFF));
    EXPECT_CALL(oemMessageHandlerMock, getRequestPayloadSize()).WillOnce(Return(0x02U));
    EXPECT_CALL(oemMessageHandlerMock, getResponsePayloadType()).WillOnce(Return(0xFEFE));
    EXPECT_CALL(oemMessageHandlerMock, getResponsePayloadSize()).WillOnce(Return(3U));
    uint8_t const receivedPayload2[] = {0x12, 0x34};
    EXPECT_CALL(
        oemMessageHandlerMock,
        onPayloadReceived(::testing::ElementsAreArray(receivedPayload2, 2), _))
        .WillOnce(Return(true));
    EXPECT_CALL(oemMessageHandlerMock, createResponse(_))
        .WillOnce(Invoke(
            [responsePayload](::estd::slice<uint8_t> response)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    response[i] = responsePayload[i];
                }
            }));
    receiveRequest(
        config,
        remoteEndpoint,
        {{0x02, 0xfd, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x12, 0x34}},
        timestamp);
    EXPECT_CALL(
        *fSocketMock,
        send(Matcher<::udp::DatagramPacket const&>(
            IsDatagram(remoteEndpoint, ::estd::slice<uint8_t const>(response)))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    tick(timestamp);
    Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    Mock::VerifyAndClearExpectations(&oemMessageHandlerMock);

    // wait up to end of wait time and expect first announcement
    timestamp = fParametersMock.getAnnounceWait();
    expectAnnouncement(broadcastEndpoint, timestamp);
    tick(timestamp);

    EXPECT_CALL(*fSocketMock, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

} // namespace
