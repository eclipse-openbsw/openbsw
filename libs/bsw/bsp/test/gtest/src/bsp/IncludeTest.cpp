// Copyright 2024 Accenture.

#include "bsp/Bsp.h"
#include "bsp/SystemTime.h"
#include "bsp/background/AbstractBackgroundJob.h"
#include "bsp/background/IBackgroundJobHandler.h"
#include "bsp/can/ICanPhy.h"
#include "bsp/can/canTransceiver/CanPhy.h"
#include "bsp/charInputOutput/Bspio.h"
#include "bsp/commandInterpreter/ICommandInterpreter.h"
#include "bsp/eeprom/IEepromDriver.h"
#include "bsp/eepromemulation/IEepromEmulationDriver.h"
#include "bsp/ethernet/IMdio.h"
#include "bsp/ethernet/IMdioClause22.h"
#include "bsp/ethernet/IMdioClause45.h"
#include "bsp/flash/IFlashDriver.h"
#include "bsp/flexray/IFlexrayPhy.h"
#include "bsp/hwInfo/IHardwareInfoProvider.h"
#include "bsp/i2c/I2cCommon.h"
#include "bsp/i2c/II2cMaster.h"
#include "bsp/lin/ILinDevice.h"
#include "bsp/lin/ILinSciReceiver.h"
#include "bsp/power/AbstractPowerStateListener.h"
#include "bsp/power/IEcuPowerStateController.h"
#include "bsp/power/IUnderVoltageListener.h"
#include "bsp/power/IVoltageManager.h"
#include "bsp/timer/ICompareTimer.h"
#include "bsp/timer/IHighResolutionTimer.h"
#include "bsp/timer/SystemTimer.h"
#include "bsp/xcp/IOverlayController.h"

#include <gtest/gtest.h>

// Include mock classes
#include "bsp/background/AbstractBackgroundJobMock.h"
#include "bsp/background/BackgroundJobHandlerMock.h"
#include "bsp/can/CanPhyMock.h"
#include "bsp/can/canTransceiver/CanPhyMock.h"
#include "bsp/eeprom/EepromDriverMock.h"
#include "bsp/eepromemulation/EepromEmulationDriverMock.h"
#include "bsp/error/IEventListener.h"
#include "bsp/hwInfo/HardwareInfoProviderMock.h"
#include "bsp/memory/safeMemoryAccess.h"
#include "bsp/power/AbstractPowerStateListenerMock.h"
#include "bsp/stepperMotor/IStepperMotorDevice.h"
#include "bsp/timer/SystemTimerMock.h"

namespace
{
TEST(BspTest, IncludeCheck)
{
    // This test currently only checks if the includes are working.
}

} // anonymous namespace
