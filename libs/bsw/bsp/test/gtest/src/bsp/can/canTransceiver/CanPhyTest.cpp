// Copyright 2024 Accenture.

#include "bsp/can/canTransceiver/CanPhy.h"

#include "bsp/can/CanPhyMock.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::bios;
using namespace testing;

TEST(CanPhyDummy, SimpleTest)
{
    uint32_t const DOES_NOT_HAVE_ANY_EFFECT = 0U;
    CanPhyDummy& canPhy                     = CanPhyDummy::getInstance();
    canPhy.init();
    canPhy.setMode(CanPhy::CAN_PHY_MODE_STANDBY);

    ASSERT_EQ(CanPhy::CAN_PHY_MODE_STANDBY, canPhy.getMode());
    ASSERT_EQ(
        CanPhy::CAN_PHY_ERROR_UNSUPPORTED, canPhy.getPhyErrorStatus(DOES_NOT_HAVE_ANY_EFFECT));
}

} // anonymous namespace
