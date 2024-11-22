// Copyright 2024 Accenture.

#include "bsp/Bsp.h"
#include "bsp/SystemTime.h"
#include "bsp/can/canTransceiver/CanPhy.h"
#include "bsp/charInputOutput/Bspio.h"
#include "bsp/eeprom/IEepromDriver.h"
#include "bsp/eepromemulation/IEepromEmulationDriver.h"
#include "bsp/power/IEcuPowerStateController.h"
#include "bsp/timer/SystemTimer.h"

#include <gtest/gtest.h>

// Include mock classes
#include "bsp/can/canTransceiver/CanPhyMock.h"
#include "bsp/eeprom/EepromDriverMock.h"
#include "bsp/eepromemulation/EepromEmulationDriverMock.h"
#include "bsp/memory/safeMemoryAccess.h"
#include "bsp/timer/SystemTimerMock.h"

namespace
{
TEST(BspTest, IncludeCheck)
{
    // This test currently only checks if the includes are working.
}

} // anonymous namespace
