#include "interrupt_manager.h"
#include "mcu/mcu.h"

void SYS_EnableIRQ(IRQn_Type irqNumber)
{
    NVIC_EnableIRQ(irqNumber);
}

void SYS_DisableIRQ(IRQn_Type irqNumber)
{
    NVIC_DisableIRQ(irqNumber);
}

void SYS_SetPriority(IRQn_Type irqNumber, uint8_t priority)
{
    NVIC_SetPriority(irqNumber, priority);
}

uint8_t SYS_GetPriority(IRQn_Type irqNumber)
{
    return NVIC_GetPriority(irqNumber);
}


void SYS_ClearPendingIRQ(IRQn_Type irqNumber)
{
    NVIC_ClearPendingIRQ(irqNumber);
}

void SYS_SetPendingIRQ(IRQn_Type irqNumber)
{
    NVIC_SetPendingIRQ(irqNumber);
}

uint32_t SYS_GetPendingIRQ(IRQn_Type irqNumber)
{
    return NVIC_GetPendingIRQ(irqNumber);
}

uint32_t SYS_GetActive(IRQn_Type irqNumber)
{
    return NVIC_GetActive(irqNumber);
}
/*
 * Illegal ISR
 * replace weak Function from startUp default ISR table
 */
extern void vIllegalISR(void);

void DefaultISR(void)
{
	vIllegalISR();
}
