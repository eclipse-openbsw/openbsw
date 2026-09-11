#ifndef GUARD_1179B604_D69A_440B_B7EC_9D8E64F977CE
#define GUARD_1179B604_D69A_440B_B7EC_9D8E64F977CE

#include "FreeRTOSConfig.h"

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

#endif /* GUARD_1179B604_D69A_440B_B7EC_9D8E64F977CE */