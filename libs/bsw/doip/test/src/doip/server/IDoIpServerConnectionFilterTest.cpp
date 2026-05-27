/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/IDoIpServerConnectionFilter.h"

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(IDoIpServerConnectionFilterTest, TestRoutingActivationCheckResult)
{
    // default constructor
    IDoIpServerConnectionFilter::RoutingActivationCheckResult cut;
    EXPECT_EQ(IDoIpServerConnectionFilter::Action::CONTINUE, cut.getAction());
    EXPECT_EQ(uint8_t(DoIpConstants::RoutingResponseCodes::ROUTING_SUCCESS), cut.getResponseCode());
    EXPECT_EQ(
        uint16_t(::transport::TransportMessage::INVALID_ADDRESS), cut.getInternalSourceAddress());
    EXPECT_EQ(0x123U, cut.resolveInternalSourceAddress(0x123U));
    // setters
    cut.setAction(IDoIpServerConnectionFilter::Action::KEEP)
        .setResponseCode(0x08U)
        .setInternalSourceAddress(0x1234U);
    EXPECT_EQ(IDoIpServerConnectionFilter::Action::KEEP, cut.getAction());
    EXPECT_EQ(0x08U, cut.getResponseCode());
    EXPECT_EQ(0x1234, cut.getInternalSourceAddress());
    EXPECT_EQ(0x1234U, cut.resolveInternalSourceAddress(0x123U));
}

} // namespace test
} // namespace doip
