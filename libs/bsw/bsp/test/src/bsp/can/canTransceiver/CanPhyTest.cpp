/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/can/canTransceiver/CanPhy.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::bios;
using namespace testing;

TEST(CanPhyDummy, SimpleTest)
{
    uint32_t const DOES_NOT_HAVE_ANY_EFFECT = 0U;
    CanPhyDummy& canPhy                     = CanPhyDummy::getInstance();
    canPhy.init(0U);
    canPhy.setMode(CanPhy::CAN_PHY_MODE_STANDBY, 0U);

    ASSERT_EQ(CanPhy::CAN_PHY_MODE_STANDBY, canPhy.getMode(0U));
    ASSERT_EQ(
        CanPhy::CAN_PHY_ERROR_UNSUPPORTED, canPhy.getPhyErrorStatus(DOES_NOT_HAVE_ANY_EFFECT));
}

} // anonymous namespace
