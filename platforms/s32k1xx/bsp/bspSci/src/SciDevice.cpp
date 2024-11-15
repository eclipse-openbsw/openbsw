// Copyright 2024 Accenture.

#include "sci/SciDevice.h"

#include "bsp/clock/boardClock.h"
#include "bsp/sci/sciConfiguration.h"
#include "mcu/mcu.h"

extern "C"
{
void sciInit(uint8_t const speedMode)
{
    (void)speedMode;

    (void)bios::Io::setDefaultConfiguration(sciConfiguration.txPin);
    (void)bios::Io::setDefaultConfiguration(sciConfiguration.rxPin);
    sciConfiguration.sci->GLOBAL = 0;
    sciConfiguration.sci->CTRL   = 0;

    sciConfiguration.sci->PINCFG = 0;
    sciConfiguration.sci->BAUD   = 0;
    sciConfiguration.sci->BAUD   = sciConfiguration.fpBaudRate[0].BAUD;
    sciConfiguration.sci->STAT   = 0xFFFFFFFFU;
    sciConfiguration.sci->STAT   = 0;
    sciConfiguration.sci->MODIR  = 0;
    sciConfiguration.sci->FIFO   = 0;
    sciConfiguration.sci->WATER  = 0;
    // Last
    sciConfiguration.sci->CTRL   = LPUART_CTRL_RE(1U) + LPUART_CTRL_TE(1U);
}

uint8_t sciGetRxReady()
{
    if ((sciConfiguration.sci->STAT & LPUART_STAT_OR_MASK) != 0)
    {
        sciConfiguration.sci->STAT = sciConfiguration.sci->STAT | LPUART_STAT_OR_MASK;
    }
    if ((sciConfiguration.sci->STAT & LPUART_STAT_RDRF_MASK) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t sciGeth() { return static_cast<uint8_t>(sciConfiguration.sci->DATA & 0xFFU); }

uint8_t sciGetTxNotReady()
{
    if ((sciConfiguration.sci->STAT & LPUART_STAT_TDRE_MASK) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void sciPuth(int const c) { sciConfiguration.sci->DATA = (static_cast<uint32_t>(c) & 0xFFU); }

uint8_t sciGetInitState()
{
    uint32_t const ctrl = sciConfiguration.sci->CTRL;
    if (((ctrl & LPUART_CTRL_RE_MASK) == 0) || ((ctrl & LPUART_CTRL_TE_MASK) == 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

} // extern "C"
