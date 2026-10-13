/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerVehicleIdentificationService.h"

#include "bsp/timer/SystemTimerMock.h"
#include "doip/common/DoIpVehicleAnnouncementListenerMock.h"
#include "doip/server/DoIpServerEntityStatusCallbackMock.h"
#include "doip/server/DoIpServerVehicleAnnouncementParameterProviderMock.h"
#include "doip/server/DoIpServerVehicleIdentificationCallbackMock.h"
#include "doip/server/DoIpServerVehicleIdentificationConfig.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <common/busid/BusId.h>
#include <ip/NetworkInterfaceConfigRegistryMock.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

namespace
{
using namespace ::testing;
using namespace ::doip;

MATCHER_P(IsDatagram, endpoint, "")
{
    ::udp::DatagramPacket const& argDatagram = arg;
    return (argDatagram.getEndpoint() == endpoint);
}

class DoIpServerVehicleIdentificationServiceTest : public Test
{
public:
    DoIpServerVehicleIdentificationServiceTest()
    : asyncMock(), asyncContext(1U), testContext(asyncContext)
    {}

protected:
    virtual void SetUp();
    virtual void TearDown();

    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    DoIpServerEntityStatusCallbackMock fEntityStatusCallbackMock;
    ::testing::StrictMock<DoIpServerVehicleIdentificationCallbackMock>
        fVehicleIdentificationCallbackMock;
    DoIpVehicleAnnouncementListenerMock fVehicleAnnouncementListenerMock;
    ::ip::NetworkInterfaceConfigRegistryMock fNetworkInterfaceConfigRegistryMock;
    ::testing::StrictMock<DoIpServerVehicleAnnouncementParameterProviderMock> fParametersMock;
    ::etl::signal<void(uint8_t, ::ip::NetworkInterfaceConfig const&), 2> fConfigChangedSignal;
    ::testing::StrictMock<SystemTimerMock> timerMock;
};

void DoIpServerVehicleIdentificationServiceTest::SetUp() { testContext.handleAll(); }

void DoIpServerVehicleIdentificationServiceTest::TearDown() {}

TEST_F(DoIpServerVehicleIdentificationServiceTest, TestAddSockets)
{
    ::doip::declare::
        DoIpServerVehicleIdentificationService<::udp::AbstractDatagramSocketMock, 2U, 2U, 1U, 3U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                asyncContext,
                fVehicleIdentificationCallbackMock,
                fEntityStatusCallbackMock,
                fNetworkInterfaceConfigRegistryMock,
                0x1122U,
                fParametersMock);

    // add a first socket
    ::ip::NetworkInterfaceConfigKey configKey1(0U);
    ::ip::IPAddress multicastAddress1 = ::ip::make_ip4(0x38492U);
    DoIpServerVehicleIdentificationSocketHandler& handler1
        = cut.addSocket(12U, configKey1, multicastAddress1);

    // and a second one
    ::ip::NetworkInterfaceConfigKey configKey2(1U);
    ::ip::IPAddress multicastAddress2 = ::ip::make_ip4(0x382U);
    DoIpServerVehicleIdentificationSocketHandler& handler2
        = cut.addSocket(15U, configKey2, multicastAddress2);

    // start the service (expect two times accessing the registry)
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey1))
        .WillOnce(Return(::ip::NetworkInterfaceConfig()));
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey2))
        .WillOnce(Return(::ip::NetworkInterfaceConfig()));

    // set Listener Mock
    cut.setVehicleAnnouncementListener(&fVehicleAnnouncementListenerMock);

    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&fNetworkInterfaceConfigRegistryMock);

    // propagate the unicast addresses to the sockets
    ::ip::IPAddress addresses[2];
    addresses[0] = ::ip::make_ip4(0x234783U);
    addresses[1] = ::ip::make_ip4(0x9845384U);
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));
    cut.updateUnicastAddresses(configKey2, addresses);
    EXPECT_EQ(0U, handler1.getUnicastAddresses().size());
    EXPECT_EQ(2U, handler2.getUnicastAddresses().size());

    // send a single announcement
    EXPECT_CALL(fParametersMock, getAnnounceWait()).Times(2).WillRepeatedly(Return(150U));
    cut.sendAnnouncement();

    // Shutdown
    cut.shutdown();
}

