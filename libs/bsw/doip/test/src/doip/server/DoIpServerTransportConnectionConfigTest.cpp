/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportConnectionConfig.h"

#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <async/Types.h>
#include <common/busid/BusId.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;
using namespace ::transport;

TEST(DoIpServerTransportConnectionConfigTest, TestInitialization)
{
    uint8_t busId(0U);
    ::async::ContextType asyncContext(1U);
    StrictMock<DoIpTransportMessageProvidingListenerMock> messageProvidingListenerMock;
    StrictMock<TransportMessageProcessedListenerMock> messageProcessedListenerMock;
    DoIpServerTransportLayerParameters parameters(0x123U, 0x4412U, 0x2139U, 0x8765432U);
    DoIpServerTransportConnectionConfig cut(
        busId,
        0x1357,
        asyncContext,
        messageProvidingListenerMock,
        messageProcessedListenerMock,
        parameters);
    EXPECT_EQ(busId, cut.getBusId());
    EXPECT_EQ(0x1357, cut.getLogicalEntityAddress());
    EXPECT_EQ(asyncContext, cut.getContext());
    EXPECT_EQ(&messageProvidingListenerMock, &cut.getMessageProvidingListener());
    EXPECT_EQ(&messageProcessedListenerMock, &cut.getMessageProcessedListener());
    EXPECT_EQ(&parameters, &cut.getParameters());
}

} // namespace test
} // namespace doip
