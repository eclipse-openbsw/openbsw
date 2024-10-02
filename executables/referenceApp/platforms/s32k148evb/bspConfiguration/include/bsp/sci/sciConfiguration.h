// Copyright 2024 Accenture.

#ifndef GUARD_04CFEC83_1B43_49EC_975F_F038344D33DF
#define GUARD_04CFEC83_1B43_49EC_975F_F038344D33DF

#include "sci/SciDevice.h"

sciBaudRate const sciBaudRateConfig[] = {
    {(LPUART_BAUD_OSR(15)) + LPUART_BAUD_SBR(26)}, // = 115200 48MHz FIRC
    {(LPUART_BAUD_OSR(9)) + LPUART_BAUD_SBR(8)}    // = 2MBit 80MHz PLL
};

SciConfig const sciConfiguration
    = {(LPUART1),
       bios::Io::UART1_TX,
       bios::Io::UART1_RX,
       static_cast<uint8_t>(sizeof(sciBaudRateConfig) / sizeof(sciBaudRate)),
       sciBaudRateConfig};

#endif // sciConfiguration.h
