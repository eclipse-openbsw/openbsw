// Copyright 2024 Accenture.

#include "lifecycle/StaticBsp.h"

#include "bsp/SystemTime.h"
#include "bsp/eeprom/EepromConfiguration.h"
#include "bsp/phy/phyConfiguration.h"
#include "bsp/timer/ftmConfiguration.hpp"
#include "clock/clockConfig.h"
#include "commonDebug.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "interrupts/disableEnableAllInterrupts.h"
#include "io/Io.h"
#include "mcu/mcu.h"
#include "mdio/MdioTja1101.h"
#include "sci/SciDevice.h"

using namespace bios;

extern "C"
{
extern void initSystemTimerHelper(bool sleep);
}

using Io = bios::Io;

extern StaticBsp staticBsp;

StaticBsp::StaticBsp()
: fCyclic10Msec(0U)
, _eepromDriver(eeprom::EEPROM_CONFIG)
, _output()
, _digitalInput()
, _commonCanPhy()
, _ftm4(*FTM4)
, _pwmSupport(_ftm4)
, _adc()
, _requestUpdateGateRegisters(false)
, _mode(_INIT_)
{
    sciInit();
    initSystemTimer();
}

void StaticBsp::init() { hwInit(); }

void StaticBsp::hwInit()
{
    initSystemTimerHelper(false);
    (void)_eepromDriver.init();
    _output.init(0);
    _digitalInput.init(0);

    sysDelayUs(150U);

    _adc.init();
    _adc.start();

    _ftm4.init(&bios::_cfgFtm4);

    _pwmSupport.init();
    _ftm4.start();
    _pwmSupport.start();

    IP_MPU->RGDAAC[0] = IP_MPU->RGDAAC[0]
                        | (MPU_RGDAAC_M3SM(3) /* 11b - Same as User mode defined in M3UM */
                           | MPU_RGDAAC_M3UM(7) /* 111b - r/w/x */);

    _mode = _RUN_;
}

void StaticBsp::shutdown() {}

void StaticBsp::cyclic()
{
    _adc.cyclic();

    fCyclic10Msec++;
    if (fCyclic10Msec >= 10)
    {
        fCyclic10Msec = 0;
    }
}

bios::CanPhy& StaticBsp::getCanPhy() { return _commonCanPhy; }
