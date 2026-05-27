/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#if defined(__GNUC__) && (!defined(__ASSEMBLER__))
#include "mcu/mcu.h"
#endif

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus*/

void DefaultISR(void);

void SYS_EnableIRQ(IRQn_Type irqNumber);

void SYS_DisableIRQ(IRQn_Type irqNumber);

void SYS_SetPriority(IRQn_Type irqNumber, uint8_t priority);

uint8_t SYS_GetPriority(IRQn_Type irqNumber);

void SYS_ClearPendingIRQ(IRQn_Type irqNumber);

void SYS_SetPendingIRQ(IRQn_Type irqNumber);

uint32_t SYS_GetPendingIRQ(IRQn_Type irqNumber);

uint32_t SYS_GetActive(IRQn_Type irqNumber);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/
