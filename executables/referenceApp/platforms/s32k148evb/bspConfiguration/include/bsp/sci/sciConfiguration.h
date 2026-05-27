/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

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
