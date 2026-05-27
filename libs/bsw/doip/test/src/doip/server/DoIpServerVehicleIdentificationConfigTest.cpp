/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerVehicleIdentificationConfig.h"

#include "doip/server/DoIpServerEntityStatusCallbackMock.h"
#include "doip/server/DoIpServerVehicleAnnouncementParameterProviderMock.h"
#include "doip/server/DoIpServerVehicleIdentificationCallbackMock.h"

#include <async/AsyncMock.h>
#include <etl/pool.h>
#include <ip/NetworkInterfaceConfigRegistryMock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(DoIpServerVehicleIdentificationConfigTest, TestInitialization)
{
    DoIpConstants::ProtocolVersion const protocolVersion(
        DoIpConstants::ProtocolVersion::version02Iso2012);
    ::async::ContextType asyncContext(1U);
    StrictMock<DoIpServerEntityStatusCallbackMock> entityStatusCallbackMock;
    StrictMock<DoIpServerVehicleIdentificationCallbackMock> vehicleIdentificationCallbackMock;
    StrictMock<::ip::NetworkInterfaceConfigRegistryMock> networkInterfaceConfigRegistryMock;
    StrictMock<DoIpServerVehicleAnnouncementParameterProviderMock> parametersMock;
    ::etl::pool<DoIpServerVehicleIdentificationRequest, 5U> requestPool;
    DoIpServerVehicleIdentificationConfig cut(
        protocolVersion,
        asyncContext,
        vehicleIdentificationCallbackMock,
        entityStatusCallbackMock,
        networkInterfaceConfigRegistryMock,
        0x2243U,
        parametersMock,
        requestPool);
    EXPECT_EQ(protocolVersion, cut.getProtocolVersion());
    EXPECT_EQ(asyncContext, cut.getContext());
    EXPECT_EQ(&vehicleIdentificationCallbackMock, &cut.getVehicleIdentificationCallback());
    EXPECT_EQ(&entityStatusCallbackMock, &cut.getEntityStatusCallback());
    EXPECT_EQ(&networkInterfaceConfigRegistryMock, &cut.getNetworkInterfaceConfigRegistry());
    EXPECT_EQ(0x2243U, cut.getLogicalEntityAddress());
    EXPECT_EQ(&parametersMock, &cut.getParameters());
    EXPECT_EQ(&requestPool, &cut.getRequestPool());
}

} // namespace test
} // namespace doip
