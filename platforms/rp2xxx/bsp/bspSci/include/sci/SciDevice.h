/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "io/Io.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct sciBaudRate
{
    uint32_t BAUD;
};

using SciConfig = struct
{
#if 0
    LPUART_Type* sci;
    bios::Io::PinId txPin;
    bios::Io::PinId rxPin;
    uint8_t speedModes;
    sciBaudRate const* fpBaudRate;
#endif
    uint32_t dummy;
};

/**
 * Initialize serial interface with default baut rate of 115200K
 */
void sciInit(uint8_t speedMode = 0);
/**
 * Read current init state
 */
uint8_t sciGetInitState(void);
/**
 * Read current status of read process
 * \return 1-> Ready, 0-> NOT Ready
 */
uint8_t sciGetRxReady(void);

/**
 * Read symbol from the interface
 * \return symbol
 */
uint8_t sciGeth(void);

/**
 * Read current transfer status
 * \return NOT Ready -> 1, Ready -> 0
 */
uint8_t sciGetTxNotReady(void);

/**
 * Put the symbol onto the interface
 */
void sciPuth(int c);

#ifdef __cplusplus
} // extern "C"
#endif
