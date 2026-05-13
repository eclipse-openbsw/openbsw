// Copyright 2025 Accenture.

#include "lifecycle/StaticBsp.h"

#include "bsp/SystemTime.h"
#include "bsp/eeprom/EepromConfiguration.h"
#include "bsp/timer/ftmConfiguration.hpp"
#include "clock/clockConfig.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "interrupts/disableEnableAllInterrupts.h"
#include "io/Io.h"
#include "mcu/mcu.h"

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
, _ftm4(*FTM4)
, _pwmSupport(_ftm4)
, _adc()
, _requestUpdateGateRegisters(false)
, _mode(_INIT_)
{
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
