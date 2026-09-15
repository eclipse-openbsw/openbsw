/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/StaticBsp.h"

#include "bsp/SystemTime.h"
#include "bsp/eeprom/EepromConfiguration.h"
#include "bsp/timer/ftmConfiguration.hpp"
#include "clock/clockConfig.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "interrupts/disableEnableAllInterrupts.h"
#include "io/Io.h"
#include "mcu/mcu.h"
#include "sci/SciDevice.h"

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

void StaticBsp::releasePins()
{
#if 0
    (void)Io::resetConfig(Io::SPI1_SCK);
    (void)Io::resetConfig(Io::SPI1_CSN1);
    (void)Io::resetConfig(Io::SPI1_MOSI);
    (void)Io::resetConfig(Io::SPI1_CSN0);
    (void)Io::resetConfig(Io::SPI2_MOSI);
    (void)Io::resetConfig(Io::SPI2_CSN0);
    (void)Io::resetConfig(Io::SPI2_SCK);

    (void)Io::resetConfig(Io::UART1_RX);

    (void)Io::resetConfig(Io::canTx);
    (void)Io::resetConfig(Io::LIN1_Tx);
    (void)Io::resetConfig(Io::UART_TX);
#endif
}
