/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerVehicleIdentificationRequest.h"

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(DoIpServerVehicleIdentificationRequestTest, TestAll)
{
    ::ip::IPEndpoint endpoint(::ip::make_ip4(0x2348928U), 12832);
    DoIpServerVehicleIdentificationRequest cut(
        endpoint, DoIpServerVehicleIdentificationRequest::ISOType::ENTITYSTATUS, 0x83U, 0);
    ASSERT_EQ(endpoint, cut.getDestinationEndpoint());
    ASSERT_EQ(0U, cut.getScheduledTime());
    ASSERT_EQ(
        DoIpServerVehicleIdentificationRequest::ISOType::ENTITYSTATUS,
        ::etl::get<DoIpServerVehicleIdentificationRequest::ISOType>(cut.getType()));
    ASSERT_EQ(0x83U, cut.getNackCode());
}

} // namespace test
} // namespace doip
