// Copyright 2024 Accenture.

#include "lifecycle/StaticBsp.h"

#include "bsp/SystemTime.h"
#include "bsp/timer/ftmConfiguration.hpp"
#include "clock/clockConfig.h"
#include "commonDebug.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "interrupts/disableEnableAllInterrupts.h"
#include "io/Io.h"
#include "mcu/mcu.h"
#include "sci/SciDevice.h"

using namespace bios;

extern "C"
{
extern void initSystemTimerHelper(bool sleep);
}

extern StaticBsp staticBsp;

StaticBsp::StaticBsp()
: fCyclic10Msec(0U)
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
    _output.init(0);
    _digitalInput.init(0);

    sysDelayUs(150U);

    _adc.init();
    _adc.start();

    _ftm4.init(&_cfgFtm4);

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

bios::CanPhy& StaticBsp::getCanPhy() { return _commonCanPhy; }

void StaticBsp::releasePins()
{
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
}
