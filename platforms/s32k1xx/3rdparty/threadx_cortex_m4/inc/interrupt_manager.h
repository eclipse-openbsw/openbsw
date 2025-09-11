// Copyright 2025 Accenture.

#pragma once

#if defined(__GNUC__) && (!defined(__ASSEMBLER__))
#include "mcu/mcu.h"
#endif

#if defined(__cplusplus)
extern "C" {
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
