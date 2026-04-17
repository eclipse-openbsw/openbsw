// Copyright 2025 Accenture.

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
    ::ip::declare::NetworkInterfaceConfigRegistry<2>::ConfigChangedSignal fConfigChangedSignal;
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

} // namespace