TEST_F(DoIpServerVehicleIdentificationServiceTest, ConfigChangeOnOneSocketKeepsOtherSocketsRequests)
{
    ::doip::declare::
        DoIpServerVehicleIdentificationService<::udp::AbstractDatagramSocketMock, 2U, 2U, 1U, 1U>
            cut(DoIpConstants::ProtocolVersion::version02Iso2012,
                asyncContext,
                fVehicleIdentificationCallbackMock,
                fEntityStatusCallbackMock,
                fNetworkInterfaceConfigRegistryMock,
                0x1122U,
                fParametersMock);

    // add the first socket
    ::ip::NetworkInterfaceConfigKey configKey1(0U);
    auto& handler1 = cut.addSocket(12U, configKey1, ::ip::make_ip4(0x1U));
    ::udp::AbstractDatagramSocketMock& socket1 = handler1.getSocket();
    ::ip::NetworkInterfaceConfig const config1(0x1U, 0x00a8ffffU, 0x0U);

    // and the second one
    ::ip::NetworkInterfaceConfigKey configKey2(1U);
    auto& handler2 = cut.addSocket(15U, configKey2, ::ip::make_ip4(0x2U));
    ::udp::AbstractDatagramSocketMock& socket2 = handler2.getSocket();
    ::ip::NetworkInterfaceConfig const config2(0x2U, 0x00a9ffffU, 0x0U);

    // create a broadcast endpoint for the first socket
    ::ip::IPEndpoint const broadcastEndpoint1(
        config1.broadcastAddress(), DoIpConstants::Ports::UDP_DISCOVERY);

    // start the service
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey1))
        .WillOnce(Return(config1));
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(configKey2))
        .WillOnce(Return(config2));

    // each socket schedules pending announcement request in the shared request pool
    EXPECT_CALL(socket1, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(socket2, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));

    EXPECT_CALL(fParametersMock, getAnnounceWait()).WillRepeatedly(Return(50U));
    EXPECT_CALL(fParametersMock, getAnnounceInterval()).WillRepeatedly(Return(500U));
    EXPECT_CALL(timerMock, getSystemTimeMs32Bit()).WillRepeatedly(Return(0U));

    cut.start();
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&fNetworkInterfaceConfigRegistryMock);
    Mock::VerifyAndClearExpectations(&socket1);
    Mock::VerifyAndClearExpectations(&socket2);

    // now change the network config of the second socket socket
    ::ip::NetworkInterfaceConfig const newConfig2(0xc0a90101U, 0xc0a9ffffU, 0x0U);
    EXPECT_CALL(socket2, close());
    EXPECT_CALL(socket2, bind(NotNull(), DoIpConstants::Ports::UDP_DISCOVERY))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    fNetworkInterfaceConfigRegistryMock.configChangedSignal(configKey2, newConfig2);
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&socket2);

    EXPECT_CALL(fVehicleIdentificationCallbackMock, getVin(_)).Times(AnyNumber());
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getEid(_)).Times(AnyNumber());
    EXPECT_CALL(fVehicleIdentificationCallbackMock, getGid(_)).Times(AnyNumber());

    // the first socket's originally scheduled announcement must still be sent to own endpoint
    EXPECT_CALL(
        socket1, send(Matcher<::udp::DatagramPacket const&>(IsDatagram(broadcastEndpoint1))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(socket2, send(Matcher<::udp::DatagramPacket const&>(_)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));

    // fire timeouts to call send()
    testContext.setNow(50000U);
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&socket1);
    Mock::VerifyAndClearExpectations(&socket2);

    EXPECT_CALL(socket1, close());
    EXPECT_CALL(socket2, close());
    cut.shutdown();
    testContext.expireAndExecute();
}

} // namespace
