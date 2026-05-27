/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(DoIpServerTransportLayerParametersTest, TestAll)
{
    DoIpServerTransportLayerParameters cut(
        1234U,    // initialInactivityTimeout
        5689123U, // generalInactivityTimeout
        3432U,    // aliveCheckTimeout
        456789U   // maxPayloadLength
    );
    ASSERT_EQ(1234U, cut.getInitialInactivityTimeout());
    ASSERT_EQ(5689123U, cut.getGeneralInactivityTimeout());
    ASSERT_EQ(3432U, cut.getAliveCheckTimeout());
    ASSERT_EQ(456789U, cut.getMaxPayloadLength());
}

} // namespace test
} // namespace doip
